#ifndef BLE_CONFIG_H
#define BLE_CONFIG_H

#include <QWidget>
#include <QStandardItemModel>
#include <QBluetoothDeviceInfo>
#include <QSerialPortInfo>
#include "Plot/waitingspinnerwidget.h"
#include <QLowEnergyCharacteristic>
#include "blechar.h"
#include <QScopedPointer>

#include "device_model.h"
#include <QHash>
#include "BLE/ble.h"
#include <QTimer>
// #include "bpslabel.h"

namespace Ui {
class BLE_Config;
}



class BLE_Config : public QWidget
{
    Q_OBJECT

public:
    explicit BLE_Config(QWidget *parent = nullptr);
    // explicit BLE_Config(BPSLabel *bpsLabel, QWidget *parent = nullptr);
    ~BLE_Config();

private slots:
    void on_refreshBLEdevices_clicked();


private:
    Ui::BLE_Config *ui;

    // BLE *m_bleDevice;
    BluetoothDeviceModel *m_bleDevicesListViewModel;
    // SerialPortModel *m_serialPortListModel;
    double m_samplingRate=1;
    WaitingSpinnerWidget* m_refreshBleSpinner;
    WaitingSpinnerWidget* m_connectingBleSpinner;

    bool m_bleDEviceConnected=false;
    QBluetoothDeviceInfo m_currentSelectedDevice;
    // QScopedPointer<QList<bleChar*>> m_charUiList;
    QScopedPointer<QHash<QBluetoothUuid,bleChar*>> m_charUiMap;
    // QScopedPointer<bleChar*> m_bleChar;

    // quint64 m_totalData=0;
    // QTimer m_speedTimer;
    // BPSLabel *bpsLabel;

signals:
    void refreshBLEdevicesSignal();
    void connectToBleDeviceSignal(QBluetoothDeviceInfo device, bool toConnect);

    void readCharacteristicsSignal(QBluetoothUuid charUuid);
    void writeCharacteristicsSignal(QBluetoothUuid charUuid, QByteArray text);
    void enableNotificationSignal(bool toEnable, QBluetoothUuid charUuid);
    void bleDeviceConnectedSignal(bool connected);


public slots:
    void startRefreshLoadingIndicator();
    void stopRefreshLoadingIndicator();
    void startConnectionLoadingIndicator();
    void stopConnectionLoadingIndicator();

    void addDiscoveredBLEdevices(const QBluetoothDeviceInfo &deviceInfo);
    void bleDeviceConnected(bool connected);
    void disableBleDeviceList();
    void enableDeviceList();
    void addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag);
    void readCharacteristicsSuccess(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);



private slots:
    void connectToBleDevice();
    void onBleDeviceSelection(const QModelIndex &index);
    void removeAllCharacteristics();
    // void enableCharNotification(bool toEnable, QBluetoothUuid charUuid);
    // void speedTimerExpired();
    void updateCharacteristics(QBluetoothUuid uuid, const QByteArray &value);
};

#endif // BLE_CONFIG_H
