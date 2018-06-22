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
macx:LIBS += -lQuaZip

win32:CONFIG(debug, debug|release): LIBS += -lQuaZipd
else:win32:CONFIG(debug, debug|release): LIBS += -lQuaZip

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

# Add in the QPLY library
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/release/libQPLY.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/debug/libQPLY.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/release/QPLY.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QPLY/debug/QPLY.lib
else:macx: PRE_TARGETDEPS += $$OUT_PWD/../QPLY/libQPLY.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QPLY/release/ -lQPLY
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QPLY/debug/ -lQPLY
else:macx: LIBS += -L$$OUT_PWD/../QPLY/ -lQPLY

INCLUDEPATH += $$PWD/../QPLY/include
DEPENDPATH += $$PWD/../QPLY/include
