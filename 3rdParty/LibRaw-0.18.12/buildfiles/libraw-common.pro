win32:CONFIG+=console
win32:LIBS+=libraw.lib

macx:CONFIG-=app_bundle
macx:LIBS+=-lraw -ljpeg

win32-g++: {
#    LIBS += -lws2_32
}

include (libraw-common-lib.pro)
