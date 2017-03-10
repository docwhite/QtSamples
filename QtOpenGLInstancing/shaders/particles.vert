#version 450 core

in vec2 position;
in vec3 color;

in vec3 instance;  // x: positionX, y: positionY, z: radius

out vec3 vCol;

void main(void) {
    vCol = color;
    gl_Position = vec4(instance.z * position + instance.xy, 0.0, 1.0);
}
