#version 330 core

out float fColor;

in vec2 vTexCoords;

uniform sampler2D tInputSSAO;

uniform int blurAmount;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(tInputSSAO, 0));
    float result = 0.0;

    for (int x = -blurAmount; x < blurAmount; ++x)
    {
        for (int y = -blurAmount; y < blurAmount; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(tInputSSAO, vTexCoords + offset).r;
        }
    }
    fColor = result / float(blurAmount*blurAmount*blurAmount*blurAmount);
}
