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

# Add this so we don't have to append 'include' in front of every .h file
INCLUDEPATH += include/

# Enable C++17 support
win32 {
    QMAKE_CXXFLAGS += /std:c++17
} else {
    QMAKE_CXXFLAGS += -std=c++17
}
