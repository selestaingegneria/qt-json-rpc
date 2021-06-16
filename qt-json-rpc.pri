QT += network
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

HEADERS += $$PWD/error.h \
        $$PWD/httphelper.h \
        $$PWD/peer.h \
        $$PWD/responsehandler.h \
        $$PWD/tcphelper.h

SOURCES += $$PWD/error.cpp \
        $$PWD/httphelper.cpp \
        $$PWD/peer.cpp \
        $$PWD/responsehandler.cpp \
        $$PWD/tcphelper.cpp

include($$PWD/3rdparty/qt-json/qt-json.pri)

INCLUDEPATH += $$PWD \
    $$PWD/3rdparty
