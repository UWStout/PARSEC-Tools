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

# Link in 3rd party libs
macx:LIBS += -lquazip
win32:CONFIG(debug, debug|release): LIBS += -lquazipd -lopengl32 -lglu32
else:win32:CONFIG(debug, debug|release): LIBS += -lquazip -lopengl32 -lglu32

# Link in exiv2 on windows
win32:DEFINES += USE_EXIV2 EXV_USING_CPP_ELEVEN
win32:CONFIG(debug, debug|release): LIBS += -lexiv2d -lxmpsdkd -llibexpatd -lzlibd
else:win32:CONFIG(debug, debug|release): LIBS += -lexiv2 -lxmpsdk -llibexpat -lzlib

# Uncomment to enable libraw
#macx:LIBS += -lraw -ljpeg
#win32:CONFIG(debug, debug|release): LIBS += -llibrawd -llibjpegd
#else:win32:CONFIG(debug, debug|release): LIBS += -llibraw -llibjpeg
#win32:DEFINES += USE_LIB_RAW LIBRAW_NODLL

# Windows and Mac OS Icons (respectively)
RC_ICONS = resources/icons/PhotoScanHelper.ico
ICON = resources/icons/PhotoScanHelper.icns

SOURCES += \
    src/PhotoScanHelperApp.cpp \
    src/app/PSandPhotoScanner.cpp \
    src/app/ImageProcessor.cpp \
    src/app/RawImageExposer.cpp \
    src/UILogic/InteractivePhotoScanDialog.cpp \
    src/UILogic/QtConsole.cpp \
    src/UILogic/CollectionSelectionDialog.cpp \
    src/UILogic/PSHelperMainWindow.cpp \
    src/UILogic/PSProjectInfoDialog.cpp \
    src/UILogic/CancelableModalProgressDialog.cpp \
#    src/UILogic/ProcessQueueProgressDialog.cpp \
#    src/UILogic/ProgramPreferencesDialog.cpp \
    src/UILogic/GeneralSettingsDialog.cpp \
#    src/UILogic/PSProjectInfoDialog.cpp
    src/UILogic/RawImageExposureDialog.cpp \
    src/UILogic/QuickPreviewDialog.cpp \
    src/UILogic/ScriptedPhotoScanDialog.cpp \
    src/UILogic/GLModelWidget.cpp \
    src/UILogic/QtModelViewerWidget.cpp \
    src/UILogic/QtTrackball.cpp \
    src/UILogic/InitializeSessionDialog.cpp \
    src/UILogic/CreateNewSessionDialog.cpp \
    src/UILogic/CaptureSessionDialog.cpp \
    src/UILogic/PhotoScanPhase1Dialog.cpp \
    src/UILogic/ScriptedPhase1Dialog.cpp \
    src/UILogic/PhotoScanPhase2Dialog.cpp

HEADERS += \
    include/PSandPhotoScanner.h \
    include/ImageProcessor.h \
    include/RawImageExposer.h \
    include/CollectionSelectionDialog.h \
    include/PSHelperMainWindow.h \
    include/PSProjectInfoDialog.h \
    include/CancelableModalProgressDialog.h \
#    include/ProcessQueueProgressDialog.h \
    include/QueueableProcess.h \
#    include/ProgramPreferencesDialog.h \
#    include/ImageProcessorIM4J.h \
    include/GeneralSettingsDialog.h \
#    include/PSProjectInfoDialog.h \
    include/RawImageExposureDialog.h \
    include/InteractivePhotoScanDialog.h \
    include/QtConsole.h \
    include/QuickPreviewDialog.h \
    include/ScriptedPhotoScanDialog.h \
    include/GLModelWidget.h \
    include/QtModelViewerWidget.h \
    include/QtTrackball.h \
    include/InitializeSessionDialog.h \
    include/CreateNewSessionDialog.h \
    include/CaptureSessionDialog.h \
    include/PhotoScanPhase1Dialog.h \
    include/ScriptedPhase1Dialog.h \
    include/PhotoScanPhase2Dialog.h

FORMS += \
    forms/AboutDialog.ui \
    forms/CancelableProgressDialog.ui \
    forms/CollectionSelectionDialog.ui \
    forms/GeneralSettingsDialog.ui \
    forms/GLModelWidget.ui \
    forms/ProgramPreferencesDialog.ui \
    forms/PSChunkInfo.ui \
    forms/PSHelperMain.ui \
    forms/PSProjectInfo.ui \
    forms/RawImageExposureDialog.ui \
    forms/StageProgressDialog.ui \
    forms/InteractivePhotoScanDialog.ui \
    forms/QuickPreviewDialog.ui \
    forms/ScriptedPhotoScanDialog.ui \
    forms/InitializeSessionDialog.ui \
    forms/CreateNewSessionDialog.ui \
    forms/CaptureSessionDialog.ui \
    forms/PhotoScanPhase1Dialog.ui \
    forms/ScriptedPhase1Dialog.ui \
    forms/PhotoScanPhase2Dialog.ui

RESOURCES += \
    PSHelper.qrc

# Add in the PSData library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../PSData/release/ -lpsdata
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../PSData/debug/ -lpsdata
else:unix: LIBS += -L$$OUT_PWD/../PSData/ -lpsdata

INCLUDEPATH += $$PWD/../PSData/ $$PWD/../PSData/include
DEPENDPATH += $$PWD/../PSData/include

# Add in the QPLY library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QPLY/release/ -lQPLY
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QPLY/debug/ -lQPLY
else:macx: LIBS += -L$$OUT_PWD/../QPLY/ -lQPLY

INCLUDEPATH += $$PWD/../QPLY/include
DEPENDPATH += $$PWD/../QPLY/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/release/libQPLY.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/debug/libQPLY.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/release/QPLY.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/debug/QPLY.lib
else:macx: PRE_TARGETDEPS += $$OUT_PWD/../QPLY/libQPLY.a
