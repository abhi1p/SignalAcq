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

#include "dataformatpanel.h"
#include "ui_dataformatpanel.h"

#include <QRadioButton>
#include <QtDebug>

#include "utils.h"
#include "setting_defines.h"

DataFormatPanel::DataFormatPanel(AbstractDevice *device, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataFormatPanel),
    bsReader(device, this),
    asciiReader(device, this),
    framedReader(device, this),
    demoReader(device, this)
{
    ui->setupUi(this);

    m_device = device;
    paused = false;
    readerBeforeDemo = nullptr;
    _bytesRead = 0;

    // initalize default reader
    currentReader = &bsReader;
    bsReader.enable();
    ui->rbBinary->setChecked(true);
    /// Replaced with stacked widget
    // ui->horizontalLayout->addWidget(bsReader.settingsWidget(), 1);

    ui->dataFormatSettingsStackedWidget->insertWidget(0,bsReader.settingsWidget());
    ui->dataFormatSettingsStackedWidget->insertWidget(1,asciiReader.settingsWidget());
    ui->dataFormatSettingsStackedWidget->insertWidget(2,framedReader.settingsWidget());
    ui->dataFormatSettingsStackedWidget->insertWidget(3,demoReader.settingsWidget());

    ui->dataFormatSettingsStackedWidget->setCurrentIndex(0); // select bisnary reader

    // initalize reader selection buttons group
    readerSelectButtons.addButton(ui->rbBinary);
    readerSelectButtons.addButton(ui->rbAscii);
    readerSelectButtons.addButton(ui->rbFramed);

    connect(ui->rbBinary, &QRadioButton::toggled, this,[this](bool checked)
            {
                if (checked) selectReader(&bsReader);
            });

    connect(ui->rbAscii, &QRadioButton::toggled, this,[this](bool checked)
            {
                if (checked) selectReader(&asciiReader);
            });

    connect(ui->rbFramed, &QRadioButton::toggled, this,[this](bool checked)
            {
                if (checked) selectReader(&framedReader);
            });

    m_bleCharModel=new BluetoothCharModel(this);
    ui->sourceCharacteristicsComboBox->setModel(m_bleCharModel);
    addCharacteristics(QBluetoothUuid(),QBluetoothUuid(),QLowEnergyCharacteristic::Notify); //add a dummy char
    connect(ui->sourceCharacteristicsComboBox,&QComboBox::currentIndexChanged,this,&DataFormatPanel::sourceCharacteristcsSelected,Qt::QueuedConnection);
    connect(currentReader,&AbstractReader::log,this,&DataFormatPanel::log,Qt::QueuedConnection);

}


DataFormatPanel::~DataFormatPanel()
{
    delete m_bleCharModel;
    delete ui;
}

unsigned DataFormatPanel::numChannels() const
{
    return currentReader->numChannels();
}

Source* DataFormatPanel::activeSource()
{
    return currentReader;
}

void DataFormatPanel::pause(bool enabled)
{
    paused = enabled;
    currentReader->pause(enabled);
    demoReader.pause(enabled);
}

void DataFormatPanel::enableDemo(bool demoEnabled)
{
    if (demoEnabled)
    {
        readerBeforeDemo = currentReader;
        demoReader.setNumChannels(readerBeforeDemo->numChannels());
        selectReader(&demoReader);
    }
    else
    {
        Q_ASSERT(readerBeforeDemo != nullptr);
        selectReader(readerBeforeDemo);
    }

    // disable/enable reader selection buttons during/after demo
    ui->rbAscii->setDisabled(demoEnabled);
    ui->rbBinary->setDisabled(demoEnabled);
    ui->rbFramed->setDisabled(demoEnabled);
}

void DataFormatPanel::bleDeviceConnected(bool connected)
{
    if(!connected)
    {
        // m_srcCharList.clear();
        qInfo()<<"Clear all char";
        ui->sourceCharacteristicsComboBox->blockSignals(true);
        m_bleCharModel->clearAll();
        ui->sourceCharacteristicsComboBox->blockSignals(false);
    }
}

void DataFormatPanel::addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag)
{
    if(flag & QLowEnergyCharacteristic::Notify)
    {
        m_bleCharModel->addCharacteristic(srvcUuid,charUuid,flag);
    }

}

void DataFormatPanel::sourceCharacteristcsSelected(int index)
{
    // qInfo()<<
    QString uuidStr=ui->sourceCharacteristicsComboBox->currentText();
    // qInfo()<<"Source char selected: "<<uuidStr;
    // QBluetoothUuid uuid(uuidStr);
    // if(!uuid.isNull())
    // {
    //    emit selectedSourceCharacteristicSignal(uuid);
    // }
    emit selectedSourceCharacteristicSignal(uuidStr);

}

bool DataFormatPanel::isDemoEnabled() const
{
    return currentReader == &demoReader;
}

void DataFormatPanel::selectReader(AbstractReader* reader)
{
    currentReader->enable(false);
    reader->enable();

    // re-connect signals
    disconnect(currentReader, 0, this, 0);

    /// switch the settings widget

    // ui->horizontalLayout->removeWidget(currentReader->settingsWidget());
    // currentReader->settingsWidget()->hide();
    // ui->horizontalLayout->addWidget(reader->settingsWidget(), 1);
    // reader->settingsWidget()->show();

    //select current widget from stacked widget
    ui->dataFormatSettingsStackedWidget->setCurrentWidget(reader->settingsWidget());

    reader->pause(paused);

    currentReader = reader;
    emit sourceChanged(currentReader);
}

uint64_t DataFormatPanel::bytesRead()
{
    _bytesRead += currentReader->getBytesRead();
    return _bytesRead;
}

void DataFormatPanel::saveSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_DataFormat);

    // save selected data format (current reader)
    QString format;
    AbstractReader* selectedReader = isDemoEnabled() ? readerBeforeDemo : currentReader;
    if (selectedReader == &bsReader)
    {
        format = "binary";
    }
    else if (selectedReader == &asciiReader)
    {
        format = "ascii";
    }
    else // framed reader
    {
        format = "custom";
    }
    settings->setValue(SG_DataFormat_Format, format);

    settings->endGroup();

    // save reader settings
    bsReader.saveSettings(settings);
    asciiReader.saveSettings(settings);
    framedReader.saveSettings(settings);
}

void DataFormatPanel::loadSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_DataFormat);

    // load selected format
    QString format = settings->value(
        SG_DataFormat_Format, QString()).toString();

    if (format == "binary")
    {
        selectReader(&bsReader);
        ui->rbBinary->setChecked(true);
    }
    else if (format == "ascii")
    {
        selectReader(&asciiReader);
        ui->rbAscii->setChecked(true);
    }
    else if (format == "custom")
    {
        selectReader(&framedReader);
        ui->rbFramed->setChecked(true);
    } // else current selection stays

    settings->endGroup();

    // load reader settings
    bsReader.loadSettings(settings);
    asciiReader.loadSettings(settings);
    framedReader.loadSettings(settings);
}

AbstractDevice::INPUT_DEVICE DataFormatPanel::currentSelectedDevice()
{
    return m_device->currentDevice();
}
