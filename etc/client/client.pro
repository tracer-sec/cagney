#-------------------------------------------------
#
# Project created by QtCreator 2016-10-15T10:28:17
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    connection.cpp \
    botwindow.cpp \
    connectiondialog.cpp

HEADERS  += mainwindow.h \
    connection.h \
    botwindow.h \
    connectiondialog.h

FORMS    += mainwindow.ui \
    botwindow.ui \
    connectiondialog.ui

unix {
    QMAKE_CXXFLAGS += -std=c++11
}
