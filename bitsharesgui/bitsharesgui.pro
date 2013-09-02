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
    bshare_gui.cpp \
    bshare_viewport.cpp \
    bshare_viewports/bshare_viewport_mail.cpp

HEADERS  += \
    bshare_gui.h \
    bshare_viewport.hpp \
    bshare_viewports/bshare_viewport_mail.hpp

FORMS    += \
    bshare_gui.ui
