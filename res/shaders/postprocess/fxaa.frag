#version 330 core

#ifndef FXAA_REDUCE_MIN
#define FXAA_REDUCE_MIN   (1.0 / 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
#define FXAA_SPAN_MAX     8.0
#endif

in vec2 texCoord;
uniform sampler2D textureSampler;
uniform sampler2D testSampler;
uniform vec2 resolution;

out vec4 fragColor;

void main() {
    vec2 inverseRes = 1.0 / resolution;

    vec2 posNW = texCoord + vec2(-1.0, -1.0) * inverseRes;
    vec2 posNE = texCoord + vec2( 1.0, -1.0) * inverseRes;
    vec2 posSW = texCoord + vec2(-1.0,  1.0) * inverseRes;
    vec2 posSE = texCoord + vec2( 1.0,  1.0) * inverseRes;

    vec3 rgbNW = texture(testSampler, posNW).rgb;
    vec3 rgbNE = texture(testSampler, posNE).rgb;
    vec3 rgbSW = texture(testSampler, posSW).rgb;
    vec3 rgbSE = texture(testSampler, posSE).rgb;
    vec3 rgbM  = texture(testSampler, texCoord).rgb;

    vec3 lumaWeights = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, lumaWeights);
    float lumaNE = dot(rgbNE, lumaWeights);
    float lumaSW = dot(rgbSW, lumaWeights);
    float lumaSE = dot(rgbSE, lumaWeights);
    float lumaM  = dot(rgbM,  lumaWeights);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
    (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
    FXAA_REDUCE_MIN
    );

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -FXAA_SPAN_MAX, FXAA_SPAN_MAX) * inverseRes;

    vec3 rgbA = 0.5 * (
    texture(textureSampler, texCoord + dir * (1.0 / 3.0 - 0.5)).rgb +
    texture(textureSampler, texCoord + dir * (2.0 / 3.0 - 0.5)).rgb
    );
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
    texture(textureSampler, texCoord + dir * -0.5).rgb +
    texture(textureSampler, texCoord + dir * 0.5).rgb
    );

    float lumaB = dot(rgbB, lumaWeights);
    vec3 finalColor = (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;

    fragColor = vec4(finalColor, 1.0);
}