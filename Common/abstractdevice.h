#ifndef ABSTRACTDEVICE_H
#define ABSTRACTDEVICE_H

#include <QObject>
#include <QSerialPort>
#include "BLE/ble.h"
#include <QByteArray>



class AbstractDevice : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief The INPUT_DEVICE enum
     * @enum INPUT_DEVICE_SERIAL_PORT
     * @enum INPUT_DEVICE_BLE
     */
    enum INPUT_DEVICE
    {
        INPUT_DEVICE_SERIAL_PORT=0xa,
        INPUT_DEVICE_BLE
    };
    Q_ENUM(INPUT_DEVICE)

    explicit AbstractDevice(AbstractDevice::INPUT_DEVICE device=AbstractDevice::INPUT_DEVICE_SERIAL_PORT,QObject *parent = nullptr);
private:
    //BLE *m_bleDevice;
    //QSerialPort *m_seriaPortDevice;
    QSerialPort::BaudRate a;

    AbstractDevice::INPUT_DEVICE m_device;
    bool m_canReadLine=false;
    int m_readLIneIndex=-1;

    QScopedPointer<QByteArray> m_dataBuffer;
    qint64 m_bytesAvailableCount=0;


public:

    ///Points to currently selected function depending upon selected device
    // bool (AbstractDevice::*canReadLine)();
    // bool (*canReadLine)();
    // QByteArray (AbstractDevice::*readLine)(qint64 maxSize);
    // qint64 (AbstractDevice::*bytesAvailable)();
    // QByteArray (AbstractDevice::*read)(qint64 maxSize);
    // qint64 (AbstractDevice::*read_b)(char *data, qint64 maxSize);
    // bool (AbstractDevice::*getChar)(char *c);
    AbstractDevice::INPUT_DEVICE currentDevice();


public slots:
    void updateInputDevice(AbstractDevice::INPUT_DEVICE device);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void serialDataRead(const QByteArray &newValue);

    qint64 read_b(char *data, qint64 maxSize);
    bool canReadLine();
    // QByteArray readLine(qint64 maxSize);
    QByteArray readLine();
    qint64 bytesAvailable();
    QByteArray read(qint64 maxSize);
    bool getChar(char *c);

signals:
    void readyRead();

private slots:
    // bool canReadLine_SerialPort();

    // bool canReadLine_BLE();

    // QByteArray readLine_SerialPort(qint64 maxSize = 0);
    // QByteArray readLine_BLE(qint64 maxSize = 0);

    // qint64 bytesAvailable_SerialPort();
    // qint64 bytesAvailable_BLE();

    // QByteArray read_SerialPort(qint64 maxSize);
    // QByteArray read_BLE(qint64 maxSize);

    // qint64 read_b_SerialPort(char *data, qint64 maxSize);
    // qint64 read_b_BLE(char *data, qint64 maxSize);

    // bool getChar_SerialPort(char *c);
    // bool getChar_BLE(char *c);


};

#endif // ABSTRACTDEVICE_H
