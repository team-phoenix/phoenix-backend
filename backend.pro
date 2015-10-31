##
## Extra targets
##

    QMAKE_EXTRA_TARGETS += portable

##
## Qt settings
##

    # Undefine this (for some reason it's on by default on Windows)
    CONFIG -= debug_and_release debug_and_release_target
    CONFIG += staticlib static

    TEMPLATE = lib

    QT += qml quick sql multimedia

    TARGET = phoenix-backend

##
## Compiler settings
##

    CONFIG += c++11

    OBJECTS_DIR = obj
    MOC_DIR     = moc
    RCC_DIR     = rcc
    UI_DIR      = gui

    # FIXME: Remove once newer Qt versions make this unnecessary
    macx: QMAKE_MAC_SDK = macosx10.11

    # Include libraries
    win32: INCLUDEPATH += C:/msys64/mingw64/include C:/msys64/mingw64/include/SDL2 # MSYS2
    macx:  INCLUDEPATH += /usr/local/include /usr/local/include/SDL2               # Homebrew
    macx:  INCLUDEPATH += /usr/local/include /opt/local/include/SDL2               # MacPorts
    unix:  INCLUDEPATH += /usr/include /usr/include/SDL2                           # Linux

    # Include externals
    DEFINES += QUAZIP_STATIC
    INCLUDEPATH += ../externals/quazip/quazip

    # Include our stuff
    INCLUDEPATH += core input consumer

    HEADERS += \
        videoitem.h \
        logging.h \
        consumer/audiobuffer.h \
        consumer/audiooutput.h \
        consumer/videooutput.h \
        core/core.h \
        core/corecontrol.h \
        core/libretro.h \
        core/libretrocoreold.h \
        core/libretrocore.h \
        core/libretrovariable.h \
        core/libretrosymbols.h \
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
        logging.cpp \
        consumer/audiobuffer.cpp \
        consumer/audiooutput.cpp \
        consumer/videooutput.cpp \
        core/core.cpp \
        core/corecontrol.cpp \
        core/libretrocoreold.cpp \
        core/libretrocore.cpp \
        core/libretrovariable.cpp \
        core/libretrosymbols.cpp \
        input/keyboard.cpp \
        input/inputmanager.cpp \
        input/joystick.cpp \
        input/sdleventloop.cpp \
        input/inputdevice.cpp \
        input/inputdeviceevent.cpp \
        input/qmlinputdevice.cpp

    RESOURCES += input/controllerdb.qrc

##
## Linker settings
##

    ##
    ## Library paths
    ##

    # Use mingw64 prefix for static builds (uses mingw64/qt5-static prefix by default)
    QMAKE_LFLAGS += -Wl,-Bstatic
    LIBS += C:/msys64/mingw64/lib

    # Externals
    LIBS += -L../externals/quazip/quazip

    # SDL2
    macx: LIBS += -L/usr/local/lib -L/opt/local/lib # Homebrew, MacPorts

    ##
    ## Libraries
    ##

    # Externals
    LIBS += -lquazip

    # SDL 2
    win32: LIBS += -lmingw32 -lSDL2main
    LIBS += -lSDL2

    # Other libraries we use
    LIBS += -lsamplerate -lz
