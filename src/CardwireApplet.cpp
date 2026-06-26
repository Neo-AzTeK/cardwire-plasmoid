#include "CardwireApplet.h"
#include <Solid/Device>
#include <Solid/DeviceNotifier>

CardwireApplet::CardwireApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args) {
    
    auto &ctl = DaemonController::from();
    connect(&ctl, &DaemonController::daemonFailingChanged, this, &CardwireApplet::daemonFailingChanged);
    connect(&ctl, &DaemonController::modeChanged, this, &CardwireApplet::modeChanged);
    connect(&ctl, &DaemonController::modeChanged, this, &CardwireApplet::iconNameChanged);
    connect(&ctl, &DaemonController::gpusChanged, this, &CardwireApplet::gpusChanged);
    connect(&ctl, &DaemonController::gpusChanged, this, &CardwireApplet::iconNameChanged);
    connect(&ctl, &DaemonController::configChanged, this, &CardwireApplet::configChanged);

    // Monitor Solid Battery state
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
    for (const Solid::Device &device: devices) {
        auto batteryCandidate = device.as<Solid::Battery>();
        if (batteryCandidate && batteryCandidate->type() == Solid::Battery::PrimaryBattery) {
            battery = batteryCandidate;
            break;
        }
    }
    if (battery) {
        connect(battery, &Solid::Battery::chargeStateChanged, this, &CardwireApplet::chargingChanged);
    }
}

bool CardwireApplet::isDaemonFailing() const {
    return DaemonController::from().isDaemonFailing();
}

quint32 CardwireApplet::mode() const {
    return DaemonController::from().mode();
}

QList<QObject*> CardwireApplet::gpus() const {
    QList<QObject*> list;
    const auto &gpus = DaemonController::from().gpus();
    for (auto *gpu : gpus) {
        connect(gpu, &CardwireGpu::powerStateChanged, this, &CardwireApplet::iconNameChanged, Qt::UniqueConnection);
        list.append(gpu);
    }
    return list;
}

QString CardwireApplet::iconName() const {
    quint32 currentMode = DaemonController::from().mode();
    const auto &gpus = DaemonController::from().gpus();
    
    bool dGpuActive = false;
    for (auto *gpu : gpus) {
        if (!gpu->isDefault()) {
            QString powerState = gpu->powerState().trimmed().toLower();
            if (powerState != QStringLiteral("d3cold") && powerState != QStringLiteral("unknown")) {
                dGpuActive = true;
            }
        }
    }

    switch (currentMode) {
        case 0: // Integrated
            return QStringLiteral("cardwire-plasmoid-gpu-integrated");
        case 1: // Hybrid
            return dGpuActive ? QStringLiteral("cardwire-plasmoid-gpu-hybrid-active") : QStringLiteral("cardwire-plasmoid-gpu-hybrid");
        case 3: // Smart
            return dGpuActive ? QStringLiteral("cardwire-plasmoid-gpu-smart-active") : QStringLiteral("cardwire-plasmoid-gpu-smart");
        case 2: // Manual
        default:
            return dGpuActive ? QStringLiteral("cardwire-plasmoid-gpu-manual-active") : QStringLiteral("cardwire-plasmoid-gpu-manual");
    }
}

bool CardwireApplet::isCharging() const {
    if (!battery) return false;
    return battery->chargeState() != Solid::Battery::Discharging;
}

bool CardwireApplet::autoApplyGpuState() const {
    return DaemonController::from().autoApplyGpuState();
}

void CardwireApplet::setAutoApplyGpuState(bool state) {
    DaemonController::from().setAutoApplyGpuState(state);
}

bool CardwireApplet::experimentalNvidiaBlock() const {
    return DaemonController::from().experimentalNvidiaBlock();
}

void CardwireApplet::setExperimentalNvidiaBlock(bool state) {
    DaemonController::from().setExperimentalNvidiaBlock(state);
}

bool CardwireApplet::batteryAutoSwitch() const {
    return DaemonController::from().batteryAutoSwitch();
}

void CardwireApplet::setBatteryAutoSwitch(bool state) {
    DaemonController::from().setBatteryAutoSwitch(state);
}

quint32 CardwireApplet::batteryAutoSwitchMode() const {
    return DaemonController::from().batteryAutoSwitchMode();
}

void CardwireApplet::setBatteryAutoSwitchMode(quint32 mode) {
    DaemonController::from().setBatteryAutoSwitchMode(mode);
}

void CardwireApplet::setMode(quint32 mode) {
    DaemonController::from().setMode(mode);
}

void CardwireApplet::refreshDevices() {
    DaemonController::from().refreshDevices();
}

K_PLUGIN_CLASS(CardwireApplet)

#include "CardwireApplet.moc"
