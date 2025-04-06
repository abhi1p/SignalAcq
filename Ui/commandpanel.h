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

#ifndef COMMANDPANEL_H
#define COMMANDPANEL_H

#include <QWidget>
#include <QSerialPort>
#include <QByteArray>
#include <QList>
#include <QMenu>
#include <QAction>
#include <QSettings>

#include "commandwidget.h"
#include "commandwidgetble.h"

namespace Ui {
class CommandPanel;
}

class CommandPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CommandPanel( QWidget *parent = 0);
    ~CommandPanel();

    QMenu* menu();
    /// Action for creating a new command.
    QAction* newCommandAction();
    /// Stores commands into a `QSettings`
    void saveSettings(QSettings* settings);
    /// Loads commands from a `QSettings`.
    void loadSettings(QSettings* settings);
    /// Number of commands
    unsigned numOfCommandsSerial();
    unsigned numOfCommandsBLE();

signals:
    // emitted when user tries to send an empty command
    void focusRequested();
    void addCharacteristicsSignal(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag);
    void writeCharacteristicsSignal(QBluetoothUuid charUuid, QByteArray value);
    void sendCommandSerialSignal(QByteArray command);
    void log(QtMsgType type, QString text);

private:
    Ui::CommandPanel *ui;
    // QSerialPort* serialPort;
    QMenu _menu;
    QAction _newCommandAction;
    QList<CommandWidget*> commandsSerial;
    QList<CommandWidgetBLE*> commandsBLE;

    /// Reassigns F1-12 keys to commands in order.
    void reAssignShortcuts();

    unsigned serialCommand_name_counter;
    unsigned bleCommand_name_counter;
    bool m_bleConnected=false;
    bool m_serialPortOpened=false;
    BluetoothCharModel *m_bleCharModelMain;

private slots:
    CommandWidget* newCommandSerial();
    CommandWidgetBLE* newCommandBLE();
    void addNewCommand();
    // void sendCommandSerial(QByteArray command);
    void sendCommandBLE(QBluetoothUuid charUuid, QByteArray command);


public slots:
    void setCurrentIndex(int index);
    void bleConnected(bool connected);
    void serialPortOpened(bool opened);
    void addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag);

    void updateMenuCommands();
};

#endif // COMMANDPANEL_H
