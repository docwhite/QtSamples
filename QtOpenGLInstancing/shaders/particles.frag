#version 450 core
in vec3 vNormal;
out vec4 fCol;

void main(void) {
    fCol = vec4(vNormal, 1.0);
}
