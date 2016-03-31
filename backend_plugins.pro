uri = "Phoenix.Backend"

QT += qml quick multimedia

# Undefine this (for some reason it's on by default on Windows)
CONFIG -= debug_and_release debug_and_release_target

# We'll always be 64-bit
CONFIG += qt plugin c++11 x86_64
TEMPLATE = lib
TARGET = $$qtLibraryTarget($$TARGET)
DISTFILES = qmldir
RESOURCES += input/controllerdb.qrc
QMAKE_EXTRA_TARGETS += portable

OBJECTS_DIR = .obj
MOC_DIR     = .moc
RCC_DIR     = .rcc
UI_DIR      = .gui

INCLUDEPATH += core consumer input role util

win32 {
    LIBS += -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main
    INCLUDEPATH += C:/msys64/mingw64/include C:/msys64/mingw64/include/SDL2 # MSYS2
}

macx {
    INCLUDEPATH += /usr/local/include /usr/local/include/SDL2               # Homebrew
    INCLUDEPATH += /usr/local/include /opt/local/include/SDL2               # MacPorts
    LIBS += -L/usr/local/lib -L/opt/local/lib # Homebrew, MacPorts

    # FIXME: Remove once newer Qt versions make this unnecessary
    QMAKE_MAC_SDK = macosx10.11
}

unix {
    INCLUDEPATH += /usr/include /usr/include/SDL2                           # Linux
}

LIBS += -lSDL2 -lsamplerate -lz

HEADERS += \
        consumer/audiobuffer.h \
        consumer/audiooutput.h \
        consumer/videooutput.h \
        core/core.h \
        core/corecontrol.h \
        core/corecontrolproxy.h \
        core/libretro.h \
        core/libretrocore.h \
        core/libretrovariable.h \
        core/libretrosymbols.h \
        input/keyboard.h \
        input/inputmanager.h \
        input/inputdevice.h \
        input/inputdeviceevent.h \
        input/joystick.h \
        input/sdleventloop.h \
        input/qmlinputdevice.h \
        role/consumer.h \
        role/controllable.h \
        role/control.h \
        role/producer.h \
        util/controlhelper.h \
        util/logging.h \
        util/looper.h \
        backendplugin.h

    PRECOMPILED_HEADER += backendcommon.h

    SOURCES += \
        consumer/audiobuffer.cpp \
        consumer/audiooutput.cpp \
        consumer/videooutput.cpp \
        core/core.cpp \
        core/corecontrol.cpp \
        core/corecontrolproxy.cpp \
        core/libretrocore.cpp \
        core/libretrovariable.cpp \
        core/libretrosymbols.cpp \
        input/keyboard.cpp \
        input/inputmanager.cpp \
        input/joystick.cpp \
        input/sdleventloop.cpp \
        input/inputdevice.cpp \
        input/inputdeviceevent.cpp \
        input/qmlinputdevice.cpp \
        role/consumer.cpp \
        role/controllable.cpp \
        role/control.cpp \
        role/producer.cpp \
        util/logging.cpp \
        util/looper.cpp  \
        backendplugin.cpp

# [!! COPY FILES INTO BUILD LOCATION !!]
!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/$$DESTDIR/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}


# [!! END COPYING !!]

# [!! COPY FILES TO INSTALL LOCATION DURING "make install" !!]
qmldir.files = qmldir
installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
qmldir.path = $$installPath
target.path = $$installPath
INSTALLS += target qmldir
# [!! END MAKE INSTALL COPYING !!]
