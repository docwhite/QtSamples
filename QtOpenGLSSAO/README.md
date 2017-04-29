# QtOpenGLSSAO
Demonstrates how to use deferred shading, texture rendering and framebuffer
usage using Qt5's OpenGL wrapping classes to make a screen space ambient
occlusion from geometry as described in Joey de Vries' LearnOpenGL series.

## Dependencies
Assimp is being used for model loading. I have compiled the libraries from
source code and dropped them into the 3rd party folder for MacOSX (Sierra),
Linux (CentOS 7) and Windows 10 so you don't need to install them.

## Building
```
$ git clone git@github.com:docwhite/QtSamples.git
$ cd QtSamples/QtOpenGLSSAO
$ qmake QtOpenGLSSAO.pro
$ make
$ ./ssao
```

## Controls
* `0` displays **position** texture
* `1` displays **normal** texture
* `2` displays **occlusion** texture (SSAO technique)
* `R` toggle rotation (model matrix)

The camera controls are a bit dodgy, for now it's a `lookAt` camera looking at
the origin. The eye can be moved with:

* `RIGHT ARROW` move eye +X
* `LEFT ARROW` move eye -X
* `UP ARROW` move eye +Y
* `DOWN ARROW` move eye -Y
* `W` move eye -Z
* `S` move eye +Z

A small UI will be shown so you can play with some parametrized options for the
SSAO calculation. I recommend reading the LearnOpenGL guide to get a grasp.

