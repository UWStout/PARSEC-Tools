#-------------------------------------------------
#
# Project created by QtCreator 2018-06-08T13:41:30
#
#-------------------------------------------------

TARGET = psdata
TEMPLATE = lib
QT += core gui

DEFINES += PSDATA_LIBRARY

# Shared Project Configuration
include(../config.pri)

# Link in quazip
LIBS += -lquazip

SOURCES += \
    src/PLYMeshData.cpp \
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
    src/DirLister.cpp \
    src/PSStatusDescribable.cpp

HEADERS += \
    psdata_global.h \
    include/EnumFactory.h \
    include/PLYMeshData.h \
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

DISTFILES +=

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
