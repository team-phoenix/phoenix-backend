##
## Extra targets
##

    QMAKE_EXTRA_TARGETS += portable

##
## Qt settings
##

    QT += qml quick multimedia

    uri = "Phoenix.Backend"

    # Undefine this (for some reason it's on by default on Windows)
    CONFIG -= debug_and_release debug_and_release_target
    CONFIG += qt plugin

    # FIXME: Remove once newer Qt versions make this unnecessary
    macx: QMAKE_MAC_SDK = macosx10.11

##
## Compiler settings
##

    TARGET = $$qtLibraryTarget($$TARGET)

    TEMPLATE = lib

    CONFIG += c++11

    DISTFILES = qmldir
    RESOURCES += input/controllerdb.qrc

    OBJECTS_DIR = .obj
    MOC_DIR     = .moc
    RCC_DIR     = .rcc
    UI_DIR      = .gui

    # Include platform specific libraries
    win32: INCLUDEPATH += C:/msys64/mingw64/include C:/msys64/mingw64/include/SDL2 # MSYS2
    macx {
        INCLUDEPATH += /usr/local/include /usr/local/include/SDL2                  # Homebrew
        INCLUDEPATH += /usr/local/include /opt/local/include/SDL2                  # MacPorts
    }
    unix:  INCLUDEPATH += /usr/include /usr/include/SDL2                           # Linux

    # Include external libraries
    # ...
    # ...

    # Include our stuff
    INCLUDEPATH += core consumer input role util

    HEADERS += \
    consumer/audiobuffer.h \
    consumer/audiooutput.h \
    consumer/videooutput.h \
    core/core.h \
    core/libretro.h \
    core/libretrocore.h \
    core/libretrovariable.h \
    core/libretrosymbols.h \
    input/keyboard.h \
    input/inputdevice.h \
    input/joystick.h \
    input/sdleventloop.h \
    role/consumer.h \
    role/controllable.h \
    role/control.h \
    role/producer.h \
    util/controlhelper.h \
    util/logging.h \
    util/looper.h \
    backendplugin.h \
    backendcommon.h \
    input/keyboardevent.h \
    input/joystickevent.h \
    libretro_cpp.h \
    util/iter.h \
    input/inputmapvalue.h \
    input/gamepad.h \
    core/gameconsoleproxy.h \
    core/gameconsole.h \
    input/gamepadmanager.h

    SOURCES += \
    consumer/audiobuffer.cpp \
    consumer/audiooutput.cpp \
    consumer/videooutput.cpp \
    core/core.cpp \
    core/libretrocore.cpp \
    core/libretrovariable.cpp \
    core/libretrosymbols.cpp \
    input/keyboard.cpp \
    input/joystick.cpp \
    input/sdleventloop.cpp \
    input/inputdevice.cpp \
    role/consumer.cpp \
    role/controllable.cpp \
    role/control.cpp \
    role/producer.cpp \
    util/logging.cpp \
    util/looper.cpp  \
    backendplugin.cpp \
    input/keyboardevent.cpp \
    input/joystickevent.cpp \
    util/iter.cpp \
    input/inputmapvalue.cpp \
    input/gamepad.cpp \
    core/gameconsoleproxy.cpp \
    core/gameconsole.cpp \
    input/gamepadmanager.cpp

##
## Linker settings
##

    LIBS += -LC:/msys64/mingw64/lib

    # SDL2
    macx: LIBS += -L/usr/local/lib -L/opt/local/lib # Homebrew, MacPorts
    win32: LIBS += -lmingw32 -lSDL2main
    LIBS += -lSDL2

    # Secret Rabbit Code (libsamplerate)
    LIBS += -lsamplerate

##
## qmldir file
##

    # Copy the qmldir file to the output folder
    !equals(_PRO_FILE_PWD_, $$OUT_PWD) {
        qmldir.target = $$OUT_PWD/$$DESTDIR/qmldir
        qmldir.depends = $$_PRO_FILE_PWD_/qmldir
        qmldir.commands = $(COPY_FILE) \"$$replace(qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(qmldir.target, /, $$QMAKE_DIR_SEP)\"
        QMAKE_EXTRA_TARGETS += qmldir
        PRE_TARGETDEPS += $$qmldir.target
    }
