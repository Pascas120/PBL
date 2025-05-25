#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 diffuse;

uniform sampler2D texture_diffuse1;

void main()
{
    vec4 textureDif = texture(texture_diffuse1, TexCoords);
    FragColor = vec4(diffuse, 1.0) * textureDif;
}