    TEMPLATE = lib
    TARGET = phxinputplugin
    CONFIG += qt plugin

    QT += qml quick multimedia

    win32: LIBS += -lmingw32 -lSDL2main
    LIBS += -lSDL2

    # Include libraries
    win32: INCLUDEPATH += C:/msys64/mingw64/include C:/msys64/mingw64/include/SDL2 # MSYS2
    macx:  INCLUDEPATH += /usr/local/include /usr/local/include/SDL2               # Homebrew
    macx:  INCLUDEPATH += /usr/local/include /opt/local/include/SDL2               # MacPorts
    unix:  INCLUDEPATH += /usr/include /usr/include/SDL2                           # Linux

    RESOURCES += ../../input/controllerdb.qrc

    OTHER_FILES += qmldir

    INCLUDEPATH += ../../input ../../role ../../util ../../ ../../core

    HEADERS += ../../input/keyboard.h \
        ../../input/inputmanager.h \
        ../../input/inputdevice.h \
        ../../input/inputdeviceevent.h \
        ../../input/joystick.h \
        ../../input/sdleventloop.h \
        ../../input/qmlinputdevice.h \
        ../../backendcommon.h \
        ../../core/libretro.h \
        ../../role/producer.h \
        ../../role/consumer.h \
        ../../util/logging.h \
        ../../role/controllable.h \
        ../../role/control.h \
    phxinputplugin.h

    SOURCES += ../../input/keyboard.cpp \
        ../../input/inputmanager.cpp \
        ../../input/joystick.cpp \
        ../../input/sdleventloop.cpp \
        ../../input/inputdevice.cpp \
        ../../input/inputdeviceevent.cpp \
        ../../input/qmlinputdevice.cpp \
        ../../role/producer.cpp \
        ../../role/consumer.cpp \
        ../../util/logging.cpp \
        ../../role/controllable.cpp \
        ../../role/control.cpp \
    phxinputplugin.cpp

URI = "Phoenix/PhxInput"
unix {
    installPath = $$[QT_INSTALL_QML]/$$URI
    qmldir.path = $$installPath
    qmldir.files += $$PWD/$$OTHER_FILES
    target.path = $$installPath
    INSTALLS += target qmldir

}
