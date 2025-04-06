#ifndef COMMANDWIDGETBLE_H
#define COMMANDWIDGETBLE_H

#include "Ui/device_model.h"
#include <QWidget>

namespace Ui {
class CommandWidgetBLE;
}

class CommandWidgetBLE : public QWidget
{
    Q_OBJECT

public:
    explicit CommandWidgetBLE(QWidget *parent = nullptr);
    ~CommandWidgetBLE();

    void setName(QString name);
    QString name();
    void setFocusToEdit();
    /// An action that triggers sending of command.
    QAction* sendAction();
    /// true: ascii mode, false hex mode
    bool isASCIIMode();
    /// true: ascii mode, false hex mode
    void setASCIIMode(bool ascii);
        /// Returns the command data as text
    QString commandText();
    /// Set command data as text. Text is validated according to current mode.
    void setCommandText(QString str);

    ///return current selected Uuid
    QVariant currentCharacteristics();

    ///return current characteristics


    ///set current selected Uuid
    void setCurrentCharUuid(QString Uuid);

signals:
    void deleteRequested(CommandWidgetBLE* thisWidget);
    void focusRequested();
    void sendCommandBLE(QBluetoothUuid charUuid, QByteArray command);
    void writeCharacteristicsSignal(QBluetoothUuid charUuid, QString text);
    void log(QtMsgType type, QString text);


public slots:
    void bleDeviceConnected(bool connected);
    void addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag);


private slots:
    void onDeleteClicked();
    void onSendClicked();
    void onASCIIToggled(bool checked);
    void targetCharacteristcsSelected(int index);


private:
    Ui::CommandWidgetBLE *ui;
    QAction _sendAction;

    BluetoothCharModel *m_bleCharModel;
};

#endif // COMMANDWIDGETBLE_H
