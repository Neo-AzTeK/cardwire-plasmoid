#include "DaemonController.h"
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSet>
#include <utility>

DaemonController::DaemonController() {
    pollDaemon();

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DaemonController::pollDaemon);
    timer->setInterval(2000);
    timer->start();
}

void DaemonController::pollDaemon() {
    if (m_commandQueue.size() > 5) {
        return;
    }

    runCommand({"manager", "status"}, [this](const QString & /*output*/, int exitCode) {
        bool failing = (exitCode != 0);
        if (failing != m_isDaemonFailing) {
            m_isDaemonFailing = failing;
            emit daemonFailingChanged();
            if (!m_isDaemonFailing) {
                refreshDevices();
            }
        }

        if (m_isDaemonFailing) {
            return;
        }

        fetchMode();
        
        if (m_gpus.isEmpty()) {
            fetchConfig();
            fetchGpuList();
        } else {
            updateGpuPowerStates();
        }
    });
}

void DaemonController::refreshDevices() {
    if (m_isDaemonFailing) {
        return;
    }
    fetchGpuList();
}

void DaemonController::fetchGpuList() {
    runCommand({"list", "--json"}, [this](const QString &output, int exitCode) {
        if (exitCode != 0) return;

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8(), &parseError);
        if (doc.isNull() || !doc.isObject()) {
            qWarning() << "Failed to parse cardwire list JSON:" << parseError.errorString();
            return;
        }

        QJsonObject obj = doc.object();
        bool listChanged = false;

        QSet<int> newIds;
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            newIds.insert(it.value().toObject().value("id").toInt());
        }

        QSet<int> currentIds;
        for (CardwireGpu *gpu : std::as_const(m_gpus)) {
            currentIds.insert(gpu->id());
        }

        if (newIds != currentIds) {
            qDeleteAll(m_gpus);
            m_gpus.clear();
            listChanged = true;
        }

        for (auto it = obj.begin(); it != obj.end(); ++it) {
            QJsonObject gpuObj = it.value().toObject();
            int id = gpuObj.value("id").toInt();
            QString name = gpuObj.value("name").toString();
            QString pci = gpuObj.value("pci").toString();
            bool isDefault = gpuObj.value("default").toBool();
            bool isBlocked = gpuObj.value("blocked").toBool();

            QString powerState = QStringLiteral("Unknown");
            QFile powerFile(QStringLiteral("/sys/bus/pci/devices/%1/power_state").arg(pci));
            if (powerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                powerState = QString::fromUtf8(powerFile.readAll()).trimmed();
            }

            CardwireGpu *gpu = nullptr;
            for (CardwireGpu *g : std::as_const(m_gpus)) {
                if (g->id() == id) {
                    gpu = g;
                    break;
                }
            }

            if (!gpu) {
                gpu = new CardwireGpu(id, name, pci, isDefault, isBlocked, powerState, this);
                m_gpus.append(gpu);
                listChanged = true;
            } else {
                gpu->updateBlocked(isBlocked);
                gpu->updatePowerState(powerState);
            }
        }

        if (listChanged) {
            emit gpusChanged();
        }
    });
}

void DaemonController::fetchMode() {
    runCommand({"get"}, [this](const QString &output, int exitCode) {
        if (exitCode != 0) return;

        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            if (line.startsWith(QLatin1String("Current Mode:"))) {
                QString modeStr = line.mid(13).trimmed().toLower();
                quint32 mode = 2; // Default to manual
                if (modeStr == QLatin1String("integrated")) mode = 0;
                else if (modeStr == QLatin1String("hybrid")) mode = 1;
                else if (modeStr == QLatin1String("manual")) mode = 2;
                else if (modeStr == QLatin1String("smart")) mode = 3;

                if (m_mode != mode) {
                    m_mode = mode;
                    emit modeChanged();
                }
                break;
            }
        }
    });
}

void DaemonController::fetchConfig() {
    runCommand({"config", "auto-apply-gpu-state"}, [this](const QString &output, int exitCode) {
        if (exitCode != 0) return;
        if (output.contains("true")) {
            if (!m_autoApplyGpuState) { m_autoApplyGpuState = true; emit configChanged(); }
        } else if (output.contains("false")) {
            if (m_autoApplyGpuState) { m_autoApplyGpuState = false; emit configChanged(); }
        }
    });

    runCommand({"config", "experimental-nvidia-block"}, [this](const QString &output, int exitCode) {
        if (exitCode != 0) return;
        if (output.contains("true")) {
            if (!m_experimentalNvidiaBlock) { m_experimentalNvidiaBlock = true; emit configChanged(); }
        } else if (output.contains("false")) {
            if (m_experimentalNvidiaBlock) { m_experimentalNvidiaBlock = false; emit configChanged(); }
        }
    });

    runCommand({"config", "battery-auto-switch"}, [this](const QString &output, int exitCode) {
        if (exitCode != 0) return;
        if (output.contains("true")) {
            if (!m_batteryAutoSwitch) { m_batteryAutoSwitch = true; emit configChanged(); }
        } else if (output.contains("false")) {
            if (m_batteryAutoSwitch) { m_batteryAutoSwitch = false; emit configChanged(); }
        }
    });

    runCommand({"config", "battery-auto-switch-mode"}, [this](const QString &output, int exitCode) {
        if (exitCode != 0) return;
        QString modeStr = output.mid(output.indexOf(':') + 1).trimmed().toLower();
        quint32 mode = 1; // Default to hybrid
        if (modeStr == QLatin1String("integrated")) mode = 0;
        else if (modeStr == QLatin1String("hybrid")) mode = 1;
        else if (modeStr == QLatin1String("manual")) mode = 2;
        else if (modeStr == QLatin1String("smart")) mode = 3;

        if (m_batteryAutoSwitchMode != mode) {
            m_batteryAutoSwitchMode = mode;
            emit configChanged();
        }
    });
}

void DaemonController::setMode(quint32 mode) {
    QString modeStr;
    switch (mode) {
        case 0: modeStr = QLatin1String("integrated"); break;
        case 1: modeStr = QLatin1String("hybrid"); break;
        case 2: modeStr = QLatin1String("manual"); break;
        case 3: modeStr = QLatin1String("smart"); break;
        default: modeStr = QLatin1String("manual"); break;
    }

    runCommand({"set", modeStr}, [this](const QString & /*output*/, int /*exitCode*/) {
        fetchMode();
    });
}

void DaemonController::setGpuBlocked(int id, bool blocked) {
    runCommand({"gpu", QString::number(id), blocked ? "--block" : "--unblock"}, [this](const QString & /*output*/, int /*exitCode*/) {
        fetchGpuList();
    });
}

void DaemonController::setAutoApplyGpuState(bool state) {
    runCommand({"config", "auto-apply-gpu-state", state ? "true" : "false"}, [this](const QString & /*output*/, int /*exitCode*/) {
        runCommand({"config", "save"}, [this](const QString & /*output*/, int /*exitCode*/) {
            fetchConfig();
        });
    });
}

void DaemonController::setExperimentalNvidiaBlock(bool state) {
    runCommand({"config", "experimental-nvidia-block", state ? "true" : "false"}, [this](const QString & /*output*/, int /*exitCode*/) {
        runCommand({"config", "save"}, [this](const QString & /*output*/, int /*exitCode*/) {
            fetchConfig();
        });
    });
}

void DaemonController::setBatteryAutoSwitch(bool state) {
    runCommand({"config", "battery-auto-switch", state ? "true" : "false"}, [this](const QString & /*output*/, int /*exitCode*/) {
        runCommand({"config", "save"}, [this](const QString & /*output*/, int /*exitCode*/) {
            fetchConfig();
        });
    });
}

void DaemonController::setBatteryAutoSwitchMode(quint32 mode) {
    QString modeStr;
    switch (mode) {
        case 0: modeStr = QLatin1String("integrated"); break;
        case 1: modeStr = QLatin1String("hybrid"); break;
        case 2: modeStr = QLatin1String("manual"); break;
        case 3: modeStr = QLatin1String("smart"); break;
        default: modeStr = QLatin1String("hybrid"); break;
    }
    runCommand({"config", "battery-auto-switch-mode", modeStr}, [this](const QString & /*output*/, int /*exitCode*/) {
        runCommand({"config", "save"}, [this](const QString & /*output*/, int /*exitCode*/) {
            fetchConfig();
        });
    });
}

void DaemonController::runCommand(const QStringList &args, std::function<void(const QString &stdOut, int exitCode)> callback) {
    m_commandQueue.enqueue({args, callback});
    if (!m_currentProcess) {
        processNextCommand();
    }
}

void DaemonController::processNextCommand() {
    if (m_commandQueue.isEmpty()) {
        return;
    }

    CommandRequest req = m_commandQueue.dequeue();
    m_currentProcess = new QProcess(this);
    QProcess *proc = m_currentProcess;

    connect(proc, &QProcess::finished, this, [this, req, proc](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
        QString output = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
        proc->deleteLater();
        if (m_currentProcess == proc) {
            m_currentProcess = nullptr;
        }

        req.callback(output, exitCode);

        if (!m_currentProcess) {
            processNextCommand();
        }
    });

    m_currentProcess->start("cardwire", req.args);
}

void DaemonController::updateGpuPowerStates() {
    for (CardwireGpu *gpu : std::as_const(m_gpus)) {
        QString powerState = QStringLiteral("Unknown");
        QFile powerFile(QStringLiteral("/sys/bus/pci/devices/%1/power_state").arg(gpu->pci()));
        if (powerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            powerState = QString::fromUtf8(powerFile.readAll()).trimmed();
        }
        gpu->updatePowerState(powerState);
    }
}
