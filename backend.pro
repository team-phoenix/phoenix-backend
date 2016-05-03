##
## Extra targets
##

    QMAKE_EXTRA_TARGETS += portable

##
## Qt settings
##

    # Undefine this (for some reason it's on by default on Windows)
    CONFIG -= debug_and_release debug_and_release_target
    CONFIG += plugin qt

    TEMPLATE = lib

    QT += qml quick multimedia

    TARGET = phoenix-backend

##
## Compiler settings
##

    CONFIG += c++14

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
    INCLUDEPATH += consumer control core input pipeline role util

    HEADERS += \
    backendcommon.h \
    consumer/audiobuffer.h \
    consumer/audiooutput.h \
    consumer/videooutput.h \
    consumer/videooutputnode.h \
    core/core.h \
    core/libretro.h \
    core/libretrocore.h \
    core/libretrosymbols.h \
    core/libretrovariable.h \
    input/inputdevice.h \
    input/inputdeviceevent.h \
    input/inputmanager.h \
    input/joystick.h \
    input/keyboard.h \
    input/qmlinputdevice.h \
    input/sdleventloop.h \
    pipeline/node.h \
    role/consumer.h \
    role/control.h \
    role/controllable.h \
    role/producer.h \
    util/controlhelper.h \
    util/logging.h \
    pipeline/pipelinecommon.h \
    util/microtimer.h \
    control/gameconsole.h \
    input/gamepadmanager.h \
    input/globalgamepad.h \
    input/remapper.h \
    util/controloutput.h \
    util/phoenixwindow.h \
    util/phoenixwindownode.h \
    input/gamepad.h \
    backendplugin.h \
    input/remappermodel.h

    SOURCES += \
    consumer/audiobuffer.cpp \
    consumer/audiooutput.cpp \
    consumer/videooutput.cpp \
    consumer/videooutputnode.cpp \
    core/core.cpp \
    core/libretrocore.cpp \
    core/libretrosymbols.cpp \
    core/libretrovariable.cpp \
    input/inputdevice.cpp \
    input/inputdeviceevent.cpp \
    input/inputmanager.cpp \
    input/joystick.cpp \
    input/keyboard.cpp \
    input/qmlinputdevice.cpp \
    input/sdleventloop.cpp \
    pipeline/node.cpp \
    role/consumer.cpp \
    role/control.cpp \
    role/controllable.cpp \
    role/producer.cpp \
    util/logging.cpp \
    util/microtimer.cpp \
    control/gameconsole.cpp \
    input/gamepadmanager.cpp \
    input/globalgamepad.cpp \
    input/remapper.cpp \
    util/controloutput.cpp \
    util/phoenixwindow.cpp \
    util/phoenixwindownode.cpp \
    backendplugin.cpp \
    input/remappermodel.cpp \
    input/gamepad.cpp

    RESOURCES += input/controllerdb.qrc

##
## Linker settings
##

    ##
    ## Library paths
    ##

    win32: LIBS += -LC:/msys64/mingw64/lib

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
