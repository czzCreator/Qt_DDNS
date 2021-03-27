QT -= gui

QT += network

CONFIG += c++11 c++14 console

#使用QtCreator编译程序的输出目录都会附带debug或release目录，其实可以关闭这些目录的输出。只需在pro文件中添加以下行：
CONFIG -= debug_and_release
CONFIG -= app_bundle


windows{
    CONFIG(debug, debug|release){
        DESTDIR = ./bin_debug/
        TARGET = Qt_DDNS_d
    }
    CONFIG(release, debug|release){
        DESTDIR = ./bin_release/
        TARGET = Qt_DDNS
    }
}


# http client server 的 C++11 C++14 的实现
include(3rd_pri/qhttp/qhttp.pri)


## ================================================================== ##
## ================================================================== ##


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        qappglobalref.cpp \
        qddns_service.cpp \
        qhttprpc_outeraddrrequester.cpp \
        qlog_message_handler.cpp




MOC_DIR += ./tmp/moc_dir
OBJECTS_DIR += ./tmp/object_dir
UI_DIR += ./tmp/ui_dir


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    qappglobalref.h \
    qddns_service.h \
    qhttprpc_outeraddrrequester.h \
    qlog_message_handler.h
