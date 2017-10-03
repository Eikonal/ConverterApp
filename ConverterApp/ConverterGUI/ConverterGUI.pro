TEMPLATE = app
TARGET = ConverterGUI

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
LIBS += -ldl

SOURCES += MainWindow.cpp main.cpp
HEADERS += MainWindow.h
FORMS   += MainWindow.ui
