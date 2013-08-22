#-------------------------------------------------
#
# Project created by QtCreator 2013-08-14T18:36:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bitsharesgui
TEMPLATE = app


SOURCES += main.cpp \
    bitsharesmainwindow.cpp \
    profileeditor.cpp \
    bitsharestreemodel.cpp

HEADERS  += \
    bitsharesmainwindow.h \
    profileeditor.h \
    bitsharestreemodel.h

FORMS    += \
    bitsharesmainwindow.ui \
    profileeditor.ui
