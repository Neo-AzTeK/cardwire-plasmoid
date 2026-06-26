#include "CardwireGpu.h"
#include "DaemonController.h"

CardwireGpu::CardwireGpu(int id, const QString &name, const QString &pci, bool isDefault, bool isBlocked, const QString &powerState, QObject *parent)
    : QObject(parent), m_id(id), m_name(name), m_pci(pci), m_isDefault(isDefault), m_isBlocked(isBlocked), m_powerState(powerState) {}

void CardwireGpu::setBlocked(bool blocked) {
    if (m_isBlocked != blocked) {
        m_isBlocked = blocked;
        emit blockedChanged(m_isBlocked);
        DaemonController::from().setGpuBlocked(m_id, blocked);
    }
}

void CardwireGpu::updateBlocked(bool blocked) {
    if (m_isBlocked != blocked) {
        m_isBlocked = blocked;
        emit blockedChanged(m_isBlocked);
    }
}

void CardwireGpu::updatePowerState(const QString &state) {
    QString trimmed = state.trimmed();
    if (m_powerState != trimmed) {
        m_powerState = trimmed;
        emit powerStateChanged(m_powerState);
    }
}

