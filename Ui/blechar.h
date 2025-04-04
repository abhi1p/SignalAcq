#ifndef BLECHAR_H
#define BLECHAR_H

#include <QFrame>
#include <QLowEnergyCharacteristic>
#include <QBluetoothUuid>

namespace Ui {
class bleChar;
}

class bleChar : public QFrame
{
    Q_OBJECT

public:
    explicit bleChar(QWidget *parent, QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag);
    ~bleChar();

private:
    Ui::bleChar *ui;
    QBluetoothUuid m_srvcUuid;
    QBluetoothUuid m_charUuid;
    quint16 m_batteryLevelStatusUuid=0x2bed;
    quint16 m_batteryVoltageUuid=0x2cf1;
    quint16 m_batteryCurrentUuid=0x2df2;
    // quint16 m_batteryLevelStatusUuid=0x2bed;
    QLowEnergyCharacteristic::PropertyTypes m_flag;

private slots:
    void readCharacteristics();
    void writeCharacteristics();
    void enableNotification(int state);
public slots:
    void updateValue(const QByteArray &value);
    bool isValid(QBluetoothUuid charUuid);

signals:
    void readCharacteristicsSignal(QBluetoothUuid charUuid);
    void writeCharacteristicsSignal(QBluetoothUuid charUuid, QByteArray text);
    void enableNotificationSignal(bool toEnable, QBluetoothUuid charUuid);

};

#endif // BLECHAR_H
