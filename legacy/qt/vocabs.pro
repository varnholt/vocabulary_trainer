QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

OBJECTS_DIR = .obj

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/test.cpp \
    src/word.cpp

HEADERS += \
    src/mainwindow.h \
    src/test.h \
    src/word.h

FORMS += \
    src/mainwindow.ui

