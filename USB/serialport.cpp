#include "serialport.h"

SerialPort::SerialPort(QObject *parent)
    : QObject{parent}
{


}

void SerialPort::initSerialPort()
{
    m_serialPort.reset(new QSerialPort);
    m_pinUpdateTimer.reset(new QTimer);

    m_pinUpdateTimer->setInterval(1000); // ms
    connect(m_pinUpdateTimer.data(), &QTimer::timeout, this, &SerialPort::updatePinLeds,Qt::QueuedConnection);
    connect(m_serialPort.data(), &QSerialPort::readyRead, this, &SerialPort::readyRead,Qt::QueuedConnection);
    connect(m_serialPort.data(), &QSerialPort::errorOccurred, this, &SerialPort::onPortError,Qt::QueuedConnection);
}
void SerialPort::selectBaudRate(QString baudRate)
{
    if (m_serialPort->isOpen())
    {
        if (!m_serialPort->setBaudRate(baudRate.toInt()))
        {
            qCritical() << "Can't set baud rate!";
        }
    }
}

void SerialPort::selectParity(int parity)
{
    if (m_serialPort->isOpen())
    {
        if(!m_serialPort->setParity((QSerialPort::Parity) parity))
        {
            qCritical() << "Can't set parity option!";
        }
    }
}

void SerialPort::selectDataBits(int dataBits)
{
    if (m_serialPort->isOpen())
    {
        if(!m_serialPort->setDataBits((QSerialPort::DataBits) dataBits))
        {
            qCritical() << "Can't set numer of data bits!";
        }
    }
}

void SerialPort::selectStopBits(int stopBits)
{
    if (m_serialPort->isOpen())
    {
        if(!m_serialPort->setStopBits((QSerialPort::StopBits) stopBits))
        {
            qCritical() << "Can't set number of stop bits!";
        }
    }
}

void SerialPort::selectFlowControl(int flowControl)
{
    if (m_serialPort->isOpen())
    {
        if(!m_serialPort->setFlowControl((QSerialPort::FlowControl) flowControl))
        {
            qCritical() << "Can't set flow control option!";
        }
    }
}

void SerialPort::setDtr(bool toSet)
{
    if(m_serialPort->isOpen())
    {
        m_serialPort->setDataTerminalReady(toSet);
    }
}

void SerialPort::setRts(bool toSet)
{
    if(m_serialPort->isOpen())
    {
        m_serialPort->setRequestToSend(toSet);
    }
}

void SerialPort::sendCommandSerial(QByteArray command)
{
    if(m_serialPort->isOpen())
    {
        if(m_serialPort->write(command)<0)
        {
            emit log(QtCriticalMsg,"Send command failed");
        }
        else
        {
            emit log(QtInfoMsg,"Command sent");
        }
    }
    else
    {
        emit log(QtCriticalMsg,"Serial port not opened");
    }
}
void SerialPort::togglePort(QString portName, int baudRate, int parity, int dataBits, int stopBits, int flowControl, bool setDtr, bool setRts)
{
    qInfo()<<"Port name: "<<portName;
    qInfo()<<"Baud rate: "<<baudRate;
    if (m_serialPort->isOpen())
    {
        m_pinUpdateTimer->stop();
        m_serialPort->close();
        qDebug() << "Closed port:" << m_serialPort->portName();
        emit log(QtInfoMsg,"Closed port: " + m_serialPort->portName());
        emit portToggledSignal(false);
    }
    else
    {
        m_serialPort->setPortName(portName);

        // open port
        if (m_serialPort->open(QIODevice::ReadWrite))
        {
            // set port settings
            m_serialPort->setBaudRate(baudRate);
            m_serialPort->setParity((QSerialPort::Parity) parity);
            m_serialPort->setDataBits((QSerialPort::DataBits) dataBits);
            m_serialPort->setStopBits((QSerialPort::StopBits) stopBits);
            m_serialPort->setFlowControl((QSerialPort::FlowControl) flowControl);

            // set output signals
            m_serialPort->setDataTerminalReady(setDtr);
            m_serialPort->setRequestToSend(setRts);

            // update pin signals
            updatePinLeds();
            m_pinUpdateTimer->start();

            qDebug() << "Opened port:" << m_serialPort->portName();
            emit portToggledSignal(true);
            emit log(QtInfoMsg,"Opened port: " + m_serialPort->portName());
            maxBitRate();//update max bit rate
        }
    }

}

void SerialPort::updatePinLeds()
{
    QFlags<QSerialPort::PinoutSignal> pinOuts = m_serialPort->pinoutSignals();
    emit updatePinLedsSignal(pinOuts);
}

void SerialPort::onPortError(QSerialPort::SerialPortError error)
{
#ifdef Q_OS_UNIX
    // For suppressing "Invalid argument" errors that happens with pseudo terminals
    auto isPtsInvalidArgErr = [this] () -> bool {
        return m_serialPort->portName().contains("pts/") && m_serialPort->errorString().contains("Invalid argument");
    };
#endif

    switch(error)
    {
    case QSerialPort::NoError :
        break;
    case QSerialPort::ResourceError :
        qWarning() << "Port error: resource unavaliable; most likely device removed.";
        emit log(QtCriticalMsg,"Port error: device removed" );
        if (m_serialPort->isOpen())
        {
            qWarning() << "Closing port on resource error: " << m_serialPort->portName();
            emit log(QtWarningMsg,"Closing port on resource error: " + m_serialPort->portName());
            m_pinUpdateTimer->stop();
            m_serialPort->close();
            qDebug() << "Closed port: " << m_serialPort->portName();
            emit log(QtInfoMsg,"Closed port: " + m_serialPort->portName());
            // emit portToggledSignal(false);
        }
        emit loadPortListSignal();
        break;
    case QSerialPort::DeviceNotFoundError:
        qCritical() << "Device doesn't exist: " << m_serialPort->portName();
        emit log(QtCriticalMsg,"Device doesn't exist: " + m_serialPort->portName());
        // emit portToggledSignal(m_serialPort->isOpen());
        break;
    case QSerialPort::PermissionError:
        qCritical() << "Permission denied. Either you don't have \
                required privileges or device is already opened by another process.";
        emit log(QtCriticalMsg,"Port open permission denied");
               break;
    case QSerialPort::OpenError:
        qWarning() << "Device is already opened!";
        // emit portToggledSignal(m_serialPort->isOpen());
        emit log(QtCriticalMsg,"Port already opened");
        break;
    case QSerialPort::NotOpenError:
        qCritical() << "Device is not open!";
        break;

#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)

    case QSerialPort::ParityError:
        qCritical() << "Parity error detected.";
        break;
    case QSerialPort::FramingError:
        qCritical() << "Framing error detected.";
        break;
    case QSerialPort::BreakConditionError:
        qCritical() << "Break condition is detected.";
        break;
#endif
    case QSerialPort::WriteError:
        qCritical() << "An error occurred while writing data.";
        emit log(QtCriticalMsg,"An error occurred while writing data");
        break;
    case QSerialPort::ReadError:
        qCritical() << "An error occurred while reading data.";
        emit log(QtCriticalMsg,"An error occurred while reading data");
        break;
    case QSerialPort::UnsupportedOperationError:
#ifdef Q_OS_UNIX
        // Qt 5.5 gives "Invalid argument" with 'UnsupportedOperationError'
        if (isPtsInvalidArgErr())
            break;
#endif
        qCritical() << "Operation is not supported.";
        break;
    case QSerialPort::TimeoutError:
        qCritical() << "A timeout error occurred.";
        break;
    case QSerialPort::UnknownError:
#ifdef Q_OS_UNIX
        // Qt 5.2 gives "Invalid argument" with 'UnknownError'
        if (isPtsInvalidArgErr())
            break;
#endif
        qCritical() << "Unknown error! Error: " << m_serialPort->errorString();
        emit log(QtCriticalMsg,"Unknown error: "+m_serialPort->errorString());
        break;
    default:
        qCritical() << "Unhandled port error: " << error;
        break;
    }
    emit portToggledSignal(m_serialPort->isOpen());
}

void SerialPort::maxBitRate()
{
    float baud = m_serialPort->baudRate();
    float dataBits = m_serialPort->dataBits();
    float parityBits = m_serialPort->parity() == QSerialPort::NoParity ? 0 : 1;

    float stopBits;
    if (m_serialPort->stopBits() == QSerialPort::OneAndHalfStop)
    {
        stopBits = 1.5;
    }
    else
    {
        stopBits = m_serialPort->stopBits();
    }

    float frame_size = 1 /* start bit */ + dataBits + parityBits + stopBits;

    // qInfo()<<"Frame size: "<<frame_size;
    emit maxBitRateSignal(float(baud) / frame_size);
    // return ;
}

void SerialPort::readyRead()
{
    quint64 bytes=m_serialPort->bytesAvailable();
    //qInfo()<<"Available bytes: "<<bytes;

    // if(m_serialPort->canReadLine())
    // {
    //     qInfo()<<"Read"<<m_serialPort->readLine();
    // }
    emit serialDataReadSignal(m_serialPort->read(bytes));
}
