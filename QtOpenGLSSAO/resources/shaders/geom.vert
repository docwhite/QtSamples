#version 330 core

in vec3 position;

in vec3 normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 vPosition;
out vec3 vNormal;

void main() {
    vPosition = vec4(V * M * vec4(position, 1.0)).xyz;
    vPosition.z = -vPosition.z;
    mat3 normalMatrix = transpose(inverse(mat3(V * M)));
    vNormal = normalMatrix * (normal);
    gl_Position = P * V * M * vec4(position, 1.0);
}
