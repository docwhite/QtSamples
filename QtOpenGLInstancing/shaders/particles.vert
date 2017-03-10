#version 450 core

in vec3 position;
in vec3 color;

in vec4 instance;  // x: positionX, y: positionY, z: positionZ, w: radius

out vec3 vNormal;

void main(void) {
    vNormal = normalize(position.xyz);
    vNormal.z = -vNormal.z;
    gl_Position = vec4(instance.w * position + instance.xyz, 1.0);
}
