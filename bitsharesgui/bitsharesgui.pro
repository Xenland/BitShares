#-------------------------------------------------
#
# Project created by QtCreator 2013-08-14T18:36:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bitsharesgui
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11
LIBS += ../libbshare.a ../fc/libfc.a -L${BOOST_LIBRARY_DIR} -lboost_context -lboost_thread -lboost_system -lboost_chrono -lboost_filesystem -lboost_system -lboost_date_time -lboost_coroutine

INCLUDEPATH+=../include
INCLUDEPATH+=../fc/include


SOURCES += main.cpp \
    bitsharesmainwindow.cpp \
    profileeditor.cpp \
    bitsharestreemodel.cpp \
    bshare_gui.cpp

HEADERS  += \
    bitsharesmainwindow.h \
    profileeditor.h \
    bitsharestreemodel.h \
    bshare_gui.h

FORMS    += \
    bitsharesmainwindow.ui \
    profileeditor.ui \
    bshare_gui.ui
