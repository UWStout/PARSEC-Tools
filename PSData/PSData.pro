#-------------------------------------------------
#
# Project created by QtCreator 2018-06-08T13:41:30
#
#-------------------------------------------------

TARGET = psdata
TEMPLATE = lib

DEFINES += PSDATA_LIBRARY

# Shared Project Configuration
include(../config.pri)

# 3rd party libs
macx {
 LIBS += -L$$PWD/../3rdParty/lib/mac-x64/ -lquazip
}

win32 {
 LIBS += -L$$PWD/../3rdParty/lib/win32-x64/ -lquazip
}

INCLUDEPATH += $$PWD/../3rdParty/include
DEPENDPATH += $$PWD/../3rdParty/include

SOURCES += \
    src/PSCameraData.cpp \
    src/PSChunkData.cpp \
    src/PSImageData.cpp \
    src/PSModelData.cpp \
    src/PSProjectDataModel.cpp \
    src/PSProjectFileData.cpp \
    src/PSSensorData.cpp \
    src/PSSessionData.cpp \
    src/PSXMLReader.cpp \
    src/ExposureSettings.cpp \
    src/DirLister.cpp

HEADERS += \
    psdata_global.h \
    include/EnumFactory.h \
    include/PSCameraData.h \
    include/PSChunkData.h \
    include/PSImageData.h \
    include/PSModelData.h \
    include/PSProjectDataModel.h \
    include/PSProjectFileData.h \
    include/PSSensorData.h \
    include/PSSessionData.h \
    include/PSStatusDescribable.h \
    include/PSXMLReader.h \
    include/ExposureSettings.h \
    include/DirLister.h
