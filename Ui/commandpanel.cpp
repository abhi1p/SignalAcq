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

#include <QByteArray>
#include <QtDebug>

#include "commandpanel.h"
#include "ui_commandpanel.h"
#include "setting_defines.h"

CommandPanel::CommandPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandPanel),
    // _menu(trUtf8("&Commands")), _newCommandAction(trUtf8("&New Command"), this)
    _menu(tr("&Commands")), _newCommandAction(tr("&New Command"), this)
{
    // serialPort = port;

    ui->setupUi(this);
    QVBoxLayout *layout1 = new QVBoxLayout();
    layout1->setSpacing(5);
    layout1->setAlignment(Qt::AlignTop);
    ui->scrollAreaWidgetContentsSerial->setLayout(layout1);

    QVBoxLayout *layout2 = new QVBoxLayout();
    layout2->setSpacing(5);
    layout2->setAlignment(Qt::AlignTop);
    ui->scrollAreaWidgetContentsBLE->setLayout(layout2);

    connect(ui->pbNew, &QPushButton::clicked, this, &CommandPanel::addNewCommand);
    connect(&_newCommandAction, &QAction::triggered, this, &CommandPanel::addNewCommand);

    _menu.addAction(&_newCommandAction);
    _menu.addSeparator();

    serialCommand_name_counter = 0;
    bleCommand_name_counter=0;

    setCurrentIndex(0);
    qWarning()<<"Command panel init";

    m_bleCharModelMain=new BluetoothCharModel(this);
}

CommandPanel::~CommandPanel()
{
    delete m_bleCharModelMain;
    commandsSerial.clear(); // UI will 'delete' actual objects
    commandsBLE.clear();
    delete ui;
}

CommandWidget* CommandPanel::newCommandSerial()
{
    auto command = new CommandWidget();
    serialCommand_name_counter++;
    // command->setName(trUtf8("Command ") + QString::number(command_name_counter));
    command->setName(tr("Command ") + QString::number(serialCommand_name_counter));
    ui->scrollAreaWidgetContentsSerial->layout()->addWidget(command);
    command->setFocusToEdit();
    connect(command, &CommandWidget::sendCommand, this, &CommandPanel::sendCommandSerial);
    connect(command, &CommandWidget::focusRequested, this, &CommandPanel::focusRequested);
    _menu.addAction(command->sendAction());

    // add to command list and remove on destroy
    commandsSerial << command;
    connect(command, &QObject::destroyed, this,[this](QObject* obj)
            {
                commandsSerial.removeOne(static_cast<CommandWidget*>(obj));

                reAssignShortcuts();
                qInfo()<<"Serial command removed";
            });


    reAssignShortcuts();
    return command;
}

CommandWidgetBLE *CommandPanel::newCommandBLE()
{
    auto command = new CommandWidgetBLE();
    bleCommand_name_counter++;
    qInfo()<<"BLE command count: "<<bleCommand_name_counter;
    // command->setName(trUtf8("Command ") + QString::number(command_name_counter));
    command->setName(tr("Command ") + QString::number(bleCommand_name_counter));
    ui->scrollAreaWidgetContentsBLE->layout()->addWidget(command);
    command->setFocusToEdit();
    connect(this, &CommandPanel::addCharacteristicsSignal, command, &CommandWidgetBLE::addCharacteristics,Qt::QueuedConnection);
    connect(command, &CommandWidgetBLE::sendCommandBLE, this, &CommandPanel::sendCommandBLE);
    connect(command, &CommandWidgetBLE::focusRequested, this, &CommandPanel::focusRequested);
    _menu.addAction(command->sendAction());

    // add to command list and remove on destroy
    commandsBLE << command;
    connect(command, &QObject::destroyed, this,[this](QObject* obj)
            {
                commandsBLE.removeOne(static_cast<CommandWidgetBLE*>(obj));
                reAssignShortcuts();
                qInfo()<<"BLE command removed";
            });

    reAssignShortcuts();
    return command;
}

void CommandPanel::addNewCommand()
{
    QString objName=ui->commandStackedWidget->currentWidget()->objectName();
    qInfo()<<"Object name: "<<objName;
    if(objName=="portPage")
    {
        newCommandSerial();
    }
    else if(objName=="blePage")
    {
        CommandWidgetBLE *command=newCommandBLE();
        command->addCharacteristics(QBluetoothUuid(),QBluetoothUuid(),QLowEnergyCharacteristic::WriteNoResponse);
        qInfo()<<"Model size: "<<m_bleCharModelMain->rowCount();
        for(int i=0;i<m_bleCharModelMain->rowCount();i++)
        {
            QModelIndex mIndex=m_bleCharModelMain->index(i,0);
            QVariant data=m_bleCharModelMain->data(mIndex,Qt::UserRole);
            BluetoothCharModel::CharacteristicDetails details = data.value<BluetoothCharModel::CharacteristicDetails>();
            command->addCharacteristics(details.serviceUuid,details.characteristicUuid,details.properties);
        }
        // if()

    }
}

void CommandPanel::reAssignShortcuts()
{
#if 0
    // can assign shortcuts to first 12 commands
    // for (int i = 0; i < std::min(12, commands.size()); i++)
    for (int i = 0; i < qMin(12, commandsSerial.size()); i++)
    {
        auto cmd = commandsSerial[i];
        cmd->sendAction()->setShortcut(QKeySequence(Qt::Key_F1 + i));
    }
#endif
}

void CommandPanel::sendCommandSerial(QByteArray command)
{


    // QString objName=ui->commandStackedWidget->currentWidget()->objectName();
    // // qInfo()<<"Object name: "<<objName;
    // if(objName=="portPage")
    // {
    //     if (!m_serialPortOpened)
    //     {
    //         qCritical() << "Port is not open!";
    //         return;
    //     }
    //     else
    //     {

    //     }

    //     // if (serialPort->write(command) < 0)
    //     // {
    //     //     qCritical() << "Send command failed!";
    //     // }
    // }
    // else if(objName=="blePage")
    // {
    //     if(m_bleConnected)
    //     {

    //     }
    //     else
    //     {
    //         qCritical() << "BLE device is not connected!";
    //     }
    // }

}

void CommandPanel::sendCommandBLE(QBluetoothUuid charUuid, QByteArray command)
{
    qInfo()<<"Char Uuid: "<<charUuid<<" Command: "<<QString::fromUtf8(command);
    if(!charUuid.isNull())
    {
        if(m_bleConnected)
        {
            emit writeCharacteristicsSignal(charUuid,command);
        }
        else
        {
            qInfo()<<"BLE device not connected";
        }

    }
    else
    {
        qInfo()<<"Invalid Uuid";
    }

}

void CommandPanel::addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag)
{
    if(flag & QLowEnergyCharacteristic::WriteNoResponse)
    {
        m_bleCharModelMain->addCharacteristic(srvcUuid,charUuid,flag);
    }
}

void CommandPanel::setCurrentIndex(int index)
{
    ui->commandStackedWidget->setCurrentIndex(index);
}

void CommandPanel::bleConnected(bool connected)
{
    m_bleConnected=connected;
    if(!connected)
    {
        m_bleCharModelMain->clearAll();
    }
}

void CommandPanel::serialPortOpened(bool opened)
{
    m_serialPortOpened=opened;
}

QMenu* CommandPanel::menu()
{
    return &_menu;
}

QAction* CommandPanel::newCommandAction()
{
    return &_newCommandAction;
}

unsigned CommandPanel::numOfCommandsSerial()
{
    return commandsSerial.size();
}

unsigned int CommandPanel::numOfCommandsBLE()
{
    return commandsBLE.size();
}

void CommandPanel::saveSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_Commands);
    settings->beginWriteArray(SG_Commands_CommandSerial);
    for (int i = 0; i < commandsSerial.size(); i ++)
    {
        settings->setArrayIndex(i);
        auto command = commandsSerial[i];
        settings->setValue(SG_Commands_Name, command->name());
        settings->setValue(SG_Commands_Type, command->isASCIIMode() ? "ascii" : "hex");
        settings->setValue(SG_Commands_Data, command->commandText());
    }
    settings->endArray();

    settings->beginWriteArray(SG_Commands_CommandBLE);
    for (int i = 0; i < commandsBLE.size(); i ++)
    {
        settings->setArrayIndex(i);
        auto command = commandsBLE[i];
        settings->setValue(SG_Commands_Name, command->name());
        settings->setValue(SG_Commands_Type, command->isASCIIMode() ? "ascii" : "hex");
        settings->setValue(SG_Commands_Data, command->commandText());
        qInfo()<<"BLE command to be saved";
        QVariant data=command->currentCharacteristics();
        if (data.isValid()) {
            // Extract the CharacteristicDetails struct from QVariant
            BluetoothCharModel::CharacteristicDetails details = data.value<BluetoothCharModel::CharacteristicDetails>();

            // Access details here
            qInfo() << "Service UUID:" << details.serviceUuid.toString();
            qInfo() << "Characteristic UUID:" << details.characteristicUuid;
            settings->setValue(SG_Commands_ServcUuid, details.serviceUuid);
            settings->setValue(SG_Commands_CharUuid, details.characteristicUuid);
            settings->setValue(SG_Commands_CharFlags, details.properties.toInt());
        }
        else
        {
            //invalid data in qvariant
            settings->setValue(SG_Commands_ServcUuid, QBluetoothUuid());
            settings->setValue(SG_Commands_CharUuid, QBluetoothUuid());
            settings->setValue(SG_Commands_CharFlags, 0);
        }


    }
    settings->endArray();
    settings->endGroup();
}

void CommandPanel::loadSettings(QSettings* settings)
{
    qWarning()<<"Load settings init";
    // clear all serial commands
    while (commandsSerial.size())
    {
        auto command = commandsSerial.takeLast();
        command->disconnect();
        delete command;
    }
    // clear all ble commands
    while (commandsBLE.size())
    {
        auto command = commandsBLE.takeLast();
        command->disconnect();
        delete command;
    }


    settings->beginGroup(SettingGroup_Commands);

    // load serial commands
    unsigned size = settings->beginReadArray(SG_Commands_CommandSerial);

    for (unsigned i = 0; i < size; i ++)
    {
        settings->setArrayIndex(i);
        auto command = newCommandSerial();

        // load command name
        QString name = settings->value(SG_Commands_Name, "").toString();
        if (!name.isEmpty()) command->setName(name);

        // Important: type should be set before command data for correct validation
        QString type = settings->value(SG_Commands_Type, "").toString();
        if (type == "ascii")
        {
            command->setASCIIMode(true);
        }
        else if (type == "hex")
        {
            command->setASCIIMode(false);
        } // else unchanged

        // load command data
        command->setCommandText(settings->value(SG_Commands_Data, "").toString());
    }

    settings->endArray();

    // load ble commands
    size = settings->beginReadArray(SG_Commands_CommandBLE);

    for (unsigned i = 0; i < size; i ++)
    {
        settings->setArrayIndex(i);
        auto command = newCommandBLE();

        // load command name
        QString name = settings->value(SG_Commands_Name, "").toString();
        if (!name.isEmpty()) command->setName(name);

        // Important: type should be set before command data for correct validation
        QString type = settings->value(SG_Commands_Type, "").toString();
        if (type == "ascii")
        {
            command->setASCIIMode(true);
        }
        else if (type == "hex")
        {
            command->setASCIIMode(false);
        } // else unchanged

        // load command data
        command->setCommandText(settings->value(SG_Commands_Data, "").toString());

        //load current Uuid
        // command->setCurrentCharUuid(settings->value(SG_Commands_Uuid,"").toString());
        qInfo()<<"Current UUid: "<<settings->value(SG_Commands_CharUuid,"").toString();
        QBluetoothUuid srvcUuid=settings->value(SG_Commands_ServcUuid,QBluetoothUuid()).toUuid();
        QBluetoothUuid charUuid=settings->value(SG_Commands_CharUuid,QBluetoothUuid()).toUuid();

        int propInt=settings->value(SG_Commands_CharFlags,QLowEnergyCharacteristic::WriteNoResponse).toInt();
        QLowEnergyCharacteristic::PropertyTypes prop=QFlags<QLowEnergyCharacteristic::PropertyType>(propInt);
        command->addCharacteristics(srvcUuid,charUuid,prop);

    }

    settings->endArray();
    settings->endGroup();
}
