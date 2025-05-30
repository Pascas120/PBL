#version 330 core

#define FXAA_REDUCE_MIN   (1.0 / 128.0)

#define FXAA_REDUCE_MUL   (1.0 / 8.0)

#define FXAA_SPAN_MAX     8.0


in vec2 texCoord;
uniform sampler2D shadowMap;
uniform vec2 resolution;

out vec4 fragColor;

void main() {
    vec2 inverseRes = 1.0 / resolution;


    float depthNW = texture(shadowMap, texCoord + vec2(-1.0, -1.0) * inverseRes).r;
    float depthNE = texture(shadowMap, texCoord + vec2( 1.0, -1.0) * inverseRes).r;
    float depthSW = texture(shadowMap, texCoord + vec2(-1.0,  1.0) * inverseRes).r;
    float depthSE = texture(shadowMap, texCoord + vec2( 1.0,  1.0) * inverseRes).r;
    float depthM  = texture(shadowMap, texCoord).r;

    float depthMin = min(depthM, min(min(depthNW, depthNE), min(depthSW, depthSE)));
    float depthMax = max(depthM, max(max(depthNW, depthNE), max(depthSW, depthSE)));

    vec2 dir;
    dir.x = -((depthNW + depthNE) - (depthSW + depthSE));
    dir.y =  ((depthNW + depthSW) - (depthNE + depthSE));

    float dirReduce = max(
        (depthNW + depthNE + depthSW + depthSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN
    );

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -FXAA_SPAN_MAX, FXAA_SPAN_MAX) * inverseRes;

    float depthA = 0.5 * (
        texture(shadowMap, texCoord + dir * (1.0 / 3.0 - 0.5)).r +
        texture(shadowMap, texCoord + dir * (2.0 / 3.0 - 0.5)).r
    );

    float depthB = depthA * 0.5 + 0.25 * (
        texture(shadowMap, texCoord + dir * -0.5).r +
        texture(shadowMap, texCoord + dir * 0.5).r
    );

    float finalDepth = (depthB < depthMin || depthB > depthMax) ? depthA : depthB;

    fragColor = vec4(vec3(finalDepth), 1.0);
}