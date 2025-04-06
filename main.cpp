#include "mainwindow.h"
#include "BLE/ble.h"
#include <QApplication>
// #include <iostream>
#include "USB/serialport.h"


// MainWindow* pMainWindow = nullptr;

// const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER=qInstallMessageHandler(nullptr);
// void messageHandler(QtMsgType type, const QMessageLogContext &context,
//                     const QString &msg)
// {
//     QString logString;

//     switch (type)
//     {
// #if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
//     case QtInfoMsg:
//         logString = "[Info] " + msg;
//         break;
// #endif
//     case QtDebugMsg:
//         logString = "[Debug] " + msg;
//         break;
//     case QtWarningMsg:
//         logString = "[Warning] " + msg;
//         break;
//     case QtCriticalMsg:
//         logString = "[Error] " + msg;
//         break;
//     case QtFatalMsg:
//         logString = "[Fatal] " + msg;
//         break;
//     }

//     std::cerr <<logString.toStdString() << "\tFile: "<<context.file<<" Line: "<<context.line<< std::endl;

//     // if (pMainWindow != nullptr)
//     // {
//     //     // TODO: don't call MainWindow::messageHandler if window is destroyed
//     //     pMainWindow->messageHandler(type, logString, msg);
//     // }

//     if (type == QtFatalMsg)
//     {
//         // __builtin_trap();
// // Use different methods based on the compiler
// #if defined(__GNUC__) || defined(__clang__)
//         __builtin_trap(); // GCC/Clang specific
// #elif defined(_MSC_VER)
//         __debugbreak(); // MSVC specific
// #else
//         abort(); // Fallback for other compilers
// #endif
//     }
//     // (*QT_DEFAULT_MESSAGE_HANDLER)(type,context,msg);
// }

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName(PROGRAM_NAME);
    a.setApplicationDisplayName(PROGRAM_NAME);
    a.setWindowIcon(QIcon(":/icons/tango/tango/testIcon4.ico"));
    //qInfo()<<argc<<" "<<argv;
    qInfo()<<a.applicationDirPath();
    MainWindow w;
    // pMainWindow = &w;
    // qInstallMessageHandler(messageHandler);
    // BLE ble;
    // QSharedPointer<BLE> ble(new BLE,&QObject::deleteLater);
    BLE ble;
    SerialPort serial;
    qInfo()<<"Running in thread: "<<ble.thread();

    QSharedPointer<QThread> worker1(new QThread,&QObject::deleteLater);
    worker1->setObjectName("Data acq worker");
    QObject::connect(&a,&QApplication::aboutToQuit,worker1.data(),&QThread::quit,Qt::QueuedConnection);
    // QObject::connect(worker.data(),&QThread::finished,worker.data(),&QThread::quit,Qt::QueuedConnection);
    w.connectBLESignalsAndSlots(&ble);
    ble.moveToThread(worker1.data());
    QObject::connect(worker1.data(), &QThread::started, &serial, &SerialPort::initSerialPort);
    w.connectSerialPortSignalsAndSlots(&serial);
    serial.moveToThread(worker1.data());

    worker1->start();

    w.show();
    return a.exec();
}
