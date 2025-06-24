#version 330

layout(std140) uniform SplitScreen
{
    float split_slope;
    bool player1_above;

    float split_line_thickness;
    vec4 split_line_color;
};

uniform bool swapStencil;

in vec2 texCoord;

const vec2 center = vec2(0.5, 0.5);

void main() {
    vec2 lineVec = normalize(vec2(1.0, -split_slope));
    vec2 normal = normalize(vec2(-lineVec.y, lineVec.x));

    if (player1_above == swapStencil)
        normal = -normal;

    normal *= 0.005;

    vec2 split_origin = center + normal;

    float split_current_y = (texCoord.x - split_origin.x) * -split_slope + split_origin.y;

    bool above_split = texCoord.y < split_current_y;

    bool should_discard = above_split != player1_above;
    if (swapStencil)
        should_discard = !should_discard;

    if (should_discard) {
        discard;
    }
}