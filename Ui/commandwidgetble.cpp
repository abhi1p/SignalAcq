#include "commandwidgetble.h"
#include "ui_commandwidgetble.h"

CommandWidgetBLE::CommandWidgetBLE(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CommandWidgetBLE),_sendAction(this)
{
    ui->setupUi(this);

    m_bleCharModel=new BluetoothCharModel(this);
    ui->CharComboBox->setModel(m_bleCharModel);

    connect(ui->pbDelete, &QPushButton::clicked, this, &CommandWidgetBLE::onDeleteClicked);
    connect(ui->pbSend, &QPushButton::clicked, this, &CommandWidgetBLE::onSendClicked);
    connect(ui->pbASCII, &QPushButton::toggled, this, &CommandWidgetBLE::onASCIIToggled);
    connect(ui->leName, &QLineEdit::textChanged, this,[this](QString text)
            {
                this->_sendAction.setText(text);
            });
    connect(&_sendAction, &QAction::triggered, this, &CommandWidgetBLE::onSendClicked);
    connect(ui->CharComboBox,&QComboBox::currentIndexChanged,this,&CommandWidgetBLE::targetCharacteristcsSelected,Qt::QueuedConnection);
}

CommandWidgetBLE::~CommandWidgetBLE()
{
    //disconnect(this,&CommandWidgetBLE::,0);
    delete m_bleCharModel;
    delete ui;

}
void CommandWidgetBLE::targetCharacteristcsSelected(int index)
{
    qInfo()<<"Char seleceted"<<ui->CharComboBox->currentText();
}

void CommandWidgetBLE::bleDeviceConnected(bool connected)
{
    if(!connected)
    {
        // m_srcCharList.clear();
        m_bleCharModel->clearAll();
    }
}

void CommandWidgetBLE::addCharacteristics(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag)
{
    qInfo()<<"Add characteristics in commands";
    if(flag & QLowEnergyCharacteristic::WriteNoResponse)
    {
        m_bleCharModel->addCharacteristic(srvcUuid,charUuid,flag);
    }

}

void CommandWidgetBLE::onDeleteClicked()
{
    // this->disconnect();
    qInfo()<<"Ble command delete called";
    this->deleteLater();
}

void CommandWidgetBLE::onSendClicked()
{
    auto command = ui->leCommand->text();

    if (command.isEmpty())
    {
        qWarning() << "Enter a command to send!";
        emit log(QtWarningMsg,"Enter a command to send!");
        ui->leCommand->setFocus(Qt::OtherFocusReason);
        emit focusRequested();
        return;
    }

    if (isASCIIMode())
    {
        qDebug() << "Sending " << name() << ":" << command;
        emit sendCommandBLE(QBluetoothUuid(ui->CharComboBox->currentText()), ui->leCommand->unEscapedText().toLatin1());
    }
    else // hex mode
    {
        command = command.remove(' ');
        // check if nibbles are missing
        if (command.size() % 2 == 1)
        {
            qWarning() << "HEX command is missing a nibble at the end!";
            ui->leCommand->setFocus(Qt::OtherFocusReason);
            // highlight the byte that is missing a nibble (last byte obviously)
            int textSize = ui->leCommand->text().size();
            ui->leCommand->setSelection(textSize-1, textSize);
            return;
        }
        qDebug() << "Sending HEX:" << command;
        emit sendCommandBLE(QBluetoothUuid(ui->CharComboBox->currentText()), QByteArray::fromHex(command.toLatin1()));
    }
}

void CommandWidgetBLE::onASCIIToggled(bool checked)
{
    ui->leCommand->setMode(checked);
}

bool CommandWidgetBLE::isASCIIMode()
{
    return ui->pbASCII->isChecked();
}

void CommandWidgetBLE::setASCIIMode(bool enabled)
{
    if (enabled)
    {
        ui->pbASCII->setChecked(true);
    }
    else
    {
        ui->pbHEX->setChecked(true);
    }
}

void CommandWidgetBLE::setName(QString name)
{
    ui->leName->setText(name);
}

QString CommandWidgetBLE::name()
{
    return ui->leName->text();
}

void CommandWidgetBLE::setFocusToEdit()
{
    ui->leCommand->setFocus(Qt::OtherFocusReason);
}

QAction* CommandWidgetBLE::sendAction()
{
    return &_sendAction;
}

QString CommandWidgetBLE::commandText()
{
    return ui->leCommand->text();
}

void CommandWidgetBLE::setCommandText(QString str)
{
    ui->leCommand->selectAll();
    ui->leCommand->insert(str);
}

QVariant CommandWidgetBLE::currentCharacteristics()
{
    int index=ui->CharComboBox->currentIndex();
    QModelIndex mIndex=m_bleCharModel->index(index,0);
    QVariant data=m_bleCharModel->data(mIndex,Qt::UserRole);
    // qInfo()<<
    return data;
}

void CommandWidgetBLE::setCurrentCharUuid(QString Uuid)
{
    QBluetoothUuid Uuid2(Uuid);

    if(!Uuid2.isNull())
    {
        qInfo()<<"Uuid is not null";
        m_bleCharModel->addCharacteristic(QBluetoothUuid(),Uuid2,QLowEnergyCharacteristic::WriteNoResponse);
    }
    else
    {
        qInfo()<<"Uuid is null";
    }
}
