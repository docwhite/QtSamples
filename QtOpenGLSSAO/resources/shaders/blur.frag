#version 330 core

out float fColor;

in vec2 vTexCoords;

uniform sampler2D tInputSSAO;

void main() {
    float unblurredSSAO = texture(tInputSSAO, vTexCoords).r;
    fColor = unblurredSSAO;
}
