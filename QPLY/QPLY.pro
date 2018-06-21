#-------------------------------------------------
#
# Project created by QtCreator 2018-06-19T13:26:02
#
#-------------------------------------------------

QT       -= gui

TARGET = QPLY
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

# Shared Project Configuration
include(../config.pri)

SOURCES += \
    src/base.cpp \
    src/header.cpp \
    src/io.cpp \
    src/object.cpp \
    src/unknown.cpp \
    src/ply_impl.cpp \

HEADERS += \
    include/base.h \
    include/header.h \
    include/io.h \
    include/object.h \
    include/unknown.h \
    include/ply_impl.h \
