/*
  Copyright © 2021 Hasan Yavuz Özderya

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

#ifndef DATAFORMATPANEL_H
#define DATAFORMATPANEL_H

#include <stdint.h>
#include <QWidget>
#include <QButtonGroup>
#include <QSerialPort>
#include <QList>
#include <QSettings>
#include <QtGlobal>
#include <QButtonGroup>

#include "Common/binarystreamreader.h"
#include "Common/asciireader.h"
#include "Common/demoreader.h"
#include "device_model.h"
#include "Common/framedreader.h"
#include "Common/datarecorder.h"

#include "Common/abstractdevice.h"

namespace Ui {
class DataFormatPanel;
}

class DataFormatPanel : public QWidget
{
    Q_OBJECT

public:
    // explicit DataFormatPanel(QSerialPort* port, QWidget* parent = 0);
    explicit DataFormatPanel(AbstractDevice* device, QWidget* parent = 0);
    ~DataFormatPanel();

    /// Returns currently selected number of channels
    unsigned numChannels() const;
    /// Returns active source (reader)
    Source* activeSource();
    /// Returns total number of bytes read
    uint64_t bytesRead();
    /// Stores data format panel settings into a `QSettings`
    void saveSettings(QSettings* settings);
    /// Loads data format panel settings from a `QSettings`.
    void loadSettings(QSettings* settings);

    AbstractDevice::INPUT_DEVICE currentSelectedDevice();

public slots:
    void pause(bool);
    void enableDemo(bool); // demo shouldn't be enabled when port is open

    void bleDeviceConnected(bool connected);
    void addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag);

private slots:
    void sourceCharacteristcsSelected(int index);

signals:
    /// Active (selected) reader has changed.
    void sourceChanged(Source* source);
    void selectedSourceCharacteristicSignal(QString uuid);
    void log(QtMsgType type, QString text);

private:
    Ui::DataFormatPanel *ui;
    QButtonGroup readerSelectButtons;

    // QSerialPort* serialPort;
    AbstractDevice *m_device;

    BinaryStreamReader bsReader;
    AsciiReader asciiReader;
    FramedReader framedReader;
    /// Currently selected reader
    AbstractReader* currentReader;
    /// Disable current reader and enable a another one
    void selectReader(AbstractReader* reader);

    bool paused;
    uint64_t _bytesRead;

    DemoReader demoReader;
    AbstractReader* readerBeforeDemo;

    bool isDemoEnabled() const;

    // QStringList m_srcCharList;
    BluetoothCharModel *m_bleCharModel;

};

#endif // DATAFORMATPANEL_H
