#ifndef DAEMONCONTROLLER_H
#define DAEMONCONTROLLER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QMap>
#include <QList>
#include "CardwireGpu.h"

struct DbusGpuDevice {
    QString name;
    QString pci;
    quint32 render;
    quint32 card;
    bool isDefault;
    bool nvidia;
    QString nvidiaMinor;
};
Q_DECLARE_METATYPE(DbusGpuDevice)

const QDBusArgument &operator>>(const QDBusArgument &argument, DbusGpuDevice &device);
QDBusArgument &operator<<(QDBusArgument &argument, const DbusGpuDevice &device);

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

    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface *modeInterface = nullptr;
    QDBusInterface *configInterface = nullptr;
    QDBusInterface *managerInterface = nullptr;

    bool m_isDaemonFailing = false;
    quint32 m_mode = 2; // Default to manual
    QList<CardwireGpu*> m_gpus;

    bool m_autoApplyGpuState = true;
    bool m_experimentalNvidiaBlock = false;
    bool m_batteryAutoSwitch = false;
    quint32 m_batteryAutoSwitchMode = 1; // Default to hybrid

    void fetchMode();
    void fetchConfig();
    void fetchGpuDynamic(CardwireGpu *gpu);

private slots:
    void onPowerStateChanged(const QString &state, const QDBusMessage &msg);
};

#endif // DAEMONCONTROLLER_H
