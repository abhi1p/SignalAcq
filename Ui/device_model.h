#ifndef DEVICE_MODEL_H
#define DEVICE_MODEL_H

#include <QStandardItemModel>
#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QSerialPortInfo>
#include <QLowEnergyCharacteristic>

class BluetoothDeviceModel : public QStandardItemModel
{
public:
    explicit BluetoothDeviceModel(QObject *parent = nullptr) : QStandardItemModel(parent)
    {

    }
    ~BluetoothDeviceModel()
    {

    }

    void addDevice(const QBluetoothDeviceInfo &deviceInfo)
    {
        QStandardItem *item = new QStandardItem(deviceInfo.name());
        item->setData(QVariant::fromValue(deviceInfo), Qt::UserRole); // Store QBluetoothDeviceInfo as user data
        appendRow(item);
    }

    QBluetoothDeviceInfo getDeviceInfo(const QModelIndex &index) const
    {
        QVariant data = itemData(index).value(Qt::UserRole);
        return data.value<QBluetoothDeviceInfo>();
    }
    // int rowCount(const QModelIndex &parent = QModelIndex()) const override
    // {

    // }
    // QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    // {

    // }
};

class BluetoothCharModel : public QAbstractListModel
{

public:
    explicit BluetoothCharModel(QObject *parent = nullptr): QAbstractListModel(parent)
    {
        // addCharacteristic(QBluetoothUuid(),QBluetoothUuid(),QLowEnergyCharacteristic::Unknown);
        qRegisterMetaType<BluetoothCharModel::CharacteristicDetails>("BluetoothCharModel::CharacteristicDetails");
    }
    struct CharacteristicDetails {
        QBluetoothUuid serviceUuid;
        QBluetoothUuid characteristicUuid;
        QLowEnergyCharacteristic::PropertyTypes properties;
    };

    // Override necessary methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return m_characteristics.count(); // Return the number of items in the list
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || index.row() >= m_characteristics.size()) {
            return QVariant();
        }

        const CharacteristicDetails &details = m_characteristics.at(index.row());

        if (role == Qt::DisplayRole) {
            // Only return the characteristic UUID as string for display purposes
            return details.characteristicUuid.toString();
        } else if (role == Qt::UserRole) {
            // You can store additional data under Qt::UserRole for internal use
            return QVariant::fromValue(details);
        }

        return QVariant();
    }

    void addCharacteristic(QBluetoothUuid srvcUuid, QBluetoothUuid charUuid, QLowEnergyCharacteristic::PropertyTypes flag)
    {
        // Check for duplicate entries
        for (CharacteristicDetails &details : m_characteristics)
        {
            if (details.serviceUuid == srvcUuid && details.characteristicUuid == charUuid) {
                // Duplicate found, do not add
                return;
            }
        }
        beginInsertRows(QModelIndex(), m_characteristics.size(), m_characteristics.size());
        CharacteristicDetails details = {srvcUuid, charUuid, flag};
        m_characteristics.append(details);
        endInsertRows();
    }

    void clearAll()
    {
        if(m_characteristics.size()>0)
        {
            beginResetModel();      // Notify views that the model is about to change
            m_characteristics.remove(1,m_characteristics.size()-1);  // Clear all data from the list
            endResetModel();        // Notify views that the model has been reset
        }

    }
private:
    QList<CharacteristicDetails> m_characteristics;
    // quint32 m_nulluuid=0;
};
// Declare the struct as a Qt metatype
Q_DECLARE_METATYPE(BluetoothCharModel::CharacteristicDetails)


class SerialPortModel : public QStandardItemModel
{
public:
    explicit SerialPortModel(QObject *parent = nullptr) : QStandardItemModel(parent)
    {

    }
    ~SerialPortModel()
    {

    }

    void addSerialPort(const QSerialPortInfo &serialPortInfo)
    {
        QString portName=serialPortInfo.portName()+"-"+serialPortInfo.description();
        QStandardItem *item = new QStandardItem(portName);
        item->setData(QVariant::fromValue(serialPortInfo), Qt::UserRole); // Store QBluetoothDeviceInfo as user data
        appendRow(item);
    }

    QSerialPortInfo getSerialPortInfo(const QModelIndex &index) const
    {
        QVariant data = itemData(index).value(Qt::UserRole);
        return data.value<QSerialPortInfo>();
    }
    // QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    // {
    //     if (!index.isValid() /*|| index.row() >= this->rowCount()*/) {
    //         return QVariant();
    //     }

    //     QVariant data = itemData(index).value(Qt::UserRole);
    //     return data.value<QSerialPortInfo>();

    //     if (role == Qt::DisplayRole) {
    //         // Only return the characteristic UUID as string for display purposes
    //         return details.characteristicUuid.toString();
    //     } else if (role == Qt::UserRole) {
    //         // You can store additional data under Qt::UserRole for internal use
    //         return QVariant::fromValue(details);
    //     }

    //     return QVariant();
    // }
};



#endif // DEVICE_MODEL_H
