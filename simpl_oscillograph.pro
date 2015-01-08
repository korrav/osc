#-------------------------------------------------
#
# Project created by QtCreator 2014-07-20T09:07:02
#
#-------------------------------------------------

QT       += core gui
QT       += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = osc
TEMPLATE = app

CONFIG +=console
CONFIG += c++11
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += main.cpp \
    clipboard.cpp \
    oscilloscope.cpp \
    debug.cpp \
    controltrigger.cpp \
    writetask.cpp \
    manipulatordata.cpp \
    screen.cpp \
    dialogues.cpp

HEADERS  += oscilloscope.h \
    clipboard.h \
    receiver.h \
    unit.h \
    debug.h \
    controltrigger.h \
    writetask.h \
    manipulatordata.h \
    screen.h \
    dialogues.h

RESOURCES += \
    resource/resource.qrc
