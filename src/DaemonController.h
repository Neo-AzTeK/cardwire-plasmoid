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

enum class GpuMode : quint32 {
    Integrated = 0,
    Hybrid = 1,
    Manual = 2,
    Smart = 3
};

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
    quint32 mode() const { return static_cast<quint32>(m_mode); }
    QList<CardwireGpu*> gpus() const { return m_gpus; }

    bool autoApplyGpuState() const { return m_autoApplyGpuState; }
    void setAutoApplyGpuState(bool state);

    bool experimentalNvidiaBlock() const { return m_experimentalNvidiaBlock; }
    void setExperimentalNvidiaBlock(bool state);

    bool batteryAutoSwitch() const { return m_batteryAutoSwitch; }
    void setBatteryAutoSwitch(bool state);

    quint32 batteryAutoSwitchMode() const { return static_cast<quint32>(m_batteryAutoSwitchMode); }
    void setBatteryAutoSwitchMode(quint32 mode);

    void setMode(quint32 mode);
    void setGpuBlocked(int id, bool blocked);

    void refreshDevices();
    void pollDaemon();

    static GpuMode modeFromString(const QString &str);
    static QString modeToString(GpuMode mode);

signals:
    void daemonFailingChanged();
    void modeChanged();
    void gpusChanged();
    void configChanged();

private:
    DaemonController();

    bool m_isDaemonFailing = false;
    GpuMode m_mode = GpuMode::Manual;
    QList<CardwireGpu*> m_gpus;

    bool m_autoApplyGpuState = true;
    bool m_experimentalNvidiaBlock = false;
    bool m_batteryAutoSwitch = false;
    GpuMode m_batteryAutoSwitchMode = GpuMode::Hybrid;

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
