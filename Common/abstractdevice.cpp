#include "abstractdevice.h"

AbstractDevice::AbstractDevice(AbstractDevice::INPUT_DEVICE device,QObject *parent)
    : QObject{parent},m_device(device)
{
    //m_device=device;
    //m_bleDevice=bleDevice;
    //m_seriaPortDevice=serialPortDEvice;
    // canReadLine=&AbstractDevice::canReadLine_BLE;
    m_dataBuffer.reset(new QByteArray);
    updateInputDevice(device);
}


void AbstractDevice::updateInputDevice(AbstractDevice::INPUT_DEVICE device)
{
    qInfo()<<"Input device update"<<device;
#if 0
    if(device==AbstractDevice::INPUT_DEVICE_BLE)
    {
        // disconnect(m_seriaPortDevice,&QSerialPort::readyRead,this,&AbstractDevice::readyRead);
        //canReadLine=&AbstractDevice::canReadLine_BLE;
        readLine=&AbstractDevice::readLine_BLE;
        read=&AbstractDevice::read_BLE;
        read_b=&AbstractDevice::read_b_BLE;
        bytesAvailable=&AbstractDevice::bytesAvailable_BLE;
        getChar=&AbstractDevice::getChar_BLE;

        // connect(m_bleDevice,&BLE::characteristicChangedSignal,this,&AbstractDevice::characteristicChanged,Qt::QueuedConnection);
    }
    else if(device==AbstractDevice::INPUT_DEVICE_SERIAL_PORT)
    {
        //canReadLine=&AbstractDevice::canReadLine_SerialPort;
        readLine=&AbstractDevice::readLine_SerialPort;
        read=&AbstractDevice::read_SerialPort;
        read_b=&AbstractDevice::read_b_SerialPort;
        bytesAvailable=&AbstractDevice::bytesAvailable_SerialPort;
        getChar=&AbstractDevice::getChar_SerialPort;

        connect(m_seriaPortDevice,&QSerialPort::readyRead,this,&AbstractDevice::readyRead,Qt::QueuedConnection);
    }
#endif
}

void AbstractDevice::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{

    // qInfo()<<"In BLE data:"<<newValue<<"Size"<<newValue.size();
    // qInfo()<<"Received data size: "<<newValue.size();
    // qInfo()<<"Elapsed time"<<m_elapsedTimer.nsecsElapsed();
    // m_Tdata+=newValue.size();
    // qInfo()<<"Total data received:"<<m_Tdata;
    // m_elapsedTimer.restart();
#if 0
    Signal_t data={{0xAA,0xBB,0xAA,0xBB},{100,200,300,400}};
    QByteArray newData=QByteArray::fromRawData((char*)&data,20);
    qInfo()<<"Data to insert: "<<newData<<"Size:"<<newData.size();
    m_dataBuffer->append(newData);
    m_bytesAvailableCount+=newData.size();
#else
    m_dataBuffer->append(newValue);
    m_bytesAvailableCount+=newValue.size();
#endif

    emit readyRead();
}

void AbstractDevice::serialDataRead(const QByteArray &newValue)
{
    m_dataBuffer->append(newValue);
    m_bytesAvailableCount+=newValue.size();
    emit readyRead();
}


// void AbstractDevice::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
// {
//     if(characteristic.uuid()==m_currentSelectedUuid) //selected characteristics
//     {

//     }
// }

// QByteArray AbstractDevice::read_SerialPort(qint64 maxSize)
// {
//     return m_seriaPortDevice->read(maxSize);
// }

// QByteArray AbstractDevice::read_BLE(qint64 maxSize)
// {
//     return m_bleDevice->read(maxSize);
//     // return QByteArray(0);

// }

// qint64 AbstractDevice::read_b_SerialPort(char *data, qint64 maxSize)
// {
//     return m_seriaPortDevice->read(data,maxSize);
// }

// qint64 AbstractDevice::read_b_BLE(char *data, qint64 maxSize)
// {
//     return m_bleDevice->read_b(data,maxSize);
// }

// bool AbstractDevice::getChar_SerialPort(char *c)
// {
//     return m_seriaPortDevice->getChar(c);
// }

// bool AbstractDevice::getChar_BLE(char *c)
// {
//     return m_bleDevice->getChar(c);
// }



bool AbstractDevice::canReadLine()
{
    int index=m_dataBuffer->indexOf('\n');

    m_readLIneIndex=index;
    if(index!=-1)
    {
        m_canReadLine=true;

    }
    else
    {
        m_canReadLine=false;
    }
    return m_canReadLine;
}

QByteArray AbstractDevice::readLine()
{
    //qInfo()<<"Newline index: "<<m_readLIneIndex;
    qint64 toRead = qMin(m_bytesAvailableCount, m_readLIneIndex+1); //should call canReadLine() before readLine() to update the index
    QByteArray data = m_dataBuffer->left(toRead);

    // Remove the read data from the buffer
    m_dataBuffer->remove(0, toRead);
    m_bytesAvailableCount -= toRead;

    return data;
    // return QByteArray(0);
}

// bool AbstractDevice::canReadLine_BLE()
// {
//     // return m_bleDevice->canReadLine();
//     return false;
// }

// QByteArray AbstractDevice::readLine_SerialPort(qint64 maxSize)
// {
//     return m_seriaPortDevice->readLine(maxSize);
// }

// QByteArray AbstractDevice::readLine_BLE(qint64 maxSize)
// {
//     // return m_bleDevice->readLine(data,maxSize);
//     return QByteArray(0);
// }

// qint64 AbstractDevice::bytesAvailable_SerialPort()
// {
//     return m_seriaPortDevice->bytesAvailable();
// }

// qint64 AbstractDevice::bytesAvailable_BLE()
// {
//     return m_bleDevice->bytesAvailable();
// }



qint64 AbstractDevice::bytesAvailable()
{
    return m_bytesAvailableCount;
}

QByteArray AbstractDevice::read(qint64 maxSize)
{
    if (maxSize <= 0 || m_bytesAvailableCount == 0)
    {
        return QByteArray();
    }

    qint64 toRead = qMin(maxSize, m_bytesAvailableCount);
    QByteArray data = m_dataBuffer->left(toRead);

    // Remove the read data from the buffer
    m_dataBuffer->remove(0, toRead);
    m_bytesAvailableCount -= toRead;

    return data;
}

qint64 AbstractDevice::read_b(char *data, qint64 maxSize)
{
    if (maxSize <= 0 || m_bytesAvailableCount == 0)
    {
        return 0;
    }

    qint64 toRead = qMin(maxSize, m_bytesAvailableCount);
    memcpy(data, m_dataBuffer->constData(), toRead);

    // Remove the read data from the internal buffer
    m_dataBuffer->remove(0, toRead);
    m_bytesAvailableCount -= toRead;

    return toRead;
}

bool AbstractDevice::getChar(char *c)
{
    // Check if there is at least one byte available in the buffer
    if (m_bytesAvailableCount > 0) {
        // Read the first byte from the buffer
        *c = m_dataBuffer->at(0);

        // Remove the first byte from the buffer
        m_dataBuffer->remove(0, 1);
        m_bytesAvailableCount--;

        return true;  // Successfully read a byte
    }

    return false;  // No data available
}
