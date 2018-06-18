win32 {
    !contains(QMAKE_HOST.arch, x86_64) {
	SUFF="x86"
    } else {
        ## Windows x64 (64bit) specific build here
	SUFF="x86_64"
    }
}

CONFIG(debug,debug|release) {
        win32:OUTD=/Users/kingd0559/Documents/GitHub/PhotoscanHelper/3rdParty/build-LibRawDebug/buildfiles/debug-$$SUFF
        macx:OUTD=/Users/berriers/Programming/Qt/ParsecHelper/3rdParty/build-LibRaw-Debug/buildfiles/debug
} else {
        win32:OUTD=/Users/kingd0559/Documents/GitHub/PhotoscanHelper/3rdParty/build-LibRawDebug/buildfiles/release-$$SUFF
        macx:OUTD=/Users/berriers/Programming/Qt/ParsecHelper/3rdParty/build-LibRaw-Debug/buildfiles/release
}

macx:INCLUDEPATH+=/opt/local/include
macx:LIBS+=-L/opt/local/lib
win32:INCLUDEPATH+=../../include
win32:LIBS+=-L../../lib/win32-x64

INCLUDEPATH+=../

OBJECTS_DIR = $$OUTD/
MOC_DIR = $$OUTD/
RCC_DIR = $$OUTD/
UI_DIR = $$OUTD/
DESTDIR = $$OUTD/
LIBS+=-L$$OUTD
CONFIG+=warn_off
