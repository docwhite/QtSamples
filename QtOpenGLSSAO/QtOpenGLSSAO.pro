TARGET = ssao
TEMPLATE = app

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
    shaders/geometry.vert \
    shaders/geometry.frag \
    shaders/quad.vert \
    shaders/quad.frag

OTHER_FILES += \
    .gitignore \
    Doxyfile \
    Mainpage.dox \
    README.md
