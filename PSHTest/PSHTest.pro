#-------------------------------------------------
#
# Project created by QtCreator 2018-06-08T12:32:42
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_pshtest_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# Shared Project Configuration
include(../config.pri)

# Link in 3rd party libs
macx:LIBS += -ltinyply -lzip

win32:CONFIG(debug, debug|release): LIBS += -ltinyplyd -lzlibd -llzmad -lbzip2d -lZipLibd
else:win32:CONFIG(debug, debug|release): LIBS += -ltinyply -lzlib -llzma -lbzip2 -lZipLib

SOURCES += \
    tst_pshtest_test.cpp

RESOURCES += \
    testresources.qrc

# Add in the PSData library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../PSData/release/ -lpsdata
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../PSData/debug/ -lpsdata
else:unix: LIBS += -L$$OUT_PWD/../PSData/ -lpsdata

INCLUDEPATH += $$PWD/../PSData/ $$PWD/../PSData/include
DEPENDPATH += $$PWD/../PSData/include
