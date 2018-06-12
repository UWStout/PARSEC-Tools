#-------------------------------------------------
#
# Project created by QtCreator 2018-06-06T08:50:13
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PhotoscanHelper
TEMPLATE = app

# Shared Project Configuration
include(../config.pri)

win32 {
    LIBS += -lopengl32 -lglu32
}

# Windows and Mac OS Icons (respectively)
RC_ICONS = resources/icons/PhotoScanHelper.ico
ICON = resources/icons/PhotoScanHelper.icns

SOURCES += \
        src/PhotoScanHelperApp.cpp \
        src/UILogic/CollectionSelectionDialog.cpp \
        src/Model/PSCameraData.cpp \
        src/Model/PSImageData.cpp \
        src/Model/PSSensorData.cpp \
        src/Model/PSXMLReader.cpp \
        src/Model/PSProjectFileData.cpp \
        src/Model/PSChunkData.cpp \
        src/Model/PSModelData.cpp \
#        src/UILogic/PSHelperMainWindow.cpp \
        src/UILogic/CancelableModalProgressDialog.cpp \
        src/UILogic/ProcessQueueProgressDialog.cpp \
        src/UILogic/ProgramPreferencesDialog.cpp

HEADERS += \
        include/EnumFactory.h \
        include/CollectionSelectionDialog.h \
        include/PSXMLReader.h \
        include/PSStatusDescribable.h \
        include/PSProjectFileData.h \
        include/PSChunkData.h \
        include/PSCameraData.h \
        include/PSImageData.h \
        include/PSSensorData.h \
        include/PSModelData.h \
#        include/PSHelperMainWindow.h \
        include/CancelableModalProgressDialog.h \
        include/ProcessQueueProgressDialog.h \
        include/QueueableProcess.h \
        include/ProgramPreferencesDialog.h

FORMS += \
        forms/AboutDialog.ui \
        forms/CancelableProgressDialog.ui \
        forms/CollectionSelectionDialog.ui \
        forms/GeneralSettingsDialog.ui \
        forms/GLModelViewer.ui \
        forms/ProgramPreferencesDialog.ui \
        forms/PSChunkInfo.ui \
        forms/PSHelperMain.ui \
        forms/PSProjectInfo.ui \
        forms/RawImageExposureDialog.ui \
        forms/StageProgressDialog.ui

RESOURCES += \
    PSHelper.qrc

DISTFILES +=
