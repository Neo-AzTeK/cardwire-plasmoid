#ifndef DAEMONCONTROLLER_H
#define DAEMONCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QQueue>
#include <QProcess>
#include <functional>
#include "CardwireGpu.h"

struct CommandRequest {
    QStringList args;
    std::function<void(const QString &stdOut, int exitCode)> callback;
};

class DaemonController : public QObject {
    Q_OBJECT
public:
    static DaemonController &from() {
        static DaemonController instance;
        return instance;
    }

    ~DaemonController() override = default;

    bool isDaemonFailing() const { return m_isDaemonFailing; }
    quint32 mode() const { return m_mode; }
    QList<CardwireGpu*> gpus() const { return m_gpus; }

    bool autoApplyGpuState() const { return m_autoApplyGpuState; }
    void setAutoApplyGpuState(bool state);

    bool experimentalNvidiaBlock() const { return m_experimentalNvidiaBlock; }
    void setExperimentalNvidiaBlock(bool state);

    bool batteryAutoSwitch() const { return m_batteryAutoSwitch; }
    void setBatteryAutoSwitch(bool state);

    quint32 batteryAutoSwitchMode() const { return m_batteryAutoSwitchMode; }
    void setBatteryAutoSwitchMode(quint32 mode);

    void setMode(quint32 mode);
    void setGpuBlocked(int id, bool blocked);

    void refreshDevices();
    void pollDaemon();

signals:
    void daemonFailingChanged();
    void modeChanged();
    void gpusChanged();
    void configChanged();
    void setModeFinished();

private:
    DaemonController();

    bool m_isDaemonFailing = false;
    quint32 m_mode = 2; // Default to manual
    QList<CardwireGpu*> m_gpus;

    bool m_autoApplyGpuState = true;
    bool m_experimentalNvidiaBlock = false;
    bool m_batteryAutoSwitch = false;
    quint32 m_batteryAutoSwitchMode = 1; // Default to hybrid

    QQueue<CommandRequest> m_commandQueue;
    QProcess *m_currentProcess = nullptr;

    void runCommand(const QStringList &args, std::function<void(const QString &stdOut, int exitCode)> callback);
    void processNextCommand();

    void fetchMode();
    void fetchConfig();
    void fetchGpuList();
    void updateGpuPowerStates();
};

#endif // DAEMONCONTROLLER_H
