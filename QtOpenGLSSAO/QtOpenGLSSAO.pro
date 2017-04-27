TARGET = ssao
TEMPLATE = app

CONFIG += c++11

QT += core gui

CONFIG += c++11
CONFIG -= app_bundle

SOURCES += \
    src/main.cpp \
    src/AbstractScene.cpp \
    src/Scene.cpp \
    src/Window.cpp


OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += include

HEADERS += \
    include/AbstractScene.h \
    include/Scene.h \
    include/Window.h

OTHER_FILES += \
    resources/shaders/quad.vert \
    resources/shaders/quad.frag

OTHER_FILES += \
    .gitignore \
    Doxyfile \
    README.md

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

win32 {
    INCLUDEPATH += "$$PWD/../3rdparty/Assimp/include"
    LIBS += -L"$$PWD/../3rdparty/Assimp/lib" -l"assimp-vc140-mt"
    LIBS += -L"$$PWD/../3rdparty/Assimp/bin"
}

RESOURCES += \
    resources.qrc

DISTFILES += \
    resources/shaders/geom.frag \
    resources/shaders/geom.vert
