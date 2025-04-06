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

#include "bpslabel.h"

const char* BPS_TOOLTIP = "bits per second";
const char* BPS_TOOLTIP2 = "kbits per second";
const char* BPS_TOOLTIP_ERR = "Maximum baud rate may be reached!";

BPSLabel::BPSLabel(PortControl* portControl,
                   DataFormatPanel* dataFormatPanel,
                   QWidget *parent) :
    QLabel(parent)
{
    _portControl = portControl;
    _dataFormatPanel = dataFormatPanel;
    prevBytesRead = 0;

    setText("0bps");
    setToolTip(tr(BPS_TOOLTIP));

    connect(&bpsTimer, &QTimer::timeout,
            this, &BPSLabel::onBpsTimeout);

    connect(portControl, &PortControl::portToggledSignal,
            this, &BPSLabel::onDataTransferStart);
}

void BPSLabel::setBLEConfig(BLE_Config *bleConfig)
{
    _bleConfig=bleConfig;
    connect(_bleConfig,&BLE_Config::bleDeviceConnectedSignal,this,&BPSLabel::onDataTransferStart,Qt::QueuedConnection);
}

QString BPSLabel::getAutoScaledSpeed(quint64 bitsRate)
{
    QString str;
    if(bitsRate>2)
    {
        bitsRate/=1024;
        str = QString(tr("%1kbps")).arg(bitsRate);
        setToolTip(tr(BPS_TOOLTIP2));
    }
    else
    {
        str = QString(tr("%1bps")).arg(bitsRate);
        setToolTip(tr(BPS_TOOLTIP));
    }
    return str;
}

void BPSLabel::onBpsTimeout()
{
    quint64 curBytesRead = _dataFormatPanel->bytesRead();
    quint64 bytesRead = curBytesRead - prevBytesRead;
// #ifdef QT_DEBUG
//     qInfo()<<"Current read: "<<curBytesRead<<"Prev:"<<prevBytesRead<<"Diff: "<<bytesRead;
// #endif
    prevBytesRead = curBytesRead;

    quint64 bits = bytesRead * 8;
    // qInfo()<<"Total bits:"<<bits;
    quint64 maxBps = _portControl->maxBitRate();
    // qInfo()<<"Max bit rate: "<<maxBps;
    QString str=getAutoScaledSpeed(bits);
    if ( (_dataFormatPanel->currentSelectedDevice()==AbstractDevice::INPUT_DEVICE_SERIAL_PORT) && (bits >= maxBps))
    {
        // TODO: an icon for bps warning
        str = QString(tr("!%1/%2bps")).arg(bits).arg(maxBps);
        setToolTip(tr(BPS_TOOLTIP_ERR));
    }
    // else
    // {
    //     str = QString(tr("%1bps")).arg(bits);

    // }
    setText(str);
}

void BPSLabel::onDataTransferStart(bool started)
{
    if (started)
    {
        bpsTimer.start(1000);
    }
    else
    {
        bpsTimer.stop();
        // if not cleared last displayed value is stuck
        setText("0bps");
        setToolTip(tr(BPS_TOOLTIP));
    }
}
