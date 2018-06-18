#-------------------------------------------------
#
# Project created by QtCreator 2018-06-06T08:50:13
#
#-------------------------------------------------

QT += core gui concurrent

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
        src/app/PSandPhotoScanner.cpp \
        src/app/PLYMeshData.cpp \
        src/UILogic/CollectionSelectionDialog.cpp \
        src/UILogic/PSHelperMainWindow.cpp \
        src/UILogic/PSProjectInfoDialog.cpp \
#        src/UILogic/CancelableModalProgressDialog.cpp \
#        src/UILogic/ProcessQueueProgressDialog.cpp \
#        src/UILogic/ProgramPreferencesDialog.cpp \
#        src/UILogic/GeneralSettingsDialog.cpp \
#        src/UILogic/PSProjectInfoDialog.cpp

HEADERS += \
        include/PSandPhotoScanner.h \
        include/PLYMeshData.h \
        include/CollectionSelectionDialog.h \
        include/PSHelperMainWindow.h \
        include/PSProjectInfoDialog.h \
#        include/CancelableModalProgressDialog.h \
#        include/ProcessQueueProgressDialog.h \
#        include/QueueableProcess.h \
#        include/ProgramPreferencesDialog.h \
#        include/ImageProcessorIM4J.h \
#        include/GeneralSettingsDialog.h \
#        include/PSProjectInfoDialog.h \

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

# Add in the PSData library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../PSData/release/ -lpsdata
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../PSData/debug/ -lpsdata
else:unix: LIBS += -L$$OUT_PWD/../PSData/ -lpsdata

INCLUDEPATH += $$PWD/../PSData/ $$PWD/../PSData/include
DEPENDPATH += $$PWD/../PSData/include
