TARGET = ssao
TEMPLATE = app

CONFIG += c++11

QT += core gui widgets

CONFIG += c++11
CONFIG -= app_bundle

SOURCES += \
    src/main.cpp \
    src/AbstractScene.cpp \
    src/Scene.cpp \
    src/Window.cpp \
    src/Panel.cpp


OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += \
    include \
    $$PWD/../3rdparty/include

HEADERS += \
    include/AbstractScene.h \
    include/Scene.h \
    include/Window.h \
    include/Panel.h

OTHER_FILES += \
    .gitignore \
    Doxyfile \
    README.md

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

win32 {
    LIBS += -L$$PWD/../3rdparty/windows/Assimp/lib -lassimp-vc140-mt
    LIBS += -L$$PWD/../3rdparty/windows/Assimp/bin
}

macx {
    LIBS += -L$$PWD/../3rdparty/mac/Assimp/lib -lassimp
}

unix:!macx {
    LIBS += -L$$PWD/../3rdparty/linux/z/lib -lz
    LIBS += -L$$PWD/../3rdparty/linux/Assimp/lib -lassimp
}

RESOURCES += \
    resources.qrc
