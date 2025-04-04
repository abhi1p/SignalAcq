/*
  Copyright © 2019 Hasan Yavuz Özderya

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

#ifndef PORTCONTROL_H
#define PORTCONTROL_H

#include <QWidget>
#include <QButtonGroup>
#include <QSerialPort>
#include <QStringList>
#include <QToolBar>
#include <QAction>
#include <QComboBox>
#include <QSettings>
#include <QTimer>

#include "Common/portlist.h"

namespace Ui {
class PortControl;
}

class PortControl : public QWidget
{
    Q_OBJECT

public:
    explicit PortControl(QWidget* parent = 0);
    ~PortControl();

    // QSerialPort* serialPort;
    // QToolBar* toolBar();

    void selectPort(QString portName);
    void selectBaudrate(QString baudRate);
    void openPort();
    /// Returns maximum bit rate for current baud rate
    unsigned maxBitRate() const;

    /// Stores port settings into a `QSettings`
    void saveSettings(QSettings* settings);
    /// Loads port settings from a `QSettings`. If open serial port is closed.
    void loadSettings(QSettings* settings);

private:
    Ui::PortControl *ui;

    QButtonGroup parityButtons;
    QButtonGroup dataBitsButtons;
    QButtonGroup stopBitsButtons;
    QButtonGroup flowControlButtons;

    // QToolBar portToolBar;
    QAction openAction;
    QAction loadPortListAction;
    // QComboBox tbPortList;
    PortList portList;
    quint64 m_maxBitRate=0;

    /// Used to refresh pinout signal leds periodically
    // QTimer pinUpdateTimer;

    /// Returns the currently selected (entered) "portName" in the UI
    QString selectedPortName();
    /// Returns currently selected parity as text to be saved in settings
    QString currentParityText();
    /// Returns currently selected flow control as text to be saved in settings
    QString currentFlowControlText();

private slots:

    void loadBaudRateList();
    void togglePort();
    // void selectListedPort(QString portName);

    // void _selectBaudRate(QString baudRate);
    // void selectParity(int parity); // parity must be one of QSerialPort::Parity
    // void selectDataBits(int dataBits); // bits must be one of QSerialPort::DataBits
    // void selectStopBits(int stopBits); // stopBits must be one of QSerialPort::StopBits
    // void selectFlowControl(int flowControl); // flowControl must be one of QSerialPort::FlowControl

    void openActionTriggered(bool checked);
    // void onCbPortListActivated(int index);
    // void onTbPortListActivated(int index);
    // void onPortError(QSerialPort::SerialPortError error);
    void portToggled(bool opened);


public slots:
    void updatePinLeds(QFlags<QSerialPort::PinoutSignal> pinOut);
    void setmaxBitRate(quint64 maxBitRate);
    void loadPortList();

signals:
    void portToggledSignal(bool open);
    // void updatePinLedsSignal(void);
    void togglePortSignal(QString portName, int baudRate, int parity,
                          int dataBits, int stopBits, int flowControl, bool setDtr, bool setRts);
    void selectBaudRateSignal(const QString &baudRate);
    void selectParitySignal(int parity); // parity must be one of QSerialPort::Parity
    void selectDataBitsSignal(int dataBits); // bits must be one of QSerialPort::DataBits
    void selectStopBitsSignal(int stopBits); // stopBits must be one of QSerialPort::StopBits
    void selectFlowControlSignal(int flowControl); // flowControl must be one of QSerialPort::FlowControl
    void setDtrSignal(bool toSet);
    void setRtsSignal(bool toSet);
};

#endif // PORTCONTROL_H
