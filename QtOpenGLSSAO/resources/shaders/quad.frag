#version 330 core

uniform int pass;
uniform sampler2D tPosition;
uniform sampler2D tNormal;

in vec2 vTexCoords;

out vec4 fColor;

void main() {
    vec3 color = vec3(0.0);
    vec3 position = texture2D(tPosition, vTexCoords).rgb;
    vec3 normal = texture2D(tNormal, vTexCoords).rgb;

    switch(pass) {
        case 0:
          color = position;
          break;
        case 1:
          color = normal;
          break;
        case 2:
          color = vec3(position.z);
        default:
          break;

    }
    fColor = vec4(color, 1.0);
}
