#version 330 core

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D noiseTexture;
uniform sampler2D depthTexture;

uniform vec3 samples[64];

uniform int width;
uniform int height;

layout(std140) uniform Camera
{
    vec3 viewPos;
    mat4 view;
    mat4 invView;
    mat4 projection;
    mat4 invProjection;
    mat4 viewProjection;
    mat4 invViewProjection;
    mat4 lightProjection;
    mat4 lightView;
    mat4 prevViewProjection;
};

in vec2 texCoord;
out float fragColor;

const int kernelSize = 64;
const float radius = 0.5;
const float bias = 0.1;
const float power = 2.3;

void main()
{
    if (texture(depthTexture, texCoord).r == 1.0) {
        fragColor = 1.0;
        return;
    }

    vec2 noiseScale = vec2(width, height) / 4.0;

    vec3 fragPos = (view * vec4(texture(positionTexture, texCoord).xyz, 1.0)).xyz;
    vec3 normal = normalize(mat3(view) * texture(normalTexture, texCoord).xyz);
    vec3 randomVec = texture(noiseTexture, texCoord * noiseScale).xyz;


    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0)
            continue;

        if (texture(depthTexture, offset.xy).r == 1.0)
            continue;

        float sampleDepth = (view * texture(positionTexture, offset.xy)).z;

        float depthDiff = abs(fragPos.z - sampleDepth);
        float rangeCheck = smoothstep(0.0, 1.0, radius / max(depthDiff, 1e-6));

        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);

    fragColor = pow(occlusion, power);
}
