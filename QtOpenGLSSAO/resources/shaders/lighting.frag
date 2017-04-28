#version 330 core

out vec4 fColor;

in vec2 vTexCoords;

uniform int pass;
uniform sampler2D tPosition;
uniform sampler2D tNormal;
uniform sampler2D tSSAO;

void main() {
    vec3 color = vec3(0.0);
    vec3 position = texture(tPosition, vTexCoords).rgb;
    vec3 normal = texture(tNormal, vTexCoords).rgb;
    float occlusion = texture(tSSAO, vTexCoords).r;

    switch(pass) {
        case 0:
          color = position;
          break;
        case 1:
          color = normal;
          break;
        case 2:
          color = vec3(occlusion);
        default:
          break;

    }
    fColor = vec4(color, 1.0);
}
