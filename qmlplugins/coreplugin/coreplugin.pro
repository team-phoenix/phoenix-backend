    TEMPLATE = lib
    TARGET = phxcoreplugin
    CONFIG += qt plugin

    QT += qml quick multimedia

    INCLUDEPATH += ../../input ../../role ../../util ../../ ../../core ../../consumer

    OTHER_FILES += qmldir

    win32: LIBS += -lmingw32 -lSDL2main
    LIBS += -lSDL2

    # Include libraries
    win32: INCLUDEPATH += C:/msys64/mingw64/include C:/msys64/mingw64/include/SDL2 # MSYS2
    macx:  INCLUDEPATH += /usr/local/include /usr/local/include/SDL2               # Homebrew
    macx:  INCLUDEPATH += /usr/local/include /opt/local/include/SDL2               # MacPorts
    unix:  INCLUDEPATH += /usr/include /usr/include/SDL2                           # Linux

    HEADERS += \
        ../../util/looper.h \
        ../../consumer/audiobuffer.h \
        ../../consumer/audiooutput.h \
        ../../core/core.h \
        ../../core/corecontrol.h \
        ../../core/corecontrolproxy.h \
        ../../core/libretro.h \
        ../../core/libretrocore.h \
        ../../core/libretrovariable.h \
        ../../core/libretrosymbols.h \
        ../../backendcommon.h \
        ../../util/controlhelper.h \
        ../../util/logging.h \
        ../../input/inputmanager.h \
        ../../input/inputdevice.h \
        ../../input/qmlinputdevice.h \
        ../../input/inputdeviceevent.h \
        ../../input/sdleventloop.h \
        ../../input/joystick.h \
         ../../consumer/videooutput.h \
    coreplugin.h

    SOURCES += \
        ../../util/looper.cpp \
        ../../consumer/audiobuffer.cpp \
        ../../consumer/audiooutput.cpp \
        ../../core/core.cpp \
        ../../core/corecontrol.cpp \
        ../../core/corecontrolproxy.cpp \
        ../../core/libretrocore.cpp \
        ../../core/libretrovariable.cpp \
        ../../core/libretrosymbols.cpp \
        ../../role/producer.cpp \
        ../../role/consumer.cpp \
        ../../util/logging.cpp \
        ../../role/controllable.cpp \
        ../../role/control.cpp \
        ../../input/inputmanager.cpp \
        ../../input/inputdevice.cpp \
        ../../input/qmlinputdevice.cpp \
        ../../input/inputdeviceevent.cpp \
        ../../input/sdleventloop.cpp \
        ../../input/joystick.cpp \
         ../../consumer/videooutput.cpp \
    coreplugin.cpp

URI = "Phoenix/PhxCore"
unix {
    installPath = $$[QT_INSTALL_QML]/$$URI
    qmldir.path = $$installPath
    qmldir.files += $$PWD/$$OTHER_FILES
    target.path = $$installPath
    INSTALLS += target qmldir
}
