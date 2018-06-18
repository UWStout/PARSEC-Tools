DEFINES+=LIBRAW_NODLL
win32:CONFIG+=console

CONFIG(debug,debug|release) {
    win32:LIBS+=librawd.lib libjpeg.lib
    win32:QMAKE_CXXFLAGS += /MDd
} else {
    win32:LIBS+=libraw.lib libjpeg.lib
    win32:QMAKE_CXXFLAGS += /MD
}

macx:CONFIG-=app_bundle
macx:LIBS+=-lraw -ljpeg

win32-g++: {
#    LIBS += -lws2_32
}

include (libraw-common-lib.pro)
