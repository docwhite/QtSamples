#version 330 core

out float fColor;

in vec2 vTexCoords;

uniform sampler2D tInputSSAO;

void main() {
    fColor = 0.9;
}
