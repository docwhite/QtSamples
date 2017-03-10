#version 450 core

in vec3 vCol;
out vec4 fCol;

void main(void) {
    fCol = vec4(vCol, 1.0);
}
