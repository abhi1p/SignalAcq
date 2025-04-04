#include "ble.h"

BLE::BLE(QObject *parent)
    : QObject{parent}
{
    qInfo()<<"Current BLE thread:"<<QThread::currentThread();
    //m_dataBuffer.reset(new QByteArray);
    m_bytesAvailableCount=0;
    _BLEdiscoveryAgentInit();
    m_elapsedTimer.start();

}

BLE::~BLE()
{
    if(m_BLEcontroller)
    {
        if(m_BLEcontroller->state()==QLowEnergyController::ConnectedState)
        {
            m_BLEcontroller->disconnectFromDevice();
        }
    }
    delete m_BLEdiscoveryAgent;
}

void BLE::_BLEdiscoveryAgentInit()
{
    m_BLEdiscoveryAgent=new QBluetoothDeviceDiscoveryAgent;

    m_BLEdiscoveryAgent->setLowEnergyDiscoveryTimeout(m_BLEdiscoveryTimeout);

    connect(m_BLEdiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLE::_addDevice,Qt::QueuedConnection);
    connect(m_BLEdiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,this, &BLE::_scanError,Qt::QueuedConnection);

    connect(m_BLEdiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLE::_scanFinished,Qt::QueuedConnection);
    connect(m_BLEdiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BLE::_scanFinished,Qt::QueuedConnection);
    // connect(m_BLEdiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BLE::_scanFinished,Qt::QueuedConnection);

}


void BLE::startBLEdiscovery()
{

    m_BLEdiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    qInfo()<<"Starting BLE discovery in BLE class";
}


void BLE::_addDevice(const QBluetoothDeviceInfo &info)
{
    qInfo()<<"Name: "<<info.name()<<" Address: "<<info.address();
    emit addDiscoveredBLEdevicesSignal(info);
}

void BLE::_scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    // qInfo()<<error;
    if(error==QBluetoothDeviceDiscoveryAgent::PoweredOffError)
    {
        qInfo()<<"Bluetooth disabled";
    }
    else if(error==QBluetoothDeviceDiscoveryAgent::MissingPermissionsError)
    {
        qInfo()<<"Bluetooth permisssion denied";
    }
    emit bleScanFinished();
}

void BLE::_scanFinished()
{
    qInfo()<<"BLE scan finished";
    emit bleScanFinished();
    qInfo()<<"Current  BLE thread:"<<QThread::currentThread();
}



void BLE::bleDiscoveryErrorOccurred(QLowEnergyController::Error newError)
{
    if(newError==QLowEnergyController::ConnectionError)
    {
        //failed to connect to device
        qInfo()<<"Filed to connect to ble device";
        emit bleDeviceConnectedSignal(false);
    }
    else if(newError==QLowEnergyController::RemoteHostClosedError)
    {
        qInfo()<<"The remote device closed the connection";
        emit bleDeviceConnectedSignal(false);

    }
    else if(newError==QLowEnergyController::NetworkError)
    {
        //network error
        qInfo()<<"Network error";
    }
    else
    {
        qInfo()<<"Error occured"<<newError;
    }
}

void BLE::bleDeviceConnected()
{
    emit bleDeviceConnectedSignal(true);
    m_Tdata=0;
    qInfo()<<"BLE device connected";
    // if(!m_discoveredServicesMap.isNull())
    // {
    //     qDeleteAll(*m_discoveredServicesMap);
    // }
    qInfo()<<"MTU size: "<<m_BLEcontroller->mtu();

    ///set connection parameters
    // Define connection parameters
    QLowEnergyConnectionParameters params;
    params.setIntervalRange(30,50);
    params.setLatency(0);           // Slave latency
    params.setSupervisionTimeout(5000);
    m_BLEcontroller->requestConnectionUpdate(params);

    m_discoveredServices.reset(new QList<QBluetoothUuid>); //allocate memory for storing discovered services
    m_discoveredServicesMap.reset(new QHash<QBluetoothUuid,QSharedPointer<QLowEnergyService>>); //allocate memory for storing discovered services

    connect(this,&BLE::discoverServiceDetailsSignal,this,&BLE::discoverServiceDetails,Qt::QueuedConnection);
    m_BLEcontroller->discoverServices();

}

void BLE::bleDeviceDisconnected()
{
    emit bleDeviceConnectedSignal(false);

    m_discoveredServices.clear();
    // qDeleteAll(*m_discoveredServicesMap);
    m_discoveredServicesMap->clear();

    //disconnect(this,&BLE::discoverServiceDetailsSignal,this,&BLE::discoverServiceDetails);
}

void BLE::bleCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{

    auto data = reinterpret_cast<const quint8 *>(newValue.constData());
    quint8 flags = *data;
#if 1
    qInfo()<<"Characreristics changed val[0]: "<<Qt::hex<<data[0]<<"val[1]"<<Qt::hex<<data[1];
#else
    //Heart Rate
    int hrvalue = 0;
    if (flags & 0x1) // HR 16 bit? otherwise 8 bit
        hrvalue = static_cast<int>(qFromLittleEndian<quint16>(data[1]));
    else
        hrvalue = static_cast<int>(data[1]);

    qInfo()<<"Characreristics changed: "<<hrvalue;
#endif
#
}



void BLE::bleServiceDiscovered(const QBluetoothUuid &newServiceUUID)
{
    qInfo()<<"Service found:"<<newServiceUUID.toString();

    /*
    if(newServiceUUID==QBluetoothUuid(m_serviceUUID))
    // if(newService==QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::HeartRate))
    {
        m_batteryServiceDiscovered=true;
        qInfo()<<"Service discoverd";
    }
    else if(newServiceUUID==QBluetoothUuid(m_serviceUUID2))
    {
        m_normalServiceDiscovered=true;
    }
    */

    // ///m_prSrvc1 and m_prSrvc2 are default generic access service and genric attribute service, which we do not want
    // if((newServiceUUID!=QBluetoothUuid(m_prSrvc1)) && (newServiceUUID!=QBluetoothUuid(m_prSrvc2)))
    // {
    //     m_discoveredServices->append(newServiceUUID);
    // }



}

void BLE::bleServiceDiscoveryFinished()
{
    /*
    if(m_batteryServiceDiscovered)
    {
        m_BLEservice.reset(m_BLEcontroller->createServiceObject(QBluetoothUuid(m_serviceUUID)),&QObject::deleteLater);
        // m_BLEservice.reset(m_BLEcontroller->createServiceObject(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::HeartRate)),&QObject::deleteLater);

        if(!m_BLEservice.isNull())
        {
            connect(m_BLEservice.data(), &QLowEnergyService::stateChanged, this, &BLE::bleServiceStateChanged,Qt::QueuedConnection);
            connect(m_BLEservice.data(), &QLowEnergyService::characteristicChanged, this, &BLE::bleCharacteristicChanged,Qt::QueuedConnection);
            connect(m_BLEservice.data(), &QLowEnergyService::descriptorWritten, this, &BLE::bleDescriptorWritten,Qt::QueuedConnection);
            m_BLEservice->discoverDetails();
        }
        else
        {
            qInfo()<<"Service not available";
        }

    }
    */
    /*if(m_normalServiceDiscovered)
    {
        qInfo()<<"Normal service discovered";
        QLowEnergyService *bleService=m_BLEcontroller->createServiceObject(QBluetoothUuid(m_serviceUUID2));
        connect(bleService, &QLowEnergyService::stateChanged, this, &BLE::bleServiceStateChangedNormal,Qt::QueuedConnection);
        bleService->discoverDetails();
    }*/

#if 1
    m_serviceCount=0;
    // discoverServiceDetails();

    const QList<QBluetoothUuid> discoveredServices = m_BLEcontroller->services(); //get all discovered services

    for (const QBluetoothUuid &serviceUuid : discoveredServices)
    {
        ///m_prSrvc1 and m_prSrvc2 are default generic access service and genric attribute service, which we do not want
        if((serviceUuid!=QBluetoothUuid(m_prSrvc1)) && (serviceUuid!=QBluetoothUuid(m_prSrvc2)))
        {
            m_discoveredServices->append(serviceUuid); //append discovered service
        }
    }

    emit discoverServiceDetailsSignal();
#else
    qInfo()<<"Service discovery finished";
#endif

}

void BLE::discoverServiceDetails()
{
    // if(!m_BLEservice.isNull())
    // {
    //     disconnect(m_BLEservice,0,0,0)
    // }
    m_BLEservice.reset(m_BLEcontroller->createServiceObject(m_discoveredServices.data()->at(m_serviceCount)),&QObject::deleteLater);
    if(!m_BLEservice.isNull())
    {
        connect(m_BLEservice.data(), &QLowEnergyService::stateChanged, this, &BLE::bleServiceStateChanged,Qt::QueuedConnection);
        connect(m_BLEservice.data(), &QLowEnergyService::characteristicRead, this, &BLE::readCharacteristicsSuccessSignal,Qt::QueuedConnection);
        connect(m_BLEservice.data(), &QLowEnergyService::errorOccurred, this, &BLE::bleServiceErrorOccurred,Qt::QueuedConnection);
        connect(m_BLEservice.data(), &QLowEnergyService::characteristicWritten, this, &BLE::characteristicWritten,Qt::QueuedConnection);

        /*
         * Connect characteristics changed signal to characteristics viewer
        */
        connect(m_BLEservice.data(), &QLowEnergyService::characteristicChanged, this, &BLE::characteristicChangedSignal,Qt::QueuedConnection); //signal to signal

        qInfo()<<"Counting:"<<m_serviceCount;
        m_BLEservice->discoverDetails();
    }
}

void BLE::bleServiceStateChanged(QLowEnergyService::ServiceState newState)
{
    switch (newState)
    {
        case QLowEnergyService::RemoteServiceDiscovering:
        {

        qInfo()<<"Service discovering...";
            break;
        }

        case QLowEnergyService::RemoteServiceDiscovered:
        {

            //const QLowEnergyCharacteristic hrChar =m_BLEservice->characteristic(QBluetoothUuid(m_charUUID));


            // const QLowEnergyCharacteristic hrChar =m_BLEservice->characteristic(QBluetoothUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement));

            //print all the charateristics uuid
            qInfo()<<"Characteristics for service "<<m_BLEservice->serviceUuid();
            QList<QLowEnergyCharacteristic> chars=m_BLEservice->characteristics();
            for(int i=0;i<chars.length();i++)
            {
                qInfo()<<"Characteristics:"<<chars[i].name()<<" "<<chars[i].uuid()<<" "<<chars[i].properties();
                m_discoveredServicesMap->insert(chars[i].uuid(),m_BLEservice);
                emit addCharacteristicsSignal(m_BLEservice->serviceUuid(),chars[i].uuid(),chars[i].properties());
            }

            /*
            // if()
            if (!hrChar.isValid())
            {
                qInfo()<<"Characteristics not available";
            }
            else
            {
                qInfo()<<"Characteristics discovered"<<"Service ";



                //Enable notification
                QLowEnergyDescriptor notificationDesc=hrChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                if(notificationDesc.isValid())
                {
                    m_BLEservice->writeDescriptor(notificationDesc,QByteArray::fromHex("0100"));

                }



            }
            */


            disconnect(m_BLEservice.data(), &QLowEnergyService::stateChanged, this, &BLE::bleServiceStateChanged);
            // disconnect(m_BLEservice.data(), 0, 0, 0);
            m_serviceCount++;
            if(m_serviceCount<m_discoveredServices->size())
            {
                emit discoverServiceDetailsSignal();
            }
            break;
        }
        default:
        {
            //nothing for now
            break;
        }
        }
}

// void BLE::bleConnectionUpdated(const QLowEnergyConnectionParameters &newParameters)
// {
//     qInfo()<<"Min Connection interval: "<<newParameters.minimumInterval();
//     qInfo()<<"MAx Connection interval: "<<newParameters.maximumInterval();
//     qInfo()<<"Slave latency: "<<newParameters.latency();
// }

// void BLE::bleServiceStateChangedNormal(QLowEnergyService::ServiceState newState)
// {
//     switch (newState)
//     {
//         case QLowEnergyService::RemoteServiceDiscovered:
//         {
//             qInfo()<<"Normal service details discovered";
//             QLowEnergyService *bleService=m_BLEcontroller->createServiceObject(QBluetoothUuid(m_serviceUUID2));
//             const QLowEnergyCharacteristic hrChar2 =bleService->characteristic(QBluetoothUuid(m_charUUID2));
//             bleService->writeCharacteristic(hrChar2,QByteArray::fromStdString("y"), QLowEnergyService::WriteWithoutResponse);
//             break;
//         }
//         default:
//         {
//             break;
//         }

//     }

// }

void BLE::bleDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    if(descriptor.isValid())
    {
        qInfo()<<"Descriptor written: "<<newValue.toHex();
        // const QLowEnergyCharacteristic hrChar =m_BLEservice->characteristic(QBluetoothUuid(m_charUUID));


    }
}

void BLE::bleServiceErrorOccurred(QLowEnergyService::ServiceError newError)
{
    qInfo()<<newError;
}



void BLE::stateChanged(QLowEnergyController::ControllerState state)
{
    if(state==QLowEnergyController::ConnectingState)
    {
        qInfo()<<"Connecting..";
    }
    else if(state==QLowEnergyController::ConnectedState)
    {
        qInfo()<<"Connected to BLE device";
    }
    else if(state==QLowEnergyController::ClosingState)
    {
        qInfo()<<"Disconnecting..";
    }
}

void BLE::selectBleDevice(QBluetoothDeviceInfo bleDeviceInfo)
{
    // QLowEnergyController *controller=
    // qInfo()<<"BLE device selected: "<<bleDeviceInfo.name();
    // if(!m_BLEcontroller.isNull())
    // {
    //     disconnect(m_BLEcontroller.data(), 0, 0, 0);
    // }
    // qInfo()<<"Select BLE deivice";
    m_BLEcontroller.reset(QLowEnergyController::createCentral(bleDeviceInfo),&QObject::deleteLater);
    connect(m_BLEcontroller.data(), &QLowEnergyController::serviceDiscovered,this, &BLE::bleServiceDiscovered,Qt::QueuedConnection);
    connect(m_BLEcontroller.data(), &QLowEnergyController::discoveryFinished,this, &BLE::bleServiceDiscoveryFinished,Qt::QueuedConnection);
    connect(m_BLEcontroller.data(), &QLowEnergyController::errorOccurred,this, &BLE::bleDiscoveryErrorOccurred,Qt::QueuedConnection);
    connect(m_BLEcontroller.data(), &QLowEnergyController::connected,this, &BLE::bleDeviceConnected,Qt::QueuedConnection);
    connect(m_BLEcontroller.data(), &QLowEnergyController::disconnected,this, &BLE::bleDeviceDisconnected,Qt::QueuedConnection);
    // connect(m_BLEcontroller.data(), &QLowEnergyController::connectionUpdated,this, &BLE::bleConnectionUpdated,Qt::QueuedConnection);
    connect(m_BLEcontroller.data(), &QLowEnergyController::stateChanged,this, &BLE::stateChanged,Qt::QueuedConnection);
    connect(m_BLEcontroller.data(), &QLowEnergyController::connectionUpdated,this, &BLE::connectionUpdated,Qt::QueuedConnection);


}

void BLE::connectToBleDevice(QBluetoothDeviceInfo device, bool toConnect)
{
    if(toConnect)
    {
        //connect to ble device
        selectBleDevice(device);
        m_BLEcontroller->connectToDevice();
    }
    else
    {
        //disconnect to ble device
        m_BLEcontroller->disconnectFromDevice();
    }

}

void BLE::characteristicChangedPlot(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    if(characteristic.uuid()==m_prevSelectedUuid)
    {
        emit characteristicChangedSignal_plot(characteristic,newValue);
    }
}

void BLE::selectedSourceCharacteristic(QString uuid)
{
    qInfo()<<"Selected source char";
    ///disconnect previous connection to plot
    QSharedPointer<QLowEnergyService> prevServc=m_discoveredServicesMap->value(m_prevSelectedUuid,nullptr); //get the previous BLE service
    if((!prevServc.isNull()) /*&& (!m_prevSelectedUuid.isNull()) */)
    {
        disconnect(prevServc.data(),&QLowEnergyService::characteristicChanged,this,&BLE::characteristicChangedPlot);
    }


    QBluetoothUuid currentSelectedUuid=QBluetoothUuid(uuid);
    if(!currentSelectedUuid.isNull())
    {
        // QBluetoothUuid currentSelectedUuid=uuid;
        // if(m_discoveredServicesMap->contains(currentSelectedUuid))
        // {
        QSharedPointer<QLowEnergyService> cServc=m_discoveredServicesMap->value(currentSelectedUuid); //get the current BLE service

        /*
    * Connect characteristics changed signal to plotter
    */
        connect(cServc.data(),&QLowEnergyService::characteristicChanged,this,&BLE::characteristicChangedPlot,Qt::QueuedConnection);
        // }
    }

    m_prevSelectedUuid=currentSelectedUuid;
}



void BLE::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qInfo()<<"data sent";
}

void BLE::connectionUpdated(const QLowEnergyConnectionParameters &newParameters)
{
    qInfo()<<"<--Connection parameter updated-->";
    qInfo()<<"Max Connection interval: "<<newParameters.maximumInterval();
    qInfo()<<"Min Connection interval: "<<newParameters.maximumInterval();
    qInfo()<<"Latency: "<<newParameters.latency();
    qInfo()<<"Supervision timeout: "<<newParameters.supervisionTimeout();
}



void BLE::readCharacteristics(QBluetoothUuid charUuid)
{
    qInfo()<<"Read characteristics for Char:"<<charUuid;
    // qInfo()<<"Read characteristics for Srvc:"<<srvcUuid<<" Char:"<<charUuid;
    // m_BLEservice.reset(m_BLEcontroller->createServiceObject(srvcUuid),&QObject::deleteLater);
    // const QLowEnergyCharacteristic hrChar=m_BLEservice->characteristic(charUuid);
    // qInfo()<<"Service Uuid"<<m_BLEservice->serviceUuid();
    // qInfo()<<"Char Properties:"<<hrChar.properties();
    // connect(m_BLEservice.data(), &QLowEnergyService::errorOccurred, this, &BLE::bleServiceErrorOccurred,Qt::QueuedConnection);
    // connect(m_BLEservice.data(), &QLowEnergyService::characteristicRead, this, &BLE::readCharacteristicsSuccessSignal,Qt::QueuedConnection);
    // if(hrChar.isValid())
    // {
    //     m_BLEservice->readCharacteristic(hrChar);
    //     qInfo()<<"Valid characteristics";
    // }
    // else
    // {
    //     qInfo()<<"Invalid characteristics";
    // }

    if(m_discoveredServicesMap->contains(charUuid))
    {
        QSharedPointer<QLowEnergyService> servc=m_discoveredServicesMap->value(charUuid);
        const QLowEnergyCharacteristic hrChar=servc->characteristic(charUuid);
        servc->readCharacteristic(hrChar);
    }

}

void BLE::writeCharacteristics(QBluetoothUuid charUuid, QByteArray text)
{
    if(m_discoveredServicesMap->contains(charUuid))
    {
        QSharedPointer<QLowEnergyService> servc=m_discoveredServicesMap->value(charUuid);
        const QLowEnergyCharacteristic hrChar=servc->characteristic(charUuid);
        servc->writeCharacteristic(hrChar,text,QLowEnergyService::WriteWithoutResponse);
        // servc->writeCharacteristic(hrChar,text.toUtf8(),QLowEnergyService::WriteWithoutResponse);
    }
}

void BLE::enableNotification(bool toEnable, QBluetoothUuid charUuid)
{
    if(m_discoveredServicesMap->contains(charUuid))
    {
        QSharedPointer<QLowEnergyService> servc=m_discoveredServicesMap->value(charUuid);

        const QLowEnergyCharacteristic hrChar=servc->characteristic(charUuid);
        QLowEnergyDescriptor notificationDesc = hrChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

        if(notificationDesc.isValid()) //check if descriptor is valid
        {
            if(toEnable)
            {
                servc->writeDescriptor(notificationDesc,QLowEnergyCharacteristic::CCCDEnableNotification); //enable notification
#ifdef QT_DEBUG
                // qInfo()<<"Servc Uuid: "<<serv.toString();
                qInfo()<<"Char Uuid: "<<charUuid.toString();
                qInfo()<<"Notification enabled";
#endif
            }
            else
            {
               servc->writeDescriptor(notificationDesc,QLowEnergyCharacteristic::CCCDDisable);//disable notification and indication
#ifdef QT_DEBUG
                qInfo()<<"Notification disabled";
#endif
            }

        }
    }
}


