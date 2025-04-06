#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QTimer>

#include <QSharedPointer>

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = nullptr);

signals:
    void portToggledSignal(bool opened);
    void updatePinLedsSignal(QFlags<QSerialPort::PinoutSignal> pinOut);
    void loadPortListSignal();
    void maxBitRateSignal(quint64 maxBitRate);
    void serialDataReadSignal(const QByteArray &newValue);
    void log(QtMsgType type, QString text);


public slots:
    void initSerialPort();

    void togglePort(QString portName, int baudRate, int parity,
                    int dataBits, int stopBits, int flowControl, bool setDtr, bool setRts);
    void selectBaudRate(QString baudRate);
    void selectParity(int parity); // parity must be one of QSerialPort::Parity
    void selectDataBits(int dataBits); // bits must be one of QSerialPort::DataBits
    void selectStopBits(int stopBits); // stopBits must be one of QSerialPort::StopBits
    void selectFlowControl(int flowControl); // flowControl must be one of QSerialPort::FlowControl
    void setDtr(bool toSet);
    void setRts(bool toSet);
    void sendCommandSerial(QByteArray command);

private slots:
    void updatePinLeds();
    void onPortError(QSerialPort::SerialPortError error);
    void maxBitRate();
    void readyRead();


private:
    QSharedPointer<QSerialPort> m_serialPort;
    QSharedPointer<QTimer> m_pinUpdateTimer;

};

#endif // SERIALPORT_H
