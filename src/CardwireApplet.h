#ifndef CARDWIREAPPLET_H
#define CARDWIREAPPLET_H

#include <Plasma/Applet>
#include <Solid/Battery>
#include <QVariantList>
#include "DaemonController.h"

class CardwireApplet : public Plasma::Applet {
    Q_OBJECT
    Q_PROPERTY(bool isDaemonFailing READ isDaemonFailing NOTIFY daemonFailingChanged)
    Q_PROPERTY(quint32 mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QVariantList gpus READ gpus NOTIFY gpusChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY chargingChanged)

    // Config properties
    Q_PROPERTY(bool autoApplyGpuState READ autoApplyGpuState WRITE setAutoApplyGpuState NOTIFY configChanged)
    Q_PROPERTY(bool experimentalNvidiaBlock READ experimentalNvidiaBlock WRITE setExperimentalNvidiaBlock NOTIFY configChanged)
    Q_PROPERTY(bool batteryAutoSwitch READ batteryAutoSwitch WRITE setBatteryAutoSwitch NOTIFY configChanged)
    Q_PROPERTY(quint32 batteryAutoSwitchMode READ batteryAutoSwitchMode WRITE setBatteryAutoSwitchMode NOTIFY configChanged)

public:
    CardwireApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~CardwireApplet() override = default;

    bool isDaemonFailing() const;
    quint32 mode() const;
    QVariantList gpus() const;
    QString iconName() const;
    bool isCharging() const;

    bool autoApplyGpuState() const;
    Q_INVOKABLE void setAutoApplyGpuState(bool state);

    bool experimentalNvidiaBlock() const;
    Q_INVOKABLE void setExperimentalNvidiaBlock(bool state);

    bool batteryAutoSwitch() const;
    Q_INVOKABLE void setBatteryAutoSwitch(bool state);

    quint32 batteryAutoSwitchMode() const;
    Q_INVOKABLE void setBatteryAutoSwitchMode(quint32 mode);

    Q_INVOKABLE void setMode(quint32 mode);
    Q_INVOKABLE void refreshDevices();

signals:
    void daemonFailingChanged();
    void modeChanged();
    void gpusChanged();
    void iconNameChanged();
    void chargingChanged();
    void configChanged() override;

private:
    const Solid::Battery *battery = nullptr;
};

#endif // CARDWIREAPPLET_H
