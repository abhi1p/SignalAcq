QT       += core gui serialport network svg bluetooth openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += qwt
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += PROGRAM_NAME=\\\"SignalAcq\\\"

RC_FILE = misc\windows_icon.rc

# Only define in release builds
CONFIG(release, debug|release) {
    DEFINES += QT_NO_INFO_OUTPUT
    DEFINES += QT_NO_DEBUG_OUTPUT
    DEFINES += QT_NO_CRITICAL_OUTPUT
    DEFINES += QT_NO_WARNING_OUTPUT

}

TARGET=SignalAcq
DEFINES += CHAR_MAX_DISPLAY_SIZE=50

DEFINES += VERSION_MAJOR=1 VERSION_MINOR=0 VERSION_PATCH=0 VERSION_STRING=\"\\\"1.0.0\\\"\" VERSION_REVISION=\"\\\"0\\\"\"
SOURCES += \
    BLE/ble.cpp \
    Common/abstractdevice.cpp \
    Common/abstractreader.cpp \
    Common/asciireader.cpp \
    Common/binarystreamreader.cpp \
    Common/channelinfomodel.cpp \
    Common/commandedit.cpp \
    Common/datarecorder.cpp \
    Common/demoreader.cpp \
    Common/framebufferseries.cpp \
    Common/framedreader.cpp \
    Common/indexbuffer.cpp \
    Common/linindexbuffer.cpp \
    Common/numberformat.cpp \
    Common/portlist.cpp \
    Common/readonlybuffer.cpp \
    Common/ringbuffer.cpp \
    Common/samplepack.cpp \
    Common/sink.cpp \
    Common/source.cpp \
    Common/stream.cpp \
    Common/streamchannel.cpp \
    Plot/barchart.cpp \
    Plot/barplot.cpp \
    Plot/barscaledraw.cpp \
    Plot/plot.cpp \
    Plot/plotmanager.cpp \
    Plot/plotmenu.cpp \
    Plot/plotsnapshotoverlay.cpp \
    Plot/samplecounter.cpp \
    Plot/scalepicker.cpp \
    Plot/scalezoomer.cpp \
    Plot/scrollbar.cpp \
    Plot/scrollzoomer.cpp \
    Plot/snapshot.cpp \
    Plot/snapshotmanager.cpp \
    Plot/snapshotview.cpp \
    Plot/tooltipfilter.cpp \
    Plot/waitingspinnerwidget.cpp \
    Plot/zoomer.cpp \
    USB/serialport.cpp \
    Ui/asciireadersettings.cpp \
    Ui/binarystreamreadersettings.cpp \
    Ui/ble_config.cpp \
    Ui/blechar.cpp \
    Ui/bpslabel.cpp \
    Ui/commandpanel.cpp \
    Ui/commandwidget.cpp \
    Ui/commandwidgetble.cpp \
    Ui/dataformatpanel.cpp \
    Ui/datatextview.cpp \
    Ui/demoreadersettings.cpp \
    Ui/endiannessbox.cpp \
    Ui/framedreadersettings.cpp \
    Ui/hidabletabwidget.cpp \
    Ui/ledwidget.cpp \
    Ui/numberformatbox.cpp \
    Ui/plotcontrolpanel.cpp \
    Ui/portcontrol.cpp \
    Ui/recordpanel.cpp \
    Ui/snapshotview.cpp \
    Ui/sneakylineedit.cpp \
    Ui/updatecheckdialog.cpp \
    Ui/updatechecker.cpp \
    main.cpp \
    mainwindow.cpp \
    versionnumber.cpp

HEADERS += \
    BLE/ble.h \
    Common/abstractdevice.h \
    Common/abstractreader.h \
    Common/asciireader.h \
    Common/binarystreamreader.h \
    Common/byteswap.h \
    Common/channelinfomodel.h \
    Common/commandedit.h \
    Common/datarecorder.h \
    Common/defines.h \
    Common/demoreader.h \
    Common/framebuffer.h \
    Common/framebufferseries.h \
    Common/framedreader.h \
    Common/indexbuffer.h \
    Common/linindexbuffer.h \
    Common/numberformat.h \
    Common/portlist.h \
    Common/readonlybuffer.h \
    Common/ringbuffer.h \
    Common/samplepack.h \
    Common/setting_defines.h \
    Common/sink.h \
    Common/source.h \
    Common/stream.h \
    Common/streamchannel.h \
    Plot/barchart.h \
    Plot/barplot.h \
    Plot/barscaledraw.h \
    Plot/plot.h \
    Plot/plotmanager.h \
    Plot/plotmenu.h \
    Plot/plotsnapshotoverlay.h \
    Plot/samplecounter.h \
    Plot/scalepicker.h \
    Plot/scalezoomer.h \
    Plot/scrollbar.h \
    Plot/scrollzoomer.h \
    Plot/snapshot.h \
    Plot/snapshotmanager.h \
    Plot/snapshotview.h \
    Plot/tooltipfilter.h \
    Plot/waitingspinnerwidget.h \
    Plot/zoomer.h \
    USB/serialport.h \
    Ui/asciireadersettings.h \
    Ui/binarystreamreadersettings.h \
    Ui/ble_config.h \
    Ui/blechar.h \
    Ui/bpslabel.h \
    Ui/commandpanel.h \
    Ui/commandwidget.h \
    Ui/commandwidgetble.h \
    Ui/dataformatpanel.h \
    Ui/datatextview.h \
    Ui/demoreadersettings.h \
    Ui/device_model.h \
    Ui/endiannessbox.h \
    Ui/framedreadersettings.h \
    Ui/hidabletabwidget.h \
    Ui/ledwidget.h \
    Ui/numberformatbox.h \
    Ui/plotcontrolpanel.h \
    Ui/portcontrol.h \
    Ui/recordpanel.h \
    Ui/snapshotview.h \
    Ui/sneakylineedit.h \
    Ui/updatecheckdialog.h \
    Ui/updatechecker.h \
    defines.h \
    mainwindow.h \
    setting_defines.h \
    utils.h \
    versionnumber.h


FORMS += \
    Ui/about_dialog.ui \
    Ui/asciireadersettings.ui \
    Ui/binarystreamreadersettings.ui \
    Ui/ble_config.ui \
    Ui/blechar.ui \
    Ui/commandpanel.ui \
    Ui/commandwidget.ui \
    Ui/commandwidgetble.ui \
    Ui/dataformatpanel.ui \
    Ui/datatextview.ui \
    Ui/demoreadersettings.ui \
    Ui/endiannessbox.ui \
    Ui/framedreadersettings.ui \
    Ui/numberformatbox.ui \
    Ui/plotcontrolpanel.ui \
    Ui/portcontrol.ui \
    Ui/recordpanel.ui \
    Ui/snapshotview.ui \
    Ui/updatecheckdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    misc/char.qrc \
    misc/icons.qrc \
    misc/winicons.qrc \
    qml.qrc

DISTFILES += \
    README.md \
    misc/bluetooth_icon.png \
    misc/program_name.desktop.in \
    misc/program_name.png \
    misc/pseudo_device.py \
    misc/pseudo_device_2.py \
    misc/rs232_icon.png \
    misc/serialplot.bmp \
    misc/serialplot.ico \
    misc/serialplot.svg \
    misc/tango/camera.png \
    misc/tango/edit-clear.png \
    misc/tango/index.theme \
    misc/tango/license_notice \
    misc/tango/light_green.png \
    misc/tango/light_red.png \
    misc/tango/list-add.png \
    misc/tango/list-remove.png \
    misc/tango/media-playback-pause.png \
    misc/tango/media-record.png \
    misc/tango/media-record2.png \
    misc/tango/usb_connected.png \
    misc/tango/usb_disconnected.png \
    misc/usb_icon.png \
    misc/windows_icon.rc
