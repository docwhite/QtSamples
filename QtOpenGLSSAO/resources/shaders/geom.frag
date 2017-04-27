#version 330 core

in vec3 vPosition;
in vec3 vNormal;

layout (location = 0) out vec3 gPositionPass;  // Color Attachment 0
layout (location = 1) out vec3 gNormalPass;    // Color Attachment 1

void main() {
    gPositionPass = vPosition;
    gNormalPass = vNormal;
}
