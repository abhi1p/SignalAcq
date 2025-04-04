#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QByteArray>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QDesktopServices>
#include <QMap>
#include <QtDebug>
#include <QCommandLineParser>
#include <QFileInfo>
#include <qwt_plot.h>
#include <Plot/plot.h>
#include <Plot/barplot.h>
#include "defines.h"
#include "version.h"
#include "setting_defines.h"

// // TODO: depends on tab insertion order, a better solution would be to use object names
// const QMap<int, QString> panelSettingMap({
//     {0, "Port"},
//     {1, "DataFormat"},
//     {2, "Plot"},
//     {3, "Commands"},
//     {4, "Record"},
//     {5, "TextView"},
//     {6, "Log"}
// });

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    m_aboutDialog(this),
    m_portControl(),
    m_secondaryPlot(NULL),
    m_snapshotMan(this, &m_stream),
    m_commandPanel(),
    m_dataFormatPanel(&m_device),
    m_recordPanel(&m_stream),
    m_textView(&m_stream),
    m_updateCheckDialog(this),
    m_bpsLabel(&m_portControl, &m_dataFormatPanel, this)
    // m_device()
{
    ui->setupUi(this);
    m_plotMan = new PlotManager(ui->plotArea, &m_plotMenu, &m_stream);

    connect(this,&MainWindow::updateInputDeviceSignal,&m_device,&AbstractDevice::updateInputDevice,Qt::QueuedConnection);
    m_bleConfig=new BLE_Config(&m_bpsLabel ,this);
    // m_filterBase=new Filter_base(this);
    ui->deviceStackedWidget->insertWidget(0,&m_portControl);
    ui->deviceStackedWidget->insertWidget(1,m_bleConfig);
    // ui->deviceStackedWidget->setCurrentIndex(0);
    selectCurrentActiveDevice(m_deviceList[0]);
    connect(ui->deviceStackedWidget,&QStackedWidget::currentChanged,&m_commandPanel,&CommandPanel::setCurrentIndex,Qt::QueuedConnection);

    m_deviceListMap[m_deviceList[0]]= AbstractDevice::INPUT_DEVICE_SERIAL_PORT;
    m_deviceListMap[m_deviceList[1]]= AbstractDevice::INPUT_DEVICE_BLE;

    ui->tabWidget->insertTab(1, &m_dataFormatPanel, "Data Format");
    ui->tabWidget->insertTab(2, &m_plotControlPanel, "Plot");
    ui->tabWidget->insertTab(3, &m_commandPanel, "Commands");
    ui->tabWidget->insertTab(4, &m_recordPanel, "Record");
    ui->tabWidget->insertTab(5, &m_textView, "Text View");
    // ui->tabWidget->insertTab(6, m_filterBase, "Filters");
    ui->tabWidget->setCurrentIndex(0);
    // auto tbPortControl = m_portControl.toolBar();
    // addToolBar(tbPortControl);

    addDeviceSelect_UI();

    addToolBar(m_recordPanel.toolbar());

    ui->plotToolBar->addAction(m_snapshotMan.takeSnapshotAction());
    menuBar()->insertMenu(ui->menuHelp->menuAction(), m_snapshotMan.menu());
    menuBar()->insertMenu(ui->menuHelp->menuAction(), m_commandPanel.menu());

    connect(&m_commandPanel, &CommandPanel::focusRequested, this,[this]()
            {
                this->ui->tabWidget->setCurrentWidget(&m_commandPanel);
                this->ui->tabWidget->showTabs();
            });
    // connect(&m_commandPanel)

    // tbPortControl->setObjectName("tbPortControl");
    // ui->plotToolBar->setObjectName("tbPlot");

    setupAboutDialog();

    // init view menu
    ui->menuBar->insertMenu(ui->menuSecondary->menuAction(), &m_plotMenu);
    m_plotMenu.addSeparator();
    QMenu* tbMenu = m_plotMenu.addMenu("Toolbars");
    tbMenu->addAction(ui->plotToolBar->toggleViewAction());
    // tbMenu->addAction(m_portControl.toolBar()->toggleViewAction());

    // init secondary plot menu
    auto group = new QActionGroup(this);
    group->addAction(ui->actionVertical);
    group->addAction(ui->actionHorizontal);

    // init UI signals

    // Secondary plot menu signals
    connect(ui->actionBarPlot, &QAction::triggered,
            this, &MainWindow::showBarPlot);

    connect(ui->actionVertical, &QAction::triggered,
            this,[this](bool checked)
            {
                if (checked) ui->splitter->setOrientation(Qt::Vertical);
            });

    connect(ui->actionHorizontal, &QAction::triggered,
            this,[this](bool checked)
            {
                if (checked) ui->splitter->setOrientation(Qt::Horizontal);
            });

    // Help menu signals
    QObject::connect(ui->actionHelpAbout, &QAction::triggered,
                     &m_aboutDialog, &QWidget::show);

    QObject::connect(ui->actionCheckUpdate, &QAction::triggered,
                     &m_updateCheckDialog, &QWidget::show);

    QObject::connect(ui->actionReportBug, &QAction::triggered,
                     [](){QDesktopServices::openUrl(QUrl(BUG_REPORT_URL));});

    // File menu signals
    QObject::connect(ui->actionExportCsv, &QAction::triggered,
                     this, &MainWindow::onExportCsv);

    QObject::connect(ui->actionExportSvg, &QAction::triggered,
                     this, &MainWindow::onExportSvg);

    QObject::connect(ui->actionSaveSettings, &QAction::triggered,
                     this, &MainWindow::onSaveSettings);

    QObject::connect(ui->actionLoadSettings, &QAction::triggered,
                     this, &MainWindow::onLoadSettings);

    ui->actionQuit->setShortcutContext(Qt::ApplicationShortcut);

    QObject::connect(ui->actionQuit, &QAction::triggered,
                     this, &MainWindow::close);

    // port control signals
    QObject::connect(&m_portControl, &PortControl::portToggledSignal,
                     this, &MainWindow::onPortToggled);

    // plot control signals
    connect(&m_plotControlPanel, &PlotControlPanel::numOfSamplesChanged,
            this, &MainWindow::onNumOfSamplesChanged);

    connect(&m_plotControlPanel, &PlotControlPanel::numOfSamplesChanged,
            m_plotMan, &PlotManager::setNumOfSamples);

    connect(&m_plotControlPanel, &PlotControlPanel::yScaleChanged,
            m_plotMan, &PlotManager::setYAxis);

    connect(&m_plotControlPanel, &PlotControlPanel::xScaleChanged,
            &m_stream, &Stream::setXAxis);

    connect(&m_plotControlPanel, &PlotControlPanel::xScaleChanged,
            m_plotMan, &PlotManager::setXAxis);

    connect(&m_plotControlPanel, &PlotControlPanel::plotWidthChanged,
            m_plotMan, &PlotManager::setPlotWidth);

    connect(&m_plotControlPanel, &PlotControlPanel::lineThicknessChanged,
            m_plotMan, &PlotManager::setLineThickness);

    // plot toolbar signals
    QObject::connect(ui->actionClear, SIGNAL(triggered(bool)),
                     this, SLOT(clearPlot()));

    QObject::connect(m_snapshotMan.takeSnapshotAction(), &QAction::triggered,
                     m_plotMan, &PlotManager::flashSnapshotOverlay);

    QObject::connect(ui->actionPause, &QAction::triggered,
                     &m_stream, &Stream::pause);

    QObject::connect(ui->actionPause, &QAction::triggered,
                     this,[this](bool enabled)
                     {
                         if (enabled && !m_recordPanel.recordPaused())
                         {
                             m_dataFormatPanel.pause(true);
                         }
                         else
                         {
                             m_dataFormatPanel.pause(false);
                         }
                     });

    QObject::connect(&m_recordPanel, &RecordPanel::recordPausedChanged,
                     this,[this](bool enabled)
                     {
                         if (ui->actionPause->isChecked() && enabled)
                         {
                             m_dataFormatPanel.pause(false);
                         }
                     });

    // connect(&m_serialPortDevice, &QIODevice::aboutToClose,
    //         &m_recordPanel, &RecordPanel::onPortClose);

    // init plot
    m_numOfSamples = m_plotControlPanel.numOfSamples();
    m_stream.setNumSamples(m_numOfSamples);
    m_plotControlPanel.setChannelInfoModel(m_stream.infoModel());

    // init scales
    m_stream.setXAxis(m_plotControlPanel.xAxisAsIndex(),
                    m_plotControlPanel.xMin(), m_plotControlPanel.xMax());

    m_plotMan->setYAxis(m_plotControlPanel.autoScale(),
                      m_plotControlPanel.yMin(), m_plotControlPanel.yMax());
    m_plotMan->setXAxis(m_plotControlPanel.xAxisAsIndex(),
                      m_plotControlPanel.xMin(), m_plotControlPanel.xMax());
    m_plotMan->setNumOfSamples(m_numOfSamples);
    m_plotMan->setPlotWidth(m_plotControlPanel.plotWidth());

    // init bps (bits per second) counter
    ui->statusBar->addPermanentWidget(&m_bpsLabel);

    // Init sps (sample per second) counter
    m_spsLabel.setText("0sps");
    m_spsLabel.setToolTip(tr("samples per second (per channel)"));
    ui->statusBar->addPermanentWidget(&m_spsLabel);
    connect(&m_sampleCounter, &SampleCounter::spsChanged,
            this, &MainWindow::onSpsChanged);

    m_bpsLabel.setMinimumWidth(70);
    m_bpsLabel.setAlignment(Qt::AlignRight);
    m_spsLabel.setMinimumWidth(70);
    m_spsLabel.setAlignment(Qt::AlignRight);

    // init demo
    QObject::connect(ui->actionDemoMode, &QAction::toggled,
                     this, &MainWindow::enableDemo);

    QObject::connect(ui->actionDemoMode, &QAction::toggled,
                     m_plotMan, &PlotManager::showDemoIndicator);

    // init m_stream connections
    connect(&m_dataFormatPanel, &DataFormatPanel::sourceChanged,
            this, &MainWindow::onSourceChanged);
    onSourceChanged(m_dataFormatPanel.activeSource());

    // load default settings
    QSettings settings(PROGRAM_NAME, PROGRAM_NAME);
    loadAllSettings(&settings);

    handleCommandLineOptions(*QApplication::instance());

    // // ensure command panel has 1 command if none loaded
    // if (!m_commandPanel.numOfCommandsSerial())
    // {
    //     m_commandPanel.newCommandAction()->trigger();
    // }

    // Important: This should be after newCommandAction is triggered
    // (above) we don't want user to be greeted with command panel on
    // the very first run.
    connect(m_commandPanel.newCommandAction(), &QAction::triggered, this,[this]()
            {
                this->ui->tabWidget->setCurrentWidget(&m_commandPanel);
                this->ui->tabWidget->showTabs();
            });
}

MainWindow::~MainWindow()
{
    delete m_plotMan;
    delete m_bleConfig;
    // delete m_filterBase;
    delete m_deviceSelectComboBox;
    delete ui;
}



void MainWindow::connectBLESignalsAndSlots(BLE *ble)
{
    connect(m_bleConfig,&BLE_Config::refreshBLEdevicesSignal,ble,&BLE::startBLEdiscovery,Qt::QueuedConnection);
    connect(m_bleConfig,&BLE_Config::connectToBleDeviceSignal,ble,&BLE::connectToBleDevice,Qt::QueuedConnection);
    connect(m_bleConfig,&BLE_Config::readCharacteristicsSignal,ble,&BLE::readCharacteristics,Qt::QueuedConnection);
    connect(m_bleConfig,&BLE_Config::writeCharacteristicsSignal,ble,&BLE::writeCharacteristics,Qt::QueuedConnection);
    connect(&m_commandPanel,&CommandPanel::writeCharacteristicsSignal,ble,&BLE::writeCharacteristics,Qt::QueuedConnection);
    connect(m_bleConfig,&BLE_Config::enableNotificationSignal,ble,&BLE::enableNotification,Qt::QueuedConnection);

    connect(ble,&BLE::bleScanFinished,m_bleConfig,&BLE_Config::stopRefreshLoadingIndicator,Qt::QueuedConnection);
    connect(ble,&BLE::addDiscoveredBLEdevicesSignal,m_bleConfig,&BLE_Config::addDiscoveredBLEdevices,Qt::QueuedConnection);
    connect(ble,&BLE::bleDeviceConnectedSignal,m_bleConfig,&BLE_Config::bleDeviceConnected,Qt::QueuedConnection);

    connect(ble,&BLE::addCharacteristicsSignal,m_bleConfig,&BLE_Config::addCharacteristics,Qt::QueuedConnection);
    connect(ble,&BLE::readCharacteristicsSuccessSignal,m_bleConfig,&BLE_Config::readCharacteristicsSuccess,Qt::QueuedConnection);
    connect(ble,&BLE::characteristicChangedSignal,m_bleConfig,&BLE_Config::characteristicChanged,Qt::QueuedConnection);

    connect(ble,&BLE::addCharacteristicsSignal,&m_dataFormatPanel,&DataFormatPanel::addCharacteristics,Qt::QueuedConnection);
    connect(ble,&BLE::addCharacteristicsSignal,&m_commandPanel,&CommandPanel::addCharacteristicsSignal,Qt::QueuedConnection);
    connect(ble,&BLE::addCharacteristicsSignal,&m_commandPanel,&CommandPanel::addCharacteristics,Qt::QueuedConnection);
    connect(ble,&BLE::bleDeviceConnectedSignal,&m_dataFormatPanel,&DataFormatPanel::bleDeviceConnected,Qt::QueuedConnection);
    connect(ble,&BLE::bleDeviceConnectedSignal,this,&MainWindow::deviceConnected,Qt::QueuedConnection);
    connect(ble,&BLE::bleDeviceConnectedSignal,&m_commandPanel,&CommandPanel::bleConnected,Qt::QueuedConnection);
    connect(&m_dataFormatPanel,&DataFormatPanel::selectedSourceCharacteristicSignal,ble,&BLE::selectedSourceCharacteristic,Qt::QueuedConnection);

    //connect(ble,&BLE::readyRead,&m_device,&AbstractDevice::readyRead,Qt::QueuedConnection);
    connect(ble,&BLE::characteristicChangedSignal_plot,&m_device,&AbstractDevice::characteristicChanged,Qt::QueuedConnection);

}

void MainWindow::connectSerialPortSignalsAndSlots(SerialPort *serial)
{
    connect(serial,&SerialPort::serialDataReadSignal,&m_device,&AbstractDevice::serialDataRead,Qt::QueuedConnection);
    connect(serial,&SerialPort::maxBitRateSignal,&m_portControl,&PortControl::setmaxBitRate,Qt::QueuedConnection);
    connect(serial,&SerialPort::updatePinLedsSignal,&m_portControl,&PortControl::updatePinLeds,Qt::QueuedConnection);
    connect(serial,&SerialPort::portToggledSignal,&m_portControl,&PortControl::portToggledSignal,Qt::QueuedConnection);
    connect(serial,&SerialPort::loadPortListSignal,&m_portControl,&PortControl::loadPortList,Qt::QueuedConnection);


    connect(&m_portControl,&PortControl::togglePortSignal,serial,&SerialPort::togglePort,Qt::QueuedConnection);
    connect(&m_portControl,&PortControl::selectBaudRateSignal,serial,&SerialPort::selectBaudRate,Qt::QueuedConnection);
    connect(&m_portControl,&PortControl::selectParitySignal,serial,&SerialPort::selectParity,Qt::QueuedConnection);
    connect(&m_portControl,&PortControl::selectDataBitsSignal,serial,&SerialPort::selectDataBits,Qt::QueuedConnection);
    connect(&m_portControl,&PortControl::selectStopBitsSignal,serial,&SerialPort::selectStopBits,Qt::QueuedConnection);
    connect(&m_portControl,&PortControl::selectFlowControlSignal,serial,&SerialPort::selectFlowControl,Qt::QueuedConnection);
    connect(&m_portControl,&PortControl::setDtrSignal,serial,&SerialPort::setDtr,Qt::QueuedConnection);
    connect(&m_portControl,&PortControl::setRtsSignal,serial,&SerialPort::setRts,Qt::QueuedConnection);

}


void MainWindow::closeEvent(QCloseEvent * event)
{
    // save snapshots
    if (!m_snapshotMan.isAllSaved())
    {
        auto clickedButton = QMessageBox::warning(
            this, "Closing SerialPlot",
            "There are un-saved snapshots. If you close you will loose the data.",
            QMessageBox::Discard, QMessageBox::Cancel);
        if (clickedButton == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
    }

    // save settings
    QSettings settings(PROGRAM_NAME, PROGRAM_NAME);
    saveAllSettings(&settings);
    settings.sync();

    if (settings.status() != QSettings::NoError)
    {
        QString errorText;

        if (settings.status() == QSettings::AccessError)
        {
            QString file = settings.fileName();
            errorText = QString("Serialplot cannot save settings due to access error. \
This happens if you have run serialplot as root (with sudo for ex.) previously. \
Try fixing the permissions of file: %1, or just delete it.").arg(file);
        }
        else
        {
            errorText = QString("Serialplot cannot save settings due to unknown error: %1").\
                arg(settings.status());
        }

        auto button = QMessageBox::critical(
            NULL,
            "Failed to save settings!", errorText,
            QMessageBox::Cancel | QMessageBox::Ok);

        if (button == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::addDeviceSelect_UI()
{

    //add source select combo box
    m_deviceSelectComboBox=new QComboBox(this);
    m_deviceSelectComboBox->setToolTip("Select device type");
    m_conDconLabel=new QLabel(this);
    // m_conDconLabel->toolTipDuration()
    m_conDconLabel->setMaximumSize(22,20);
    m_conDconLabel->setScaledContents(true);
    // m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/light_red.png"));
    m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/usb_disconnected.png"));
    m_conDconLabel->setToolTip("Serial port closed");
    m_deviceSelectComboBox->addItems(m_deviceList);
    m_deviceSelectToolbar.setWindowTitle("Device Select Toolbar");
    m_deviceSelectToolbar.addWidget(m_deviceSelectComboBox);
    m_deviceSelectToolbar.addSeparator();
    m_deviceSelectToolbar.addWidget(m_conDconLabel);

    addToolBar(&m_deviceSelectToolbar);

    connect(m_deviceSelectComboBox,&QComboBox::currentIndexChanged,this,&MainWindow::deviceSelectIndexChanged,Qt::QueuedConnection);
    qInfo()<<"Added source select UI";

}

void MainWindow::deviceSelectIndexChanged(int index)
{
    qInfo()<<"Current selected"<<m_deviceList[index];
    // ui->deviceStackedWidget->setCurrentIndex(index);
    selectCurrentActiveDevice(m_deviceList[index]);
    if(m_deviceList[index]=="USB Serial")
    {
        m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/usb_disconnected.png"));
        m_conDconLabel->setToolTip("Serial port closed");
    }
    else if(m_deviceList[index]=="BLE")
    {
        m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/bluetooth_off.png"));
        m_conDconLabel->setToolTip("BLE device disconnected");
    }
}

void MainWindow::addSourceTab(AbstractDevice::INPUT_DEVICE device)
{
    if(device==AbstractDevice::INPUT_DEVICE_BLE)
    {

    }
    else if(device==AbstractDevice::INPUT_DEVICE_SERIAL_PORT)
    {

    }
}

void MainWindow::selectCurrentActiveDevice(QString device)
{
    int index=m_deviceList.indexOf(device);
    qInfo()<<"Current index:"<<index;
    if(index!=-1)
    {
        int sLen=ui->deviceStackedWidget->count();
        qInfo()<<"Stack widget count:"<<sLen;
        if(index<sLen)
        {
            ui->deviceStackedWidget->setCurrentIndex(index);
            // QString text=ui->device
            emit updateInputDeviceSignal(m_deviceListMap[device]);
            ui->tabWidget->setTabText(0,device);
        }
        else
        {
            qInfo()<<"Device not available";
        }

    }

}

void MainWindow::deviceConnected(bool connected)
{
    if(connected)
    {
        m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/bluetooth_on.png"));
        // m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/light_green.png"));
        m_conDconLabel->setToolTip("BLE device connected");
        m_deviceSelectComboBox->setEnabled(false);
    }
    else
    {
        m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/bluetooth_off.png"));
        // m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/light_red.png"));
        m_conDconLabel->setToolTip("BLE device disconnected");
        m_deviceSelectComboBox->setEnabled(true);
    }
}

void MainWindow::serialPortOpened(bool opened)
{
    if(opened)
    {
        m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/usb_connected.png"));
        m_conDconLabel->setToolTip("Serial port opened");
        m_deviceSelectComboBox->setEnabled(false);
    }
    else
    {
        m_conDconLabel->setPixmap(QPixmap(":/icons/tango/tango/usb_disconnected.png"));
        m_conDconLabel->setToolTip("Serial port closed");
        m_deviceSelectComboBox->setEnabled(true);
    }
}

void MainWindow::addWidgetToTab(QTabWidget *tabWidget, QWidget *widget, QString text)
{
    if(tabWidget->indexOf(widget)==-1)
    {
        tabWidget->addTab(widget,text);
    }
}

void MainWindow::showTab(QTabWidget *tabWidget, QWidget *widget)
{
    if(!tabWidget->isVisible())
    {
        tabWidget->setVisible(true);
    }
}


void MainWindow::setupAboutDialog()
{
    Ui_AboutDialog uiAboutDialog;
    uiAboutDialog.setupUi(&m_aboutDialog);

    QObject::connect(uiAboutDialog.pbAboutQt, &QPushButton::clicked,
                     [](){ QApplication::aboutQt();});

    QString aboutText = uiAboutDialog.lbAbout->text();
    aboutText.replace("$VERSION_STRING$", VERSION_STRING);
    aboutText.replace("$VERSION_REVISION$", VERSION_REVISION);
    uiAboutDialog.lbAbout->setText(aboutText);
}

void MainWindow::onPortToggled(bool open)
{
    // make sure demo mode is disabled
    if (open && isDemoRunning()) enableDemo(false);
    ui->actionDemoMode->setEnabled(!open);
    serialPortOpened(open);
    if (!open)
    {

        m_spsLabel.setText("0sps");
    }
}

void MainWindow::onSourceChanged(Source* source)
{
    source->connectSink(&m_stream);
    source->connectSink(&m_sampleCounter);
}

void MainWindow::clearPlot()
{
    m_stream.clear();
    m_plotMan->replot();
}

void MainWindow::onNumOfSamplesChanged(int value)
{
    m_numOfSamples = value;
    m_stream.setNumSamples(value);
    m_plotMan->replot();
}

void MainWindow::onSpsChanged(float sps)
{
    int precision = sps < 1. ? 3 : 0;
    m_spsLabel.setText(QString::number(sps, 'f', precision) + "sps");
}

bool MainWindow::isDemoRunning()
{
    return ui->actionDemoMode->isChecked();
}

void MainWindow::enableDemo(bool enabled)
{
    qInfo()<<"Enable demo";
    if (enabled)
    {
        // if (!m_serialPortDevice.isOpen())
        if (true)
        {
            m_dataFormatPanel.enableDemo(true);
        }
        else
        {
            ui->actionDemoMode->setChecked(false);
        }
    }
    else
    {
        m_dataFormatPanel.enableDemo(false);
        ui->actionDemoMode->setChecked(false);
    }
}

void MainWindow::showSecondary(QWidget* wid)
{
    if (m_secondaryPlot != NULL)
    {
        m_secondaryPlot->deleteLater();
    }

    m_secondaryPlot = wid;
    ui->splitter->addWidget(wid);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);
}

void MainWindow::hideSecondary()
{
    if (m_secondaryPlot == NULL)
    {
        qFatal("Secondary plot doesn't exist!");
    }

    m_secondaryPlot->deleteLater();
    m_secondaryPlot = NULL;
}

void MainWindow::showBarPlot(bool show)
{
    if (show)
    {
        auto plot = new BarPlot(&m_stream, &m_plotMenu);
        plot->setYAxis(m_plotControlPanel.autoScale(),
                       m_plotControlPanel.yMin(),
                       m_plotControlPanel.yMax());
        connect(&m_plotControlPanel, &PlotControlPanel::yScaleChanged,
                plot, &BarPlot::setYAxis);
        showSecondary(plot);
    }
    else
    {
        hideSecondary();
    }
}

void MainWindow::onExportCsv()
{
    bool wasPaused = ui->actionPause->isChecked();
    ui->actionPause->setChecked(true); // pause plotting

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV File"));

    if (fileName.isNull())  // user canceled export
    {
        ui->actionPause->setChecked(wasPaused);
    }
    else
    {
        Snapshot* snapshot = m_snapshotMan.makeSnapshot();
        snapshot->save(fileName);
        delete snapshot;
    }
}

void MainWindow::onExportSvg()
{
    bool wasPaused = ui->actionPause->isChecked();
    ui->actionPause->setChecked(true); // pause plotting

    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Export SVG File(s)"), QString(), "Images (*.svg)");

    if (fileName.isNull())  // user canceled export
    {
        ui->actionPause->setChecked(wasPaused);
    }
    else
    {
        m_plotMan->exportSvg(fileName);
    }
}

PlotViewSettings MainWindow::viewSettings() const
{
    return m_plotMenu.viewSettings();
}

void MainWindow::messageHandler(QtMsgType type,
                                const QString &logString,
                                const QString &msg)
{
    if (ui != NULL)
        ui->ptLog->appendPlainText(logString);

    if (type != QtDebugMsg && ui != NULL)
    {
        ui->statusBar->showMessage(msg, 5000);
    }
}

void MainWindow::saveAllSettings(QSettings* settings)
{
    saveMWSettings(settings);
    m_portControl.saveSettings(settings);
    m_dataFormatPanel.saveSettings(settings);
    m_stream.saveSettings(settings);
    m_plotControlPanel.saveSettings(settings);
    m_plotMenu.saveSettings(settings);
    m_commandPanel.saveSettings(settings);
    m_recordPanel.saveSettings(settings);
    m_textView.saveSettings(settings);
    m_updateCheckDialog.saveSettings(settings);
}

void MainWindow::loadAllSettings(QSettings* settings)
{

    loadMWSettings(settings);
    m_portControl.loadSettings(settings);
    m_dataFormatPanel.loadSettings(settings);
    m_stream.loadSettings(settings);
    m_plotControlPanel.loadSettings(settings);
    m_plotMenu.loadSettings(settings);
    m_commandPanel.loadSettings(settings);
    m_recordPanel.loadSettings(settings);
    m_textView.loadSettings(settings);
    m_updateCheckDialog.loadSettings(settings);
}

void MainWindow::saveMWSettings(QSettings* settings)
{
    // save window geometry
    settings->beginGroup(SettingGroup_MainWindow);
    settings->setValue(SG_MainWindow_Size, size());
    settings->setValue(SG_MainWindow_Pos, pos());

    //save selected device
    settings->setValue(SG_MainWindow_SelectedDevice,m_deviceSelectComboBox->currentText());
    // save active panel
    // settings->setValue(SG_MainWindow_ActivePanel,
    //                    panelSettingMap.value(ui->tabWidget->currentIndex()));
    settings->setValue(SG_MainWindow_ActivePanel,
                      ui->tabWidget->currentIndex());
    qInfo()<<"Current tab save: "<<ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    // save panel minimization
    settings->setValue(SG_MainWindow_HidePanels,
                       ui->tabWidget->hideAction.isChecked());
    // save window maximized state
    settings->setValue(SG_MainWindow_Maximized,
                       bool(windowState() & Qt::WindowMaximized));
    // save toolbar/dockwidgets state
    settings->setValue(SG_MainWindow_State, saveState());
    settings->endGroup();
}

void MainWindow::loadMWSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_MainWindow);
    // load window geometry
    resize(settings->value(SG_MainWindow_Size, size()).toSize());
    move(settings->value(SG_MainWindow_Pos, pos()).toPoint());

    //set selected device
    QString device=settings->value(SG_MainWindow_SelectedDevice,m_deviceList[0]).toString();
    qInfo()<<"Load selected device"<<device;
    m_deviceSelectComboBox->setCurrentIndex(m_deviceList.indexOf(device));

    // set active panel
    // QString tabSetting =
    //     settings->value(SG_MainWindow_ActivePanel, QString()).toString();
    // ui->tabWidget->setCurrentIndex(
    //     panelSettingMap.key(tabSetting, ui->tabWidget->currentIndex()));
    int tabIndex =
        settings->value(SG_MainWindow_ActivePanel, 0).toInt();
    ui->tabWidget->setCurrentIndex(tabIndex);
    // qInfo()<<"Current tab load: "<<ui->tabWidget->in;
    // hide panels
    ui->tabWidget->hideAction.setChecked(
        settings->value(SG_MainWindow_HidePanels,
                        ui->tabWidget->hideAction.isChecked()).toBool());

    // maximize window
    if (settings->value(SG_MainWindow_Maximized).toBool())
    {
        showMaximized();
    }

    // load toolbar/dockwidgets state
    restoreState(settings->value(SG_MainWindow_State).toByteArray());
    settings->setValue(SG_MainWindow_State, saveState());

    settings->endGroup();
}

void MainWindow::onSaveSettings()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Settings"), QString(), "INI (*.ini)");

    if (!fileName.isNull()) // user canceled
    {
        QSettings settings(fileName, QSettings::IniFormat);
        saveAllSettings(&settings);
    }
}

void MainWindow::onLoadSettings()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Load Settings"), QString(), "INI (*.ini)");

    if (!fileName.isNull()) // user canceled
    {
        QSettings settings(fileName, QSettings::IniFormat);
        loadAllSettings(&settings);
    }
}

void MainWindow::handleCommandLineOptions(const QCoreApplication &app)
{
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
    parser.setApplicationDescription("Small and simple software for plotting data from serial port in realtime.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configOpt({"c", "config"}, "Load configuration from file.", "filename");
    QCommandLineOption portOpt({"p", "port"}, "Set port name.", "port name");
    QCommandLineOption baudrateOpt({"b" ,"baudrate"}, "Set port baud rate.", "baud rate");
    QCommandLineOption openPortOpt({"o", "open"}, "Open serial port.");

    parser.addOption(configOpt);
    parser.addOption(portOpt);
    parser.addOption(baudrateOpt);
    parser.addOption(openPortOpt);

    parser.process(app);

    if (parser.isSet(configOpt))
    {
        QString fileName = parser.value(configOpt);
        QFileInfo fileInfo(fileName);

        if (fileInfo.exists() && fileInfo.isFile())
        {
            QSettings settings(fileName, QSettings::IniFormat);
            loadAllSettings(&settings);
        }
        else
        {
            qCritical() << "Configuration file not exist. Closing application.";
            std::exit(1);
        }
    }

    if (parser.isSet(portOpt))
    {
        m_portControl.selectPort(parser.value(portOpt));
    }

    if (parser.isSet(baudrateOpt))
    {
        m_portControl.selectBaudrate(parser.value(baudrateOpt));
    }

    if (parser.isSet(openPortOpt))
    {
        m_portControl.openPort();
    }
}

