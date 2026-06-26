#ifndef CARDWIREGPU_H
#define CARDWIREGPU_H

#include <QObject>
#include <QString>

class CardwireGpu : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString pci READ pci CONSTANT)
    Q_PROPERTY(bool isDefault READ isDefault CONSTANT)
    Q_PROPERTY(bool isBlocked READ isBlocked WRITE setBlocked NOTIFY blockedChanged)
    Q_PROPERTY(QString powerState READ powerState NOTIFY powerStateChanged)
    Q_PROPERTY(int appCount READ appCount NOTIFY appsChanged)
    Q_PROPERTY(QString appDetails READ appDetails NOTIFY appsChanged)

public:
    CardwireGpu(int id, const QString &name, const QString &pci, bool isDefault, bool isBlocked, const QString &powerState, QObject *parent = nullptr);

    int id() const { return m_id; }
    QString name() const { return m_name; }
    QString pci() const { return m_pci; }
    bool isDefault() const { return m_isDefault; }
    bool isBlocked() const { return m_isBlocked; }
    void setBlocked(bool blocked);
    void updateBlocked(bool blocked);

    QString powerState() const { return m_powerState; }
    void updatePowerState(const QString &state);

    int appCount() const { return m_appCount; }
    QString appDetails() const { return m_appDetails; }
    void updateApps(int count, const QString &details);

signals:
    void blockedChanged(bool blocked);
    void powerStateChanged(const QString &state);
    void appsChanged();

private:
    int m_id;
    QString m_name;
    QString m_pci;
    bool m_isDefault;
    bool m_isBlocked;
    QString m_powerState;
    int m_appCount = 0;
    QString m_appDetails;
};

#endif // CARDWIREGPU_H
