#version 330 core

out float fColor;

in vec2 vTexCoords;

uniform sampler2D tPosition;
uniform sampler2D tNormal;
uniform sampler2D tTexNoise;

uniform vec3 samples[96];
uniform mat4 P;

uniform float radius;
uniform float bias;

uniform int kernelSize;

const vec2 noiseScale = vec2(720.0/4.0, 720.0/4.0); 

void main() {
    // Get the needed inputs
    vec3 position = texture(tPosition, vTexCoords).rgb;
    vec3 normal = texture(tNormal, vTexCoords).rgb;
    vec3 randomVector = normalize(texture(tTexNoise, vTexCoords * noiseScale).xyz);

    // TBN change-of-basis matrix: Change from tangent-space to view-space
    vec3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Iterate over sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        // Get sample position
        vec3 sample = TBN * samples[i];
        sample = position + sample * radius;

        // Project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = P * offset;                  // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;  // transform to range 0.0 - 1.0

        // Get sample depth
        float sampleDepth = texture(tPosition, offset.xy).z;

        // Range check and accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);
    fColor = occlusion;
}
