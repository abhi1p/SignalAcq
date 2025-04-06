#include "blechar.h"
#include <qendian.h>
#include "ui_blechar.h"

bleChar::bleChar(QWidget *parent, QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag)
    : QFrame(parent)
    , ui(new Ui::bleChar)
{
    ui->setupUi(this);

    m_srvcUuid=srvcUuid;
    m_charUuid=charUuid;
    m_flag=flag;

    ui->srvcUUIDValueLabel->setText(srvcUuid.toString());
    ui->charUUIDValueLabel->setText(charUuid.toString());

    if(flag & QLowEnergyCharacteristic::Notify)
    {
        ui->notifyEnableCheckBox->setEnabled(true);
        connect(ui->notifyEnableCheckBox,&QCheckBox::stateChanged,this,&bleChar::enableNotification,Qt::QueuedConnection);
    }
    else
    {
        ui->notifyEnableCheckBox->setEnabled(false);
    }

    if(flag &  QLowEnergyCharacteristic::WriteNoResponse)
    {
        ui->charWriteBtn->setEnabled(true);
        ui->bleCharValueLineEdit->setEnabled(true);
        // ui->bleCharValueLineEdit->setReadOnly(true);
        connect(ui->charWriteBtn,&QPushButton::clicked,this,&bleChar::writeCharacteristics,Qt::QueuedConnection);
    }
    else
    {
        // ui->charWriteBtn->setEnabled(false);
        ui->bleCharValueLineEdit->setEnabled(false);
        ui->bleCharValueLineEdit->setReadOnly(false);
    }

    if(flag & QLowEnergyCharacteristic::Read)
    {
        ui->charReadBtn->setEnabled(true);
        connect(ui->charReadBtn,&QPushButton::clicked,this,&bleChar::readCharacteristics,Qt::QueuedConnection);
    }
    else
    {
        ui->charReadBtn->setEnabled(false);
    }

}

bleChar::~bleChar()
{
    //disconnect(this, nullptr, nullptr, nullptr);
    delete ui;
}

void bleChar::readCharacteristics()
{
    qInfo()<<"Read characteristics";
    // emit readCharacteristicsSignal(m_srvcUuid,m_charUuid);
    emit readCharacteristicsSignal(m_charUuid);
}

void bleChar::writeCharacteristics()
{
    qInfo()<<"Write characteristics";
    emit writeCharacteristicsSignal(m_charUuid,ui->bleCharValueLineEdit->text().toUtf8());

}

void bleChar::enableNotification(int state)
{
    if(state==Qt::Checked)
    {
        //enable notification
        qInfo()<<"Notification enable";
        emit enableNotificationSignal(true, m_charUuid);
    }
    else if(state==Qt::Unchecked)
    {
        //disable notification
        qInfo()<<"Notification disable";
        emit enableNotificationSignal(false, m_charUuid);
    }
}

void bleChar::updateValue(const QByteArray &value)
{
    //qInfo()<<"Value read:"<<value<<"Hex value:"<<value.toHex()<<"Size"<<value.size();
    // unsigned size1=value.size();
    QString val=QString(value);
    // unsigned size2=val.length();

    // if(size1==size2)//not a string
    // {

    // }
    // if(val.isNull())
    // {
    //     qInfo()<<"String null";
    // }
    // else
    // {
    //     qInfo()<<"String not null";
    // }
    // if(val.isEmpty())
    // {
    //     qInfo()<<"String is empty";
    // }
    // else
    // {
    //     qInfo()<<"String is not empty";
    //     qInfo()<<val.length();
    //     qInfo()<<"If null character present"<<val.contains(QChar('\0'));
    // }
    // if(!val.contains(QChar('\0')))// not a string string
    // {
    //     val="0x"+QString(value.toHex()); //display as a hex
    // }
    val="0x"+QString(value.toHex()); //display as a hex
    //qInfo()<<"Value to set:"<<val;

    // ui->bleCharValueLineEdit->setText(QString(value));
    auto data = reinterpret_cast<const quint8 *>(value.constData());
    if(m_charUuid == QBluetoothUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement))
    {

        quint8 flags = *data;

        //Heart Rate
        int hrvalue = 0;
        if (flags & 0x1) // HR 16 bit? otherwise 8 bit
            hrvalue = static_cast<int>(qFromLittleEndian<quint16>(data[1]));
        else
            hrvalue = static_cast<int>(data[1]);
        val="Heart rate: " + QString::number(hrvalue);
    }
    else if(m_charUuid==QBluetoothUuid(QBluetoothUuid::CharacteristicType::BatteryLevel))
    {
        //auto data = reinterpret_cast<const quint8 *>(value.constData());
        int batteryLevel= static_cast<int>(qFromLittleEndian<quint8>(data[0]));
        // qInfo()<<"Battery Int value: "<<value.toInt();

        val= "SOC: "+ QString::number(batteryLevel) + "%";
#ifdef QT_DEBUG
        qInfo()<<"Data0: "<<data[0];
#endif
    }
    else if(m_charUuid==QBluetoothUuid(m_batteryLevelStatusUuid))
    {
        int batteryLevelStatus= static_cast<int>(qFromLittleEndian<quint16>(data));
        if(batteryLevelStatus==0x4100)
        {
            val="Discharging";
        }
        else if(batteryLevelStatus==0x2300)
        {
            val="Charging";
        }
        else
        {
            //nothing for now
        }

    }
    else if(m_charUuid==QBluetoothUuid(m_batteryVoltageUuid))
    {
        int batteryVoltage= static_cast<int>(qFromLittleEndian<quint16>(data));
        float batteryVoltageF=batteryVoltage/1000.0;
        val= "BV: "+ QString::number(batteryVoltageF) + "V";
#ifdef QT_DEBUG
        qInfo()<<"Data0: "<<data[0];
        qInfo()<<"Data1: "<<data[1];
#endif
    }
    else if(m_charUuid==QBluetoothUuid(m_batteryCurrentUuid))
    {
        int batteryCurrent= static_cast<int>(qFromLittleEndian<qint16>(data));
        if((batteryCurrent>100) || (batteryCurrent<-100))
        {
            float batteryCurrentF=batteryCurrent/1000.0;
            val= "Battery Current: "+ QString::number(batteryCurrentF) + "A";
        }
        else
        {
            val= "BC: "+ QString::number(batteryCurrent) + "mA";
        }
#ifdef QT_DEBUG
        qInfo()<<"Data0: "<<data[0];
        qInfo()<<"Data1: "<<data[1];
#endif

    }

    ui->bleCharValueLineEdit->setText(val);

}

bool bleChar::isValid(QBluetoothUuid charUuid)
{
    // if( (srvcUuid==m_srvcUuid) && (charUuid==m_charUuid) )
    if( charUuid==m_charUuid )
    {
        return true;
    }
    else
    {
        return false;
    }
}
