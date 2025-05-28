#version 330 core

uniform sampler2D textureSampler;
uniform sampler2D velTextureSampler;

uniform int width;
uniform int height;

in vec2 texCoord;
out vec4 fragColor;

const int NUM_SAMPLES = 8;
const float STRENGTH = 1.5;

void main()
{
    vec2 velocity = texture(velTextureSampler, texCoord).xy;
    if (length(velocity) < 1e-4) {
        fragColor = texture(textureSampler, texCoord);
        return;
    }
    velocity *= STRENGTH;
    vec4 color = vec4(0.0);
    float weight = 0.0;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        float t = float(i) / float(NUM_SAMPLES - 1);
        t = pow(t, 1.5);
        vec2 offset = texCoord + velocity * (t - 0.5);
        color += texture(textureSampler, offset);
        weight += 1.0;
    }

    fragColor = color / weight;
}