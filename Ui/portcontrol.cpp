/*
  Copyright © 2022 Hasan Yavuz Özderya

  This file is part of serialplot.

  serialplot is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  serialplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with serialplot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "portcontrol.h"
#include "ui_portcontrol.h"

#include <QSerialPortInfo>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QtDebug>

#include "setting_defines.h"
#include "utils.h"

#define TBPORTLIST_MINWIDTH (200)

// setting mappings
const QMap<QSerialPort::Parity, QString> paritySettingMap({
        {QSerialPort::NoParity, "none"},
        {QSerialPort::OddParity, "odd"},
        {QSerialPort::EvenParity, "even"},
    });

PortControl::PortControl(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PortControl),
    // portToolBar("Port Toolbar"),
    openAction("Open", this),
    loadPortListAction("↺", this)
{
    ui->setupUi(this);

    // serialPort = port;
    // connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
    //         this, SLOT(onPortError(QSerialPort::SerialPortError)));
    //connect(&m_serialPort, &QSerialPort::errorOccurred,this, &PortControl::onPortError,Qt::QueuedConnection);

    // setup actions
    connect(this,&PortControl::portToggledSignal,this,&PortControl::portToggled,Qt::QueuedConnection);
    openAction.setCheckable(true);
    openAction.setShortcut(QKeySequence("Ctrl+O"));
    openAction.setToolTip("Open Port");
    connect(&openAction, &QAction::triggered,
                     this, &PortControl::openActionTriggered);

    loadPortListAction.setToolTip("Reload port list");
    // connect(&loadPortListAction, &QAction::triggered,
    //                  this,[this](bool checked){loadPortList();});

    connect(&loadPortListAction,&QAction::triggered,this,&PortControl::loadPortList,Qt::QueuedConnection);

    // // setup toolbar
    // portToolBar.addWidget(&tbPortList);
    // portToolBar.addAction(&loadPortListAction);
    // portToolBar.addAction(&openAction);

    // // setup port selection widgets
    // tbPortList.setMinimumWidth(TBPORTLIST_MINWIDTH);
    // tbPortList.setModel(&portList);
    ui->cbPortList->setModel(&portList);
    /// connect(ui->cbPortList,&QComboBox::currentTextChanged,this,&PortControl::selectListedPort,Qt::QueuedConnection);

    // setup buttons
    ui->pbOpenPort->setDefaultAction(&openAction);
    ui->pbReloadPorts->setDefaultAction(&loadPortListAction);

    // setup baud rate selection widget
    // QObject::connect(ui->cbBaudRate,
    //                  SELECT<const QString&>::OVERLOAD_OF(&QComboBox::activated),
    //                  this, &PortControl::_selectBaudRate);
    connect(ui->cbBaudRate,&QComboBox::currentTextChanged,this,&PortControl::selectBaudRateSignal,Qt::QueuedConnection);

    // QObject::connect(ui->cbBaudRate,
    //                  SELECT<const QString&>::OVERLOAD_OF(&QComboBox::textActivated),
    //                  this, &PortControl::_selectBaudRate);

    // setup parity selection buttons
    parityButtons.addButton(ui->rbNoParity, (int) QSerialPort::NoParity);
    parityButtons.addButton(ui->rbEvenParity, (int) QSerialPort::EvenParity);
    parityButtons.addButton(ui->rbOddParity, (int) QSerialPort::OddParity);

    // QObject::connect(&parityButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
    //                  this, &PortControl::selectParity);
    // QObject::connect(&parityButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::idClicked),
    //                  this, &PortControl::selectParity);

    connect(&parityButtons,&QButtonGroup::idClicked,this,&PortControl::selectParitySignal,Qt::QueuedConnection);


    // setup data bits selection buttons
    dataBitsButtons.addButton(ui->rb8Bits, (int) QSerialPort::Data8);
    dataBitsButtons.addButton(ui->rb7Bits, (int) QSerialPort::Data7);
    dataBitsButtons.addButton(ui->rb6Bits, (int) QSerialPort::Data6);
    dataBitsButtons.addButton(ui->rb5Bits, (int) QSerialPort::Data5);

    // QObject::connect(&dataBitsButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
    //                  this, &PortControl::selectDataBits);
    // QObject::connect(&dataBitsButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::idClicked),
    //                  this, &PortControl::selectDataBits);
    connect(&dataBitsButtons,&QButtonGroup::idClicked,this,&PortControl::selectDataBitsSignal,Qt::QueuedConnection);

    // setup stop bits selection buttons
    stopBitsButtons.addButton(ui->rb1StopBit, (int) QSerialPort::OneStop);
    stopBitsButtons.addButton(ui->rb2StopBit, (int) QSerialPort::TwoStop);

    // QObject::connect(&stopBitsButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
    //                  this, &PortControl::selectStopBits);
    // QObject::connect(&stopBitsButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::idClicked),
    //                  this, &PortControl::selectStopBits);

    connect(&stopBitsButtons,&QButtonGroup::idClicked,this,&PortControl::selectStopBitsSignal,Qt::QueuedConnection);

    // setup flow control selection buttons
    flowControlButtons.addButton(ui->rbNoFlowControl,
                                 (int) QSerialPort::NoFlowControl);
    flowControlButtons.addButton(ui->rbHardwareControl,
                                 (int) QSerialPort::HardwareControl);
    flowControlButtons.addButton(ui->rbSoftwareControl,
                                 (int) QSerialPort::SoftwareControl);

    // QObject::connect(&flowControlButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
    //                  this, &PortControl::selectFlowControl);
    // QObject::connect(&flowControlButtons,
    //                  SELECT<int>::OVERLOAD_OF(&QButtonGroup::idClicked),
    //                  this, &PortControl::selectFlowControl);
    connect(&flowControlButtons,&QButtonGroup::idClicked,this,&PortControl::selectFlowControlSignal,Qt::QueuedConnection);

    // initialize signal leds
    ui->ledDTR->setOn(true);
    ui->ledRTS->setOn(true);

    // connect output signals
    connect(ui->pbDTR, &QPushButton::clicked, this,[this]()
            {
                // toggle DTR
                ui->ledDTR->toggle();
                emit setDtrSignal(ui->ledDTR->isOn());
            });

    connect(ui->pbRTS, &QPushButton::clicked, this,[this]()
            {
                // toggle RTS
                ui->ledRTS->toggle();
                emit setRtsSignal(ui->ledRTS->isOn());
            });

    // setup pin update leds
    ui->ledDCD->setColor(Qt::yellow);
    ui->ledDSR->setColor(Qt::yellow);
    ui->ledRI->setColor(Qt::yellow);
    ui->ledCTS->setColor(Qt::yellow);

    // pinUpdateTimer.setInterval(1000); // ms
    // connect(&pinUpdateTimer, &QTimer::timeout, this, &PortControl::updatePinLeds);

    loadPortList();
    loadBaudRateList();
    ui->cbBaudRate->setCurrentIndex(ui->cbBaudRate->findText("9600"));
}

PortControl::~PortControl()
{
    delete ui;
}

void PortControl::loadPortList()
{
    QString currentSelection = ui->cbPortList->currentData(PortNameRole).toString();
    qInfo()<<"Current selection: "<<currentSelection;
    portList.loadPortList();
    // portList.
    // for(auto portItem : portList)
    // {

    // }
    int index = portList.indexOf(currentSelection);
    if (index >= 0)
    {
        ui->cbPortList->setCurrentIndex(index);
        // tbPortList.setCurrentIndex(index);
    }

    if (portList.rowCount() == 0)
    {
        ui->cbPortList->lineEdit()->setPlaceholderText(tr("No port found - enter name"));
    }
    else
    {
        ui->cbPortList->lineEdit()->setPlaceholderText(tr("Select port or enter name"));
    }
}

void PortControl::loadBaudRateList()
{
    ui->cbBaudRate->clear();

    for (auto &baudRate : QSerialPortInfo::standardBaudRates())
    {
        ui->cbBaudRate->addItem(QString::number(baudRate));
    }
}



void PortControl::togglePort()
{
    QString portName;
    QString portText = ui->cbPortList->currentText().trimmed();

    if (portText.isEmpty())
    {
        qWarning() << "Select or enter a port name!";
        return;
    }

    // we get the port name from the edit text, which may not be
    // in the portList if user hasn't pressed Enter
    // Also note that, portText may be different than `portName`
    int portIndex = portList.indexOf(portText);
    if (portIndex < 0) // not in list, add to model and update the selections
    {
        portList.appendRow(new PortListItem(portText));
        ui->cbPortList->setCurrentIndex(portList.rowCount()-1);
        // tbPortList.setCurrentIndex(portList.rowCount()-1);
        portName = portText;
    }
    else
    {
        // get the port name from the data field
        portName = static_cast<PortListItem*>(portList.item(portIndex))->portName();
    }

    QString port=ui->cbPortList->currentData(PortNameRole).toString();
    int baudRate=ui->cbBaudRate->currentText().toInt();
    int parity= (QSerialPort::Parity) parityButtons.checkedId();
    int dataBits=(QSerialPort::DataBits) dataBitsButtons.checkedId();
    int stopBits=(QSerialPort::StopBits) stopBitsButtons.checkedId();
    int flowControl=(QSerialPort::FlowControl) flowControlButtons.checkedId();
    bool setDtr=ui->ledDTR->isOn();
    bool setRts=ui->ledRTS->isOn();

    emit togglePortSignal(port,baudRate, parity, dataBits, stopBits, flowControl, setDtr, setRts);

}

// void PortControl::togglePort()
// {
//     if (serialPort->isOpen())
//     {
//         pinUpdateTimer.stop();
//         serialPort->close();
//         qDebug() << "Closed port:" << serialPort->portName();
//         emit portToggled(false);
//     }
//     else
//     {
//         QString portName;
//         QString portText = ui->cbPortList->currentText().trimmed();

//         if (portText.isEmpty())
//         {
//             qWarning() << "Select or enter a port name!";
//             return;
//         }

//         // we get the port name from the edit text, which may not be
//         // in the portList if user hasn't pressed Enter
//         // Also note that, portText may be different than `portName`
//         int portIndex = portList.indexOf(portText);
//         if (portIndex < 0) // not in list, add to model and update the selections
//         {
//             portList.appendRow(new PortListItem(portText));
//             ui->cbPortList->setCurrentIndex(portList.rowCount()-1);
//             tbPortList.setCurrentIndex(portList.rowCount()-1);
//             portName = portText;
//         }
//         else
//         {
//             // get the port name from the data field
//             portName = static_cast<PortListItem*>(portList.item(portIndex))->portName();
//         }

//         serialPort->setPortName(ui->cbPortList->currentData(PortNameRole).toString());

//         // open port
//         if (serialPort->open(QIODevice::ReadWrite))
//         {
//             // set port settings
//             _selectBaudRate(ui->cbBaudRate->currentText());
//             selectParity((QSerialPort::Parity) parityButtons.checkedId());
//             selectDataBits((QSerialPort::DataBits) dataBitsButtons.checkedId());
//             selectStopBits((QSerialPort::StopBits) stopBitsButtons.checkedId());
//             selectFlowControl((QSerialPort::FlowControl) flowControlButtons.checkedId());

//             // set output signals
//             serialPort->setDataTerminalReady(ui->ledDTR->isOn());
//             serialPort->setRequestToSend(ui->ledRTS->isOn());

//             // update pin signals
//             updatePinLeds();
//             pinUpdateTimer.start();

//             qDebug() << "Opened port:" << serialPort->portName();
//             emit portToggled(true);
//         }
//     }
//     openAction.setChecked(serialPort->isOpen());
// }

// void PortControl::selectListedPort(QString portName)
// {
//     // portName may be coming from combobox
//     portName = portName.split(" ")[0];

//     QSerialPortInfo portInfo(portName);
//     if (portInfo.isNull())
//     {
//         qWarning() << "Device doesn't exist:" << portName;
//     }

//     // has selection actually changed
//     if (portName != serialPort->portName())
//     {
//         // if another port is already open, close it by toggling
//         if (serialPort->isOpen())
//         {
//             togglePort();

//             // open new selection by toggling
//             togglePort();
//         }
//     }
// }

QString PortControl::selectedPortName()
{
    QString portText = ui->cbPortList->currentText();
    int portIndex = portList.indexOf(portText);
    if (portIndex < 0) // not in the list yet
    {
        // return the displayed name as port name
        return portText;
    }
    else
    {
        // get the port name from the 'port list'
        return static_cast<PortListItem*>(portList.item(portIndex))->portName();
    }
}

// QToolBar* PortControl::toolBar()
// {
//     return &portToolBar;
// }

void PortControl::openActionTriggered(bool checked)
{
    togglePort();
}

// void PortControl::onCbPortListActivated(int index)
// {
//     // tbPortList.setCurrentIndex(index);
// }

// void PortControl::onTbPortListActivated(int index)
// {
//     ui->cbPortList->setCurrentIndex(index);
// }



void PortControl::updatePinLeds(QFlags<QSerialPort::PinoutSignal> pinOut)
{
    ui->ledDCD->setOn(pinOut & QSerialPort::DataCarrierDetectSignal);
    ui->ledDSR->setOn(pinOut & QSerialPort::DataSetReadySignal);
    ui->ledRI->setOn(pinOut & QSerialPort::RingIndicatorSignal);
    ui->ledCTS->setOn(pinOut & QSerialPort::ClearToSendSignal);
}

void PortControl::setmaxBitRate(quint64 maxBitRate)
{
    m_maxBitRate=maxBitRate;
}

void PortControl::portToggled(bool opened)
{
    openAction.setChecked(opened);
    if(opened)
    {
        ui->pbReloadPorts->setEnabled(false);
        ui->cbPortList->setEnabled(false);
    }
    else
    {
        ui->pbReloadPorts->setEnabled(true);
        ui->cbPortList->setEnabled(true);
    }
}

QString PortControl::currentParityText()
{
    return paritySettingMap.value(
        (QSerialPort::Parity) parityButtons.checkedId());
}

QString PortControl::currentFlowControlText()
{
    if (flowControlButtons.checkedId() == QSerialPort::HardwareControl)
    {
        return "hardware";
    }
    else if (flowControlButtons.checkedId() == QSerialPort::SoftwareControl)
    {
        return "software";
    }
    else // no parity
    {
        return "none";
    }
}

void PortControl::selectPort(QString portName)
{
    int portIndex = portList.indexOfName(portName);
    if (portIndex < 0) // not in list, add to model and update the selections
    {
        portList.appendRow(new PortListItem(portName));
        portIndex = portList.rowCount()-1;
    }

    ui->cbPortList->setCurrentIndex(portIndex);
    // tbPortList.setCurrentIndex(portIndex);

    // selectListedPort(portName);
}

void PortControl::selectBaudrate(QString baudRate)
{
    int baudRateIndex = ui->cbBaudRate->findText(baudRate);
    if (baudRateIndex < 0)
    {
        ui->cbBaudRate->setCurrentText(baudRate);
    }
    else
    {
        ui->cbBaudRate->setCurrentIndex(baudRateIndex);
    }
    emit selectBaudRateSignal(baudRate);
    // _selectBaudRate(baudRate);
}

void PortControl::openPort()
{
    // if (!serialPort->isOpen())
    // {
    //     // openAction.trigger();
    // }
    openAction.trigger();
}

unsigned PortControl::maxBitRate() const
{
    // float baud = serialPort->baudRate();
    // float dataBits = serialPort->dataBits();
    // float parityBits = serialPort->parity() == QSerialPort::NoParity ? 0 : 1;

    // float stopBits;
    // if (serialPort->stopBits() == QSerialPort::OneAndHalfStop)
    // {
    //     stopBits = 1.5;
    // }
    // else
    // {
    //     stopBits = serialPort->stopBits();
    // }

    // float frame_size = 1 /* start bit */ + dataBits + parityBits + stopBits;

    // // qInfo()<<"Frame size: "<<frame_size;
    // return float(baud) / frame_size;
    return m_maxBitRate;
}

void PortControl::saveSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_Port);
    settings->setValue(SG_Port_SelectedPort, selectedPortName());
    settings->setValue(SG_Port_BaudRate, ui->cbBaudRate->currentText());
    settings->setValue(SG_Port_Parity, currentParityText());
    settings->setValue(SG_Port_DataBits, dataBitsButtons.checkedId());
    settings->setValue(SG_Port_StopBits, stopBitsButtons.checkedId());
    settings->setValue(SG_Port_FlowControl, currentFlowControlText());
    settings->endGroup();
}

void PortControl::loadSettings(QSettings* settings)
{
    // make sure the port is closed
    // if (serialPort->isOpen()) togglePort();

    settings->beginGroup(SettingGroup_Port);

    // set port name if it exists in the current list otherwise ignore
    QString portName = settings->value(SG_Port_SelectedPort, QString()).toString();
    if (!portName.isEmpty())
    {
        int index = portList.indexOfName(portName);
        if (index > -1) ui->cbPortList->setCurrentIndex(index);
    }

    // load baud rate setting
    QString baudSetting = settings->value(
        SG_Port_BaudRate, ui->cbBaudRate->currentText()).toString();
    int baudIndex = ui->cbBaudRate->findText(baudSetting);
    if (baudIndex > -1)
    {
        ui->cbBaudRate->setCurrentIndex(baudIndex);
    }
    else
    {
        // validate
        bool ok;
        int r = baudSetting.toUInt(&ok);
        if (ok && r > 0)
        {
            ui->cbBaudRate->insertItem(0, baudSetting);
            ui->cbBaudRate->setCurrentIndex(0);
        }
        else
        {
            qCritical() << "Invalid baud setting: " << baudSetting;
        }
    }

    // load parity setting
    QString parityText =
        settings->value(SG_Port_Parity, currentParityText()).toString();
    QSerialPort::Parity paritySetting = paritySettingMap.key(
        parityText, (QSerialPort::Parity) parityButtons.checkedId());
    parityButtons.button(paritySetting)->setChecked(true);

    // load number of bits
    int dataBits = settings->value(SG_Port_DataBits, dataBitsButtons.checkedId()).toInt();
    if (dataBits >=5 && dataBits <= 8)
    {
        dataBitsButtons.button((QSerialPort::DataBits) dataBits)->setChecked(true);
    }

    // load stop bits
    int stopBits = settings->value(SG_Port_StopBits, stopBitsButtons.checkedId()).toInt();
    if (stopBits == QSerialPort::OneStop)
    {
        ui->rb1StopBit->setChecked(true);
    }
    else if (stopBits == QSerialPort::TwoStop)
    {
        ui->rb2StopBit->setChecked(true);
    }

    // load flow control
    QString flowControlSetting =
        settings->value(SG_Port_FlowControl, currentFlowControlText()).toString();
    if (flowControlSetting == "hardware")
    {
        ui->rbHardwareControl->setChecked(true);
    }
    else if (flowControlSetting == "software")
    {
        ui->rbSoftwareControl->setChecked(true);
    }
    else
    {
        ui->rbNoFlowControl->setChecked(true);
    }

    settings->endGroup();
}
