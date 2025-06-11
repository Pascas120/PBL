#version 410
precision mediump float;

uniform vec2 viewport_size;
uniform sampler2D viewport1;
uniform sampler2D viewport2;
//uniform vec2 player1_position;
//uniform vec2 player2_position;
//uniform vec2 player1_screen_pos;
//uniform vec2 player2_screen_pos;
uniform float split_slope;
uniform bool player1_above;

uniform float split_line_thickness = 5.0;
uniform vec4 split_line_color;

in vec2 texCoord;           // UV coordinates passed from vertex shader
out vec4 fragColor;     // Output color

float distanceToLine(vec2 p1, vec2 p2, vec2 point) {
    float a = p1.y - p2.y;
    float b = p2.x - p1.x;
    return abs(a * point.x + b * point.y + p1.x * p2.y - p2.x * p1.y) / sqrt(a * a + b * b);
}

void main() {
    vec3 view1 = texture(viewport1, texCoord).rgb;
    vec3 view2 = texture(viewport2, texCoord).rgb;

    float width = viewport_size.x;
    float height = viewport_size.y;

    vec2 split_origin = vec2(0.5, 0.5);
    vec2 split_line_start = vec2(0.0, height * ((split_origin.x - 0.0) * split_slope + split_origin.y));
    vec2 split_line_end = vec2(width, height * ((split_origin.x - 1.0) * split_slope + split_origin.y));
    float distance_to_split_line = distanceToLine(
    split_line_start, split_line_end,
    vec2(texCoord.x * width, texCoord.y * height)
    );

    if (distance_to_split_line < split_line_thickness) {
        fragColor = split_line_color;
    } else {
        float split_current_y = (split_origin.x - texCoord.x) * split_slope + split_origin.y;

        bool above_split = texCoord.y > split_current_y;

        if ((above_split && player1_above) || (!above_split && !player1_above)) {
            fragColor = vec4(view1, 1.0);
        } else {
            fragColor = vec4(view2, 1.0);
        }
    }
}