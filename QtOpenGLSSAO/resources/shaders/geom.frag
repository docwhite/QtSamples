#version 330 core

in vec3 vPosition;
in vec3 vNormal;

layout (location = 0) out vec3 gPositionPass;
layout (location = 1) out vec3 gNormalPass;

void main() {
    gPositionPass = vPosition;
    gNormalPass = normalize(vNormal);
}
