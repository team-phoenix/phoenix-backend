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

    # Needed to grab the native OpenGL context handle
    macx: QT += platformsupport-private

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
    INCLUDEPATH += consumer control core input pipeline util

    # Build with debugging info
    DEFINES += QT_MESSAGELOGCONTEXT

    HEADERS += \
    backendplugin.h \
    consumer/audiobuffer.h \
    consumer/audiooutput.h \
    consumer/videooutput.h \
    consumer/videooutputnode.h \
    control/controloutput.h \
    control/gameconsole.h \
    core/core.h \
    core/libretro.h \
    core/libretrocore.h \
    core/libretroloader.h \
    core/libretrorunner.h \
    core/libretrosymbols.h \
    core/libretrovariable.h \
    core/libretrovariablemodel.h \
    core/libretrovariableforwarder.h \
    input/gamepadmanager.h \
    input/gamepadstate.h \
    input/globalgamepad.h \
    input/keyboardmanager.h \
    input/keyboardmouselistener.h \
    input/keyboardstate.h \
    input/mousemanager.h \
    input/mousestate.h \
    input/remapper.h \
    input/remappermodel.h \
    input/sdlunloader.h \
    pipeline/node.h \
    pipeline/pipelinecommon.h \
    util/logging.h \
    util/microtimer.h \
    util/phoenixwindow.h \
    util/phoenixwindownode.h \

    SOURCES += \
    backendplugin.cpp \
    consumer/audiobuffer.cpp \
    consumer/audiooutput.cpp \
    consumer/videooutput.cpp \
    consumer/videooutputnode.cpp \
    control/controloutput.cpp \
    control/gameconsole.cpp \
    core/core.cpp \
    core/libretrocore.cpp \
    core/libretroloader.cpp \
    core/libretrorunner.cpp \
    core/libretrosymbols.cpp \
    core/libretrovariable.cpp \
    core/libretrovariablemodel.cpp \
    core/libretrovariableforwarder.cpp \
    input/gamepadmanager.cpp \
    input/gamepadstate.cpp \
    input/globalgamepad.cpp \
    input/keyboardmanager.cpp \
    input/keyboardmouselistener.cpp \
    input/keyboardstate.cpp \
    input/mousemanager.cpp \
    input/mousestate.cpp \
    input/remapper.cpp \
    input/remappermodel.cpp \
    input/sdlunloader.cpp \
    pipeline/node.cpp \
    util/logging.cpp \
    util/microtimer.cpp \
    util/phoenixwindow.cpp \
    util/phoenixwindownode.cpp \

    OBJECTIVE_SOURCES += \
    util/osxhelper.mm

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
