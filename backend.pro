TEMPLATE = lib
CONFIG += staticlib

TARGET = phoenix-backend

QT += qml quick widgets multimedia

HEADERS += \
    videoitem.h \
    core.h \
    libretro.h \
    logging.h \
    audiobuffer.h \
    audiooutput.h \
    input/keyboard.h \
    input/inputmanager.h \
    input/inputdevice.h \
    input/inputdeviceevent.h \
    input/joystick.h \
    input/sdleventloop.h \
    input/qmlinputdevice.h

PRECOMPILED_HEADER += backendcommon.h

SOURCES += \
    videoitem.cpp \
    core.cpp \
    audiobuffer.cpp \
    audiooutput.cpp \
    logging.cpp \
    input/keyboard.cpp \
    input/inputmanager.cpp \
    input/joystick.cpp \
    input/sdleventloop.cpp \
    input/inputdevice.cpp \
    input/inputdeviceevent.cpp \
    input/qmlinputdevice.cpp


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

QMAKE_EXTRA_TARGETS += portable
