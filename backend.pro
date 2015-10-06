TEMPLATE = lib
CONFIG += staticlib

TARGET = phoenix-backend

QT += qml quick widgets multimedia concurrent

# No need to worry about naming collision, folders are just for organization
INCLUDEPATH += core input consumer

HEADERS += \
    videoitem.h \
    logging.h \
    consumer/audiobuffer.h \
    consumer/audiooutput.h \
    consumer/videooutput.h \
    core/libretro.h \
    core/libretrocore.h \
    input/keyboard.h \
    input/inputmanager.h \
    input/inputdevice.h \
    input/inputdeviceevent.h \
    input/joystick.h \
    input/sdleventloop.h \
    input/qmlinputdevice.h \
    core/gamesession.h \
    core/gamemanager.h \
    core/common.h

SOURCES += \
    videoitem.cpp \
    logging.cpp \
    consumer/audiobuffer.cpp \
    consumer/audiooutput.cpp \
    consumer/videooutput.cpp \
    core/libretrocore.cpp \
    input/keyboard.cpp \
    input/inputmanager.cpp \
    input/joystick.cpp \
    input/sdleventloop.cpp \
    input/inputdevice.cpp \
    input/inputdeviceevent.cpp \
    input/qmlinputdevice.cpp \
    core/gamemanager.cpp \
    core/gamesession.cpp


# Externals and system libraries
INCLUDEPATH += ../externals/quazip/quazip
LIBS += -L../externals/quazip/quazip -lquazip
LIBS += -lsamplerate

macx { QMAKE_MAC_SDK = macosx10.11 }

win32 {
    CONFIG -= windows
    QMAKE_LFLAGS += $$QMAKE_LFLAGS_WINDOWS

    LIBS += -LC:/SDL2/lib
    LIBS += -lmingw32 -lSDL2main -lSDL2 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid

    DEFINES += SDL_WIN
    INCLUDEPATH += C:/SDL2/include C:/msys64/mingw64/include/SDL2 C:/msys64/mingw32/include/SDL2


    CONFIG(debug, debug|release)  {
        depends.path = $$OUT_PWD/debug
        depends.files += C:/SDL2/bin/SDL2.dll
    }

    CONFIG(release, debug|release) {
        depends.path = $$OUT_PWD/release
        depends.files += C:/SDL2/bin/SDL2.dll
    }
    INSTALLS += depends
}

else {
    LIBS += -lSDL2
    INCLUDEPATH += /usr/local/include /usr/local/include/SDL2 # Homebrew (OS X)
    INCLUDEPATH += /opt/local/include /opt/local/include/SDL2 # MacPorts (OS X)
    INCLUDEPATH += /usr/include /usr/include/SDL2 # Linux
    QMAKE_CXXFLAGS +=
    QMAKE_LFLAGS += -L/usr/local/lib -L/opt/local/lib
}


RESOURCES += input/controllerdb.qrc

CONFIG += c++11

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Check if the config file exists
!exists( ../common.pri) {
    warning( "Didn't find common.pri. Defaulting to deployment.pri, it's still good!" )
    include( deployment.pri )
} else {
    include( ../common.pri )
}


