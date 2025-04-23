#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform float alpha;
uniform int bayerLevel = 0;

const float bayer2x2[4] = float[](
     0.0,  2.0,
     3.0,  1.0
);

const float bayer4x4[16] = float[](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
);

const float bayer8x8[64] = float[](
     0.0, 32.0,  8.0, 40.0,  2.0, 34.0, 10.0, 42.0,
    48.0, 16.0, 56.0, 24.0, 50.0, 18.0, 58.0, 26.0,
    12.0, 44.0,  4.0, 36.0, 14.0, 46.0,  6.0, 38.0,
    60.0, 28.0, 52.0, 20.0, 62.0, 30.0, 54.0, 22.0,
     3.0, 35.0, 11.0, 43.0,  1.0, 33.0,  9.0, 41.0,
    51.0, 19.0, 59.0, 27.0, 49.0, 17.0, 57.0, 25.0,
    15.0, 47.0,  7.0, 39.0, 13.0, 45.0,  5.0, 37.0,
    63.0, 31.0, 55.0, 23.0, 61.0, 29.0, 53.0, 21.0
);

float bayerDither2x2(vec2 fragCoord) {
    int x = int(mod(fragCoord.x, 2.0));
    int y = int(mod(fragCoord.y, 2.0));
    int index = y * 2 + x;
    return bayer2x2[index] / 4.0;
}

float bayerDither4x4(vec2 fragCoord) {
    int x = int(mod(fragCoord.x, 4.0));
    int y = int(mod(fragCoord.y, 4.0));
    int index = y * 4 + x;
    return bayer4x4[index] / 16.0;
}

float bayerDither8x8(vec2 fragCoord) {
    int x = int(mod(fragCoord.x, 8.0));
    int y = int(mod(fragCoord.y, 8.0));
    int index = y * 8 + x;
    return bayer8x8[index] / 64.0;
}

void main() {
    vec4 color = texture(texture_diffuse1, TexCoords);
    vec2 fragCoord = gl_FragCoord.xy;

    float threshold;

    if (bayerLevel <= 0)
        threshold = bayerDither2x2(fragCoord);
    else if (bayerLevel == 1)
        threshold = bayerDither4x4(fragCoord);
    else if (bayerLevel >= 2)
        threshold = bayerDither8x8(fragCoord);

    if (alpha < threshold + 1e-5) {
        discard;
    }

    FragColor = vec4(color.rgb, 1.0);
}