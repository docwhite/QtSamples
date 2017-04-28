#version 330 core

out float fColor;

in vec2 vTexCoords;

uniform int pass;
uniform sampler2D tPosition;
uniform sampler2D tNormal;
uniform sampler2D tTexNoise;

uniform vec3 samples[64];

void main() {
    vec3 position = texture(tPosition, vTexCoords).rgb;
    vec3 normal = texture(tNormal, vTexCoords).rgb;
    fColor = 0.9;
}
