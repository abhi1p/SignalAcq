#include "ble_config.h"
#include "ui_ble_config.h"
#include <QThread>

BLE_Config::BLE_Config(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BLE_Config)
{
    ui->setupUi(this);

    // m_bleDevice=device;
    // this->bpsLabel=bpsLabel;
    // connect(this,&BLE_Config::refreshBLEdevicesSignal,m_bleDevice,&BLE::startBLEdiscovery,Qt::QueuedConnection);
    // connect(this,&BLE_Config::connectToBleDeviceSignal,m_bleDevice,&BLE::connectToBleDevice,Qt::QueuedConnection);
    // connect(this,&BLE_Config::readCharacteristicsSignal,m_bleDevice,&BLE::readCharacteristics,Qt::QueuedConnection);
    // connect(this,&BLE_Config::writeCharacteristicsSignal,m_bleDevice,&BLE::writeCharacteristics,Qt::QueuedConnection);
    // connect(this,&BLE_Config::enableNotificationSignal,m_bleDevice,&BLE::enableNotification,Qt::QueuedConnection);
    // connect(this,&BLE_Config::enableNotificationSignal,this,&BLE_Config::enableCharNotification,Qt::QueuedConnection);
    // connect(m_bleDevice,&BLE::bleScanFinished,this,&BLE_Config::stopRefreshLoadingIndicator,Qt::QueuedConnection);
    // connect(m_bleDevice,&BLE::addDiscoveredBLEdevicesSignal,this,&BLE_Config::addDiscoveredBLEdevices,Qt::QueuedConnection);
    // connect(m_bleDevice,&BLE::bleDeviceConnectedSignal,this,&BLE_Config::bleDeviceConnected,Qt::QueuedConnection);

    // connect(m_bleDevice,&BLE::addCharacteristicsSignal,this,&BLE_Config::addCharacteristics,Qt::QueuedConnection);
    // connect(m_bleDevice,&BLE::readCharacteristicsSuccessSignal,this,&BLE_Config::readCharacteristicsSuccess,Qt::QueuedConnection);
    // connect(m_bleDevice,&BLE::characteristicChangedSignal,this,&BLE_Config::characteristicChanged,Qt::QueuedConnection);

    connect(this,&BLE_Config::bleDeviceConnectedSignal,this,&BLE_Config::bleDeviceConnected,Qt::QueuedConnection);


    m_refreshBleSpinner=new WaitingSpinnerWidget(ui->refreshBLEdevices);
    m_connectingBleSpinner=new WaitingSpinnerWidget(ui->BLEconnectBtn);
    m_bleDevicesListViewModel=new BluetoothDeviceModel;
    // m_serialPortListModel=new SerialPortModel(this);
    ui->availabelBLEdevicesListView->setModel(m_bleDevicesListViewModel);

    qInfo()<<"Current  ble config thread:"<<QThread::currentThread();
    // m_charUiList.reset(new QList<bleChar*>);
    m_charUiMap.reset(new QHash<QBluetoothUuid,bleChar*>);

    connect(ui->BLEconnectBtn,&QToolButton::clicked,this,&BLE_Config::connectToBleDevice,Qt::QueuedConnection);
    connect(ui->availabelBLEdevicesListView,&QListView::clicked,this,&BLE_Config::onBleDeviceSelection,Qt::QueuedConnection);

    // m_speedTimer.setInterval(1000);
    // connect(&m_speedTimer,&QTimer::timeout,this,&BLE_Config::speedTimerExpired,Qt::QueuedConnection);
}

BLE_Config::~BLE_Config()
{
    disconnect(this,0,0,0);
    delete m_refreshBleSpinner;
    delete m_connectingBleSpinner;
    delete m_bleDevicesListViewModel;
    // delete m_serialPortListModel;
    qDeleteAll(*m_charUiMap);
    delete ui;
}

void BLE_Config::on_refreshBLEdevices_clicked()
{
    emit refreshBLEdevicesSignal();
    qInfo()<<"Refresh BLE device clicked";
    m_bleDevicesListViewModel->clear();
    startRefreshLoadingIndicator();
}

void BLE_Config::startRefreshLoadingIndicator()
{
    m_refreshBleSpinner->start();
    ui->BLEconnectBtn->setEnabled(false);
    qInfo()<<"Current  ble config thread:"<<QThread::currentThread();

}

void BLE_Config::stopRefreshLoadingIndicator()
{
    m_refreshBleSpinner->stop();
    qInfo()<<"Current  ble config thread:"<<QThread::currentThread();
}

void BLE_Config::startConnectionLoadingIndicator()
{
    m_connectingBleSpinner->start();
}

void BLE_Config::stopConnectionLoadingIndicator()
{
    m_connectingBleSpinner->stop();
}

void BLE_Config::addDiscoveredBLEdevices(const QBluetoothDeviceInfo &deviceInfo)
{
    m_bleDevicesListViewModel->addDevice(deviceInfo);
}

void BLE_Config::bleDeviceConnected(bool connected)
{
    m_bleDEviceConnected=connected;
    stopConnectionLoadingIndicator();
    if(connected)
    {
        // m_bleDEviceConnected=true;
        qInfo()<<"BLE device connected";
        disableBleDeviceList();
        ui->BLEconnectBtn->setText("Disconnect");
        removeAllCharacteristics();
    }
    else
    {
        // m_bleDEviceConnected=false;
        qInfo()<<"BLE device disconnected";
        enableDeviceList();
        ui->BLEconnectBtn->setText("Connect");
    }
}

void BLE_Config::disableBleDeviceList()
{
    ui->availabelBLEdevicesListView->setEnabled(false);
    ui->refreshBLEdevices->setEnabled(false);
    // ui->BLEconnectBtn->setEnabled(false);//not required now
}

void BLE_Config::enableDeviceList()
{
    ui->availabelBLEdevicesListView->setEnabled(true);
    ui->refreshBLEdevices->setEnabled(true);
    // ui->BLEconnectBtn->setEnabled(true); //not required now
}

void BLE_Config::addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag)
{
    bleChar* charTab=new bleChar(ui->addCharWidget, srvcUuid, charUuid, flag);
    connect(charTab,&bleChar::readCharacteristicsSignal,this,&BLE_Config::readCharacteristicsSignal,Qt::QueuedConnection);
    connect(charTab,&bleChar::writeCharacteristicsSignal,this,&BLE_Config::writeCharacteristicsSignal,Qt::QueuedConnection);
    connect(charTab,&bleChar::enableNotificationSignal,this,&BLE_Config::enableNotificationSignal,Qt::QueuedConnection);
    // m_charUiList->append(charTab);
    m_charUiMap->insert(charUuid,charTab);
    ui->bleaddCharTabLayout->addWidget(charTab);
}
void BLE_Config::updateCharacteristics(QBluetoothUuid uuid, const QByteArray &value)
{
    if(m_charUiMap->contains(uuid) && (value.size()<CHAR_MAX_DISPLAY_SIZE))
    {
        bleChar *charUi = m_charUiMap->value(uuid); //get charUI object by using key in a hash table
        charUi->updateValue(value);
    }
}
void BLE_Config::readCharacteristicsSuccess(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    // for (int i = 0; i < m_charUiList->size(); i++)
    // {
    //     bleChar *charTab=m_charUiList->at(i);
    //     if(charTab->isValid(characteristic.uuid()))
    //     {
    //         charTab->updateValue(value);
    //     }
    // }

    QBluetoothUuid uuid=characteristic.uuid();
    updateCharacteristics(uuid,value);

}

void BLE_Config::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    // m_totalData+=newValue.size(); //for transfer speed calculation


    QBluetoothUuid uuid=characteristic.uuid();
    updateCharacteristics(uuid,newValue);

}

void BLE_Config::removeAllCharacteristics()
{
    // for(int i=0;i<m_charUiList->size();i++)
    // {
    //     bleChar* widget=m_charUiList->at(i);
    //     disconnect(widget,0,0,0); //disconnect characteristics from all signals & slots
    //     ui->bleaddCharTabLayout->removeWidget(widget);
    //     widget->deleteLater();
    // }
    // m_charUiList->clear();

    // Iterate directly over the QHash
    for (auto it = m_charUiMap->begin(); it != m_charUiMap->end(); ++it)
    {
        bleChar* widget = it.value();

        // Disconnect the widget from all signals and slots
        disconnect(widget, 0, 0, 0);

        // Remove the widget from the layout
        ui->bleaddCharTabLayout->removeWidget(widget);

        // Schedule the widget for deletion
        widget->deleteLater();
    }

    // Clear the QHash to remove all key-value pairs
    m_charUiMap->clear();
}



// void BLE_Config::enableCharNotification(bool toEnable, QBluetoothUuid charUuid)
// {
//     if(toEnable)
//     {
//         m_totalData=0;
//         m_speedTimer.start();

//     }
//     else
//     {
//         m_speedTimer.stop();
//     }
// }

// void BLE_Config::speedTimerExpired()
// {
//     // qInfo()<<"Timer timeout";
//     quint64 bits=m_totalData*8;
//     QString str;
//     if(bits>1024)
//     {
//         bits/=1024;
//         str = QString(tr("%1kbps")).arg(bits);
//     }
//     else
//     {
//         str = QString(tr("%1bps")).arg(bits);
//     }

//     this->bpsLabel->setText(str);
//     m_totalData=0;
// }

void BLE_Config::connectToBleDevice()
{
    if(!m_bleDEviceConnected)
    {
        emit connectToBleDeviceSignal(m_currentSelectedDevice, true);
        disableBleDeviceList();
        startConnectionLoadingIndicator();
    }
    else
    {
        emit connectToBleDeviceSignal(m_currentSelectedDevice, false);
    }
}

void BLE_Config::onBleDeviceSelection(const QModelIndex &index)
{
    m_currentSelectedDevice=m_bleDevicesListViewModel->getDeviceInfo(index);
    qInfo()<<"Selected device:"<<m_currentSelectedDevice.name();
    ui->BLEconnectBtn->setEnabled(true);
}

