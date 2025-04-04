#ifndef BLE_H
#define BLE_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QDebug>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include <QtEndian>
#include <QPair>
#include <QThread>
#include <QElapsedTimer>
#include <QTimer>

typedef struct
{
    quint8 header[4];
    qint32 data[4];
}Signal_t;

class BLE : public QObject
{
    Q_OBJECT
public:
    explicit BLE(QObject *parent = nullptr);
    ~BLE();

private:
    int m_BLEdiscoveryTimeout=5000;     //mS
    QBluetoothDeviceDiscoveryAgent *m_BLEdiscoveryAgent;
    QSharedPointer<QLowEnergyController> m_BLEcontroller;
    QSharedPointer<QLowEnergyService> m_BLEservice;
    // QSharedPointer<QLowEnergyService> m_BLEChar;
    QSharedPointer<QList<QBluetoothUuid>> m_discoveredServices;
    QSharedPointer<QPair<QBluetoothUuid,QList<QBluetoothUuid>>> m_srvcAndChars;
    QScopedPointer<QHash<QBluetoothUuid,QSharedPointer<QLowEnergyService>>> m_discoveredServicesMap;
    // bool m_remoteServiceDiscovered=true;
    bool m_normalServiceDiscovered=false;
    quint16 m_prSrvc1=0x1801;
    quint16 m_prSrvc2=0x1800;
    quint16 m_serviceUUID2=0x2343;
    quint16 m_serviceUUID=0x1234;
    quint16 m_charUUID2=0x1010;
    quint16 m_charUUID=0x2a19;

    int m_serviceCount=0;
    QBluetoothUuid m_prevSelectedUuid;

    // QScopedPointer<QByteArray> m_dataBuffer;
    qint64 m_bytesAvailableCount;
    QElapsedTimer m_elapsedTimer;
    quint64 m_Tdata=0;
    //QTimer m_speedTimer;

protected:
    // virtual unsigned readData() = 0;

private slots:
    void _BLEdiscoveryAgentInit();
    void _addDevice(const QBluetoothDeviceInfo &info);
    void _scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void _scanFinished();
    void stateChanged(QLowEnergyController::ControllerState state);
    void bleServiceDiscovered(const QBluetoothUuid &newServiceUUID);
    void bleServiceDiscoveryFinished();
    void discoverServiceDetails();
    void bleDiscoveryErrorOccurred(QLowEnergyController::Error newError);
    void bleDeviceConnected();
    void bleDeviceDisconnected();
    void bleCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void bleServiceStateChanged(QLowEnergyService::ServiceState newState);
    // void bleServiceStateChangedNormal(QLowEnergyService::ServiceState newState);
    void bleDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);

    void bleServiceErrorOccurred(QLowEnergyService::ServiceError newError);

    // void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void connectionUpdated(const QLowEnergyConnectionParameters &newParameters);
    void characteristicChangedPlot(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

public slots:
    void startBLEdiscovery();
    void selectBleDevice(QBluetoothDeviceInfo bleDeviceInfo);
    void connectToBleDevice(QBluetoothDeviceInfo device, bool toConnect);

    void selectedSourceCharacteristic(QString uuid);

    void readCharacteristics(QBluetoothUuid charUuid);
    void writeCharacteristics(QBluetoothUuid charUuid, QByteArray text);
    void enableNotification(bool toEnable, QBluetoothUuid charUuid);

    // bool canReadLine();
    // QByteArray readLine(qint64 maxSize);
    // qint64 bytesAvailable();
    // QByteArray read(qint64 maxSize);
    // qint64 read_b(char *data, qint64 maxSize);
    // bool getChar(char *c);

signals:
    void addDiscoveredBLEdevicesSignal(const QBluetoothDeviceInfo &deviceInfo);
    void bleDeviceConnectedSignal(bool connected);

    void bleScanFinished();

    void discoverServiceDetailsSignal();

    void addCharacteristicsSignal(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag);

    void readCharacteristicsSuccessSignal(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void characteristicChangedSignal(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicChangedSignal_plot(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void readyRead();

};

#endif // BLE_H
