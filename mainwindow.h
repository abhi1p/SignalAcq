#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Plot/plotmenu.h"
#include "Plot/plotmanager.h"
#include "BLE/ble.h"
#include "Plot/samplecounter.h"
#include "Ui/ble_config.h"
#include "Ui/bpslabel.h"
#include "Ui/commandpanel.h"
#include "Ui/dataformatpanel.h"
#include "Ui/datatextview.h"
#include "Ui/plotcontrolpanel.h"
#include "Ui/portcontrol.h"
#include "Ui/recordpanel.h"
#include "Ui/updatecheckdialog.h"
#include "Plot/snapshotmanager.h"

#include "filter_base.h"
#include "ui_about_dialog.h"
#include "USB/serialport.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    PlotViewSettings viewSettings() const;

    void messageHandler(QtMsgType type, const QString &logString, const QString &msg);
    void connectBLESignalsAndSlots(BLE *ble);
    void connectSerialPortSignalsAndSlots(SerialPort *serial);
private:
    Ui::MainWindow *ui;

    QDialog m_aboutDialog;
    void setupAboutDialog();

    //QSerialPort m_serialPortDevice;
    //BLE m_bleDevice;
    AbstractDevice m_device;
    PortControl m_portControl;

    BLE_Config *m_bleConfig;
    Filter_base *m_filterBase;
    QScopedPointer<QList<bleChar*>> m_charUiList;

    unsigned int m_numOfSamples;

    QList<QwtPlotCurve*> curves;
    // ChannelManager channelMan;
    Stream m_stream;
    PlotManager* m_plotMan;
    QWidget* m_secondaryPlot;
    SnapshotManager m_snapshotMan;
    SampleCounter m_sampleCounter;

    QLabel m_spsLabel;
    CommandPanel m_commandPanel;
    DataFormatPanel m_dataFormatPanel;
    RecordPanel m_recordPanel;
    PlotControlPanel m_plotControlPanel;
    PlotMenu m_plotMenu;
    DataTextView m_textView;
    UpdateCheckDialog m_updateCheckDialog;
    BPSLabel m_bpsLabel;

    QToolBar m_deviceSelectToolbar;
    QComboBox *m_deviceSelectComboBox;
    QLabel *m_conDconLabel;
    // QStringList m_deviceList={"USB Serial","BLE","Biopac"};
    QStringList m_deviceList={"USB Serial","BLE"};
    QHash<QString, AbstractDevice::INPUT_DEVICE> m_deviceListMap;

    void handleCommandLineOptions(const QCoreApplication &app);

    /// Returns true if demo is running
    bool isDemoRunning();
    /// Display a secondary plot in the splitter, removing and
    /// deleting previous one if it exists
    void showSecondary(QWidget* wid);
    /// Hide secondary plot
    void hideSecondary();
    /// Stores settings for all modules
    void saveAllSettings(QSettings* settings);
    /// Load settings for all modules
    void loadAllSettings(QSettings* settings);
    /// Stores main window settings into a `QSettings`
    void saveMWSettings(QSettings* settings);
    /// Loads main window settings from a `QSettings`
    void loadMWSettings(QSettings* settings);

    /// `QWidget::closeEvent` handler
    void closeEvent(QCloseEvent * event);

public slots:
    void log(QtMsgType type, QString text);

private slots:
    void onPortToggled(bool open);
    void onSourceChanged(Source* source);
    void onNumOfSamplesChanged(int value);

    void clearPlot();
    void onSpsChanged(float sps);
    void enableDemo(bool enabled);
    void showBarPlot(bool show);

    void onExportCsv();
    void onExportSvg();
    void onSaveSettings();
    void onLoadSettings();

    void addWidgetToTab(QTabWidget *tabWidget, QWidget *widget, QString text);
    void showTab(QTabWidget *tabWidget, QWidget *widget);
    void addDeviceSelect_UI();
    void deviceSelectIndexChanged(int index);
    void addSourceTab(AbstractDevice::INPUT_DEVICE device);
    void selectCurrentActiveDevice(QString device);

    void deviceConnected(bool connected);
    void serialPortOpened(bool opened);

signals:
    void updateInputDeviceSignal(AbstractDevice::INPUT_DEVICE device);
    void updateMenuCommands();

};
#endif // MAINWINDOW_H
