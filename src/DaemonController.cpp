#include "DaemonController.h"
#include <QTimer>
#include <QDebug>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <utility>

const QDBusArgument &operator>>(const QDBusArgument &argument, DbusGpuDevice &device) {
    argument.beginStructure();
    argument >> device.name >> device.pci >> device.render >> device.card >> device.isDefault >> device.nvidia >> device.nvidiaMinor;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DbusGpuDevice &device) {
    argument.beginStructure();
    argument << device.name << device.pci << device.render << device.card << device.isDefault << device.nvidia << device.nvidiaMinor;
    argument.endStructure();
    return argument;
}

DaemonController::DaemonController() {
    qRegisterMetaType<DbusGpuDevice>("DbusGpuDevice");
    qDBusRegisterMetaType<DbusGpuDevice>();

    modeInterface = new QDBusInterface(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "com.github.opengamingcollective.cardwire.Mode",
        bus,
        this
    );

    configInterface = new QDBusInterface(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "com.github.opengamingcollective.cardwire.Config",
        bus,
        this
    );

    managerInterface = new QDBusInterface(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "com.github.opengamingcollective.cardwire.Manager",
        bus,
        this
    );

    refreshDevices();
    pollDaemon();

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DaemonController::pollDaemon);
    timer->setInterval(5000);
    timer->start();
}

void DaemonController::refreshDevices() {
    qDeleteAll(m_gpus);
    m_gpus.clear();

    QDBusInterface manager(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "org.freedesktop.DBus.ObjectManager",
        bus,
        this
    );

    QDBusReply<QMap<QDBusObjectPath, QMap<QString, QVariantMap>>> reply = manager.call("GetManagedObjects");
    if (!reply.isValid()) {
        m_isDaemonFailing = true;
        emit daemonFailingChanged();
        return;
    }

    m_isDaemonFailing = false;
    emit daemonFailingChanged();

    auto objects = reply.value();
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        QString pathStr = it.key().path();
        if (pathStr.startsWith("/com/github/opengamingcollective/cardwire/Gpu/")) {
            QString idStr = pathStr.mid(pathStr.lastIndexOf('/') + 1);
            bool ok;
            int gpuId = idStr.toInt(&ok);
            if (ok) {
                QDBusInterface gpuInterface(
                    "com.github.opengamingcollective.cardwire",
                    pathStr,
                    "com.github.opengamingcollective.cardwire.Gpu",
                    bus,
                    this
                );

                QDBusReply<DbusGpuDevice> deviceReply = gpuInterface.call("GetDevice");
                if (deviceReply.isValid()) {
                    DbusGpuDevice rawDev = deviceReply.value();
                    
                    QVariant blockVal = gpuInterface.property("Block");
                    bool isBlocked = blockVal.isValid() ? blockVal.toBool() : false;

                    QDBusReply<QString> powerReply = gpuInterface.call("PowerState");
                    QString powerState = powerReply.isValid() ? powerReply.value().trimmed() : QStringLiteral("Unknown");

                    auto *gpu = new CardwireGpu(gpuId, rawDev.name, rawDev.pci, rawDev.isDefault, isBlocked, powerState, this);
                    m_gpus.append(gpu);

                    // Subscribe to power state changed signal
                    bus.connect(
                        "com.github.opengamingcollective.cardwire",
                        pathStr,
                        "com.github.opengamingcollective.cardwire.Gpu",
                        "power_state_changed",
                        this,
                        SLOT(onPowerStateChanged(QString, QDBusMessage))
                    );
                }
            }
        }
    }
    emit gpusChanged();
}

void DaemonController::pollDaemon() {
    QDBusReply<void> statusReply = managerInterface->call("Status");
    bool failing = !statusReply.isValid();
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
    fetchConfig();

    for (CardwireGpu *gpu : std::as_const(m_gpus)) {
        fetchGpuDynamic(gpu);
    }
}

void DaemonController::fetchMode() {
    QVariant val = modeInterface->property("Mode");
    if (val.isValid()) {
        quint32 mode = val.toUInt();
        if (m_mode != mode) {
            m_mode = mode;
            emit modeChanged();
        }
    }
}

void DaemonController::fetchConfig() {
    bool configChangedFlag = false;

    QVariant autoApplyVal = configInterface->property("AutoApplyGpuState");
    if (autoApplyVal.isValid()) {
        bool autoApply = autoApplyVal.toBool();
        if (m_autoApplyGpuState != autoApply) {
            m_autoApplyGpuState = autoApply;
            configChangedFlag = true;
        }
    }

    QVariant nvidiaBlockVal = configInterface->property("ExperimentalNvidiaBlock");
    if (nvidiaBlockVal.isValid()) {
        bool nvidiaBlock = nvidiaBlockVal.toBool();
        if (m_experimentalNvidiaBlock != nvidiaBlock) {
            m_experimentalNvidiaBlock = nvidiaBlock;
            configChangedFlag = true;
        }
    }

    QVariant batterySwitchVal = configInterface->property("BatteryAutoSwitch");
    if (batterySwitchVal.isValid()) {
        bool batterySwitch = batterySwitchVal.toBool();
        if (m_batteryAutoSwitch != batterySwitch) {
            m_batteryAutoSwitch = batterySwitch;
            configChangedFlag = true;
        }
    }

    QVariant batterySwitchModeVal = configInterface->property("BatteryAutoSwitchMode");
    if (batterySwitchModeVal.isValid()) {
        quint32 batterySwitchMode = batterySwitchModeVal.toUInt();
        if (m_batteryAutoSwitchMode != batterySwitchMode) {
            m_batteryAutoSwitchMode = batterySwitchMode;
            configChangedFlag = true;
        }
    }

    if (configChangedFlag) {
        emit configChanged();
    }
}

void DaemonController::fetchGpuDynamic(CardwireGpu *gpu) {
    QString pathStr = QStringLiteral("/com/github/opengamingcollective/cardwire/Gpu/%1").arg(gpu->id());
    QDBusInterface gpuInterface(
        "com.github.opengamingcollective.cardwire",
        pathStr,
        "com.github.opengamingcollective.cardwire.Gpu",
        bus,
        this
    );

    // Sync block state
    QVariant blockVal = gpuInterface.property("Block");
    if (blockVal.isValid()) {
        gpu->updateBlocked(blockVal.toBool());
    }

    // Sync power state
    QDBusReply<QString> powerReply = gpuInterface.call("PowerState");
    if (powerReply.isValid()) {
        gpu->updatePowerState(powerReply.value());
    }

    // Sync active applications (lsof)
    QDBusReply<QMap<QString, QStringList>> lsofReply = gpuInterface.call("Lsof");
    if (lsofReply.isValid()) {
        auto processMap = lsofReply.value();
        QSet<QString> uniqueApps;
        for (auto it = processMap.begin(); it != processMap.end(); ++it) {
            for (const QString &app : it.value()) {
                if (!app.trimmed().isEmpty()) {
                    uniqueApps.insert(app.trimmed());
                }
            }
        }
        
        int count = uniqueApps.size();
        QStringList sortedApps = uniqueApps.values();
        sortedApps.sort();
        QString details = sortedApps.join(QStringLiteral("\n"));
        gpu->updateApps(count, details);
    }
}

void DaemonController::setMode(quint32 mode) {
    // Mode is a writable property
    QDBusPendingCall pendingCall = modeInterface->asyncCall("set_mode", mode); // Wait, properties setters in QDBusInterface are call("set_mode", val) or usingsetProperty?
    // Wait, in QDBusInterface, it is safer to use: modeInterface->setProperty("Mode", mode);
    // Let's call configInterface->setProperty or modeInterface->setProperty directly!
    // But setProperty is synchronous. Async is better if possible.
    // Wait! QDBusInterface inherits QDBusAbstractInterface.
    // In QDBusAbstractInterface:
    // QDBusPendingCall asyncCall(const QString &method, const QVariant &arg1 = ...)
    // Wait, the property setter for property "Mode" is not a method "set_mode". In DBus, properties are set via the org.freedesktop.DBus.Properties interface!
    // So we can use:
    // QDBusInterface propertiesInterface("com.github.opengamingcollective.cardwire", "/com/github/opengamingcollective/cardwire", "org.freedesktop.DBus.Properties", bus, this);
    // propertiesInterface.asyncCall("Set", "com.github.opengamingcollective.cardwire.Mode", "Mode", QVariant::fromValue(QDBusVariant(mode)));
    // Or we can use QDBusInterface::setProperty("Mode", mode) which handles Properties.Set automatically under the hood!
    // Wait, is setProperty async or sync? setProperty() is synchronous, but it is extremely short and fast, and for a Plasmoid GUI thread it's perfectly fine.
    // However, if we want to be safe, we can do it via the standard Qt interface->setProperty("Mode", mode) or async call to "org.freedesktop.DBus.Properties".
    // Let's use propertiesInterface.asyncCall as it's non-blocking and very clean!
    // Let's look:
    QDBusInterface properties(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "org.freedesktop.DBus.Properties",
        bus,
        this
    );
    properties.call("Set", "com.github.opengamingcollective.cardwire.Mode", "Mode", QVariant::fromValue(QDBusVariant(mode)));
    
    // We can poll mode immediately to update UI quickly
    QTimer::singleShot(500, this, [this]() {
        fetchMode();
        emit setModeFinished();
    });
}

void DaemonController::setGpuBlocked(int id, bool blocked) {
    QString pathStr = QStringLiteral("/com/github/opengamingcollective/cardwire/Gpu/%1").arg(id);
    QDBusInterface properties(
        "com.github.opengamingcollective.cardwire",
        pathStr,
        "org.freedesktop.DBus.Properties",
        bus,
        this
    );
    properties.call("Set", "com.github.opengamingcollective.cardwire.Gpu", "Block", QVariant::fromValue(QDBusVariant(blocked)));
}

void DaemonController::setAutoApplyGpuState(bool state) {
    QDBusInterface properties(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "org.freedesktop.DBus.Properties",
        bus,
        this
    );
    properties.call("Set", "com.github.opengamingcollective.cardwire.Config", "AutoApplyGpuState", QVariant::fromValue(QDBusVariant(state)));
    QTimer::singleShot(200, this, &DaemonController::fetchConfig);
}

void DaemonController::setExperimentalNvidiaBlock(bool state) {
    QDBusInterface properties(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "org.freedesktop.DBus.Properties",
        bus,
        this
    );
    properties.call("Set", "com.github.opengamingcollective.cardwire.Config", "ExperimentalNvidiaBlock", QVariant::fromValue(QDBusVariant(state)));
    QTimer::singleShot(200, this, &DaemonController::fetchConfig);
}

void DaemonController::setBatteryAutoSwitch(bool state) {
    QDBusInterface properties(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "org.freedesktop.DBus.Properties",
        bus,
        this
    );
    properties.call("Set", "com.github.opengamingcollective.cardwire.Config", "BatteryAutoSwitch", QVariant::fromValue(QDBusVariant(state)));
    QTimer::singleShot(200, this, &DaemonController::fetchConfig);
}

void DaemonController::setBatteryAutoSwitchMode(quint32 mode) {
    QDBusInterface properties(
        "com.github.opengamingcollective.cardwire",
        "/com/github/opengamingcollective/cardwire",
        "org.freedesktop.DBus.Properties",
        bus,
        this
    );
    properties.call("Set", "com.github.opengamingcollective.cardwire.Config", "BatteryAutoSwitchMode", QVariant::fromValue(QDBusVariant(mode)));
    QTimer::singleShot(200, this, &DaemonController::fetchConfig);
}

void DaemonController::onPowerStateChanged(const QString &state, const QDBusMessage &msg) {
    QString path = msg.path();
    QString idStr = path.mid(path.lastIndexOf('/') + 1);
    bool ok;
    int gpuId = idStr.toInt(&ok);
    if (ok) {
        for (CardwireGpu *gpu : std::as_const(m_gpus)) {
            if (gpu->id() == gpuId) {
                gpu->updatePowerState(state);
                break;
            }
        }
    }
}
