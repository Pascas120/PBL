#version 330 core

uniform sampler2D viewport1;
uniform sampler2D viewport2;

uniform vec2 viewport_size;

uniform bool split_active;
uniform vec2 player1_screen_pos;
uniform vec2 player2_screen_pos;

uniform float split_line_thickness;
uniform vec3 split_line_color;

in vec2 texCoord;
out vec4 fragColor;

float distance_to_line(vec2 p1, vec2 p2, vec2 point) {
    float a = p1.y - p2.y;
    float b = p2.x - p1.x;
    return abs(a * point.x + b * point.y + p1.x * p2.y - p2.x * p1.y) / sqrt(a * a + b * b);
}

void main() {
    vec3 view1 = texture(viewport1, texCoord).rgb;
    vec3 view2 = texture(viewport2, texCoord).rgb;

    if (split_active) {
        vec2 origin = vec2(0.5, 0.5);
        vec2 delta = player2_screen_pos - player1_screen_pos;
        float slope = (delta.y != 0.0) ? delta.x / delta.y : 100000.0;

        float split_y = (origin.x - texCoord.x) * slope + origin.y;
        float p1_y_on_split = (origin.x - player1_screen_pos.x) * slope + origin.y;

        bool use_view1;
        if (texCoord.y > split_y) {
            use_view1 = player1_screen_pos.y > p1_y_on_split;
        } else {
            use_view1 = player1_screen_pos.y < p1_y_on_split;
        }

        vec3 final_color = use_view1 ? view1 : view2;

        // Linie w pikselach
        vec2 p1_px = player1_screen_pos * viewport_size;
        vec2 p2_px = player2_screen_pos * viewport_size;
        vec2 frag_px = texCoord * viewport_size;

        float dist = distance_to_line(p1_px, p2_px, frag_px);
        if (dist < split_line_thickness) {
            final_color = mix(split_line_color, final_color, dist / split_line_thickness);
        }

        fragColor = vec4(final_color, 1.0);
    } else {
        fragColor = vec4(view1, 1.0);
    }
}
