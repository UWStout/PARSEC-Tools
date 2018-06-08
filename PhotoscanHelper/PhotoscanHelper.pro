#-------------------------------------------------
#
# Project created by QtCreator 2018-06-06T08:50:13
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PhotoscanHelper
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Where to put all the intermediate files generated during compilation
CONFIG(debug, debug|release) {
    OBJECTS_DIR = Debug/
    MOC_DIR = Debug/moc/
    UI_DIR = Debug/ui/
} else {
    OBJECTS_DIR = Release/
    MOC_DIR = Release/moc/
    UI_DIR = Release/ui/
}

win32 {
    LIBS += -lopengl32 -lglu32
}

# Windows and Mac OS Icons (respectively)
RC_ICONS = resources/icons/PhotoScanHelper.ico
ICON = resources/icons/PhotoScanHelper.icns

# Add this so we don't have to append 'include' in front of every .h file
INCLUDEPATH += include/

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
        src/UILogic/ProcessQueueProgressDialog.cpp

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
