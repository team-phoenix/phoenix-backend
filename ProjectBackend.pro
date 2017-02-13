QT += core multimedia gui

CONFIG += c++14

TARGET = ProjectBackend
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += cpp/src/main.cpp \
    cpp/src/coredemuxer.cpp \
    cpp/src/coresymbols.cpp \
    cpp/src/logging.cpp \
    cpp/src/audiobuffer.cpp \
    cpp/src/audiooutput.cpp \
    cpp/src/sharedprocessmemory.cpp \
    cpp/src/serialization.cpp

HEADERS += \
    cpp/include/coredemuxer.h \
    cpp/include/libretro.h \
    cpp/include/coresymbols.h \
    cpp/include/logging.h \
    cpp/include/audiobuffer.h \
    cpp/include/audiooutput.h \
    cpp/include/sharedprocessmemory.h \
    cpp/include/serialization.h

RESOURCES +=

INCLUDEPATH += ./cpp/src ./cpp/include

INCLUDEPATH += /usr/local/Cellar/libsamplerate/0.1.9/include

LIBS += -L/usr/local/Cellar/libsamplerate/0.1.9/lib

LIBS += -lsamplerate
