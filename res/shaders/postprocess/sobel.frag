#version 330 core

uniform sampler2D textureSampler;
uniform int width;
uniform int height;

in vec2 texCoord;
out vec4 fragColor;

void make_kernel(inout vec4 n[9], sampler2D tex, vec2 coord)
{
    float w = 1.0 / float(width);
    float h = 1.0 / float(height);

    n[0] = texture(tex, coord + vec2(-w, -h));
    n[1] = texture(tex, coord + vec2( 0.0, -h));
    n[2] = texture(tex, coord + vec2( w, -h));
    n[3] = texture(tex, coord + vec2(-w,  0.0));
    n[4] = texture(tex, coord);
    n[5] = texture(tex, coord + vec2( w,  0.0));
    n[6] = texture(tex, coord + vec2(-w,  h));
    n[7] = texture(tex, coord + vec2( 0.0,  h));
    n[8] = texture(tex, coord + vec2( w,  h));
}

void main()
{
    vec4 n[9];
    make_kernel(n, textureSampler, texCoord);

    vec4 sobel_edge_h = n[2] + vec4(2.0) * n[5] + n[8] - (n[0] + vec4(2.0) * n[3] + n[6]);
    vec4 sobel_edge_v = n[0] + vec4(2.0) * n[1] + n[2] - (n[6] + vec4(2.0) * n[7] + n[8]);
    vec4 sobel = sqrt(sobel_edge_h * sobel_edge_h + sobel_edge_v * sobel_edge_v);
    float luma = 0.2126 * sobel.r + 0.7152 * sobel.g + 0.0722 * sobel.b;

    fragColor = texture(textureSampler, texCoord) - vec4(luma,luma,luma, 0.0f);
}
