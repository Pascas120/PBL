#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform float time; // Czas przekazywany z silnika gry
uniform vec3 glowColor; // Kolor œwiecenia (np. z³oty chleb)

void main()
{
    vec4 baseColor = texture(texture_diffuse1, TexCoords);

    // Pulsuj¹ca wartoœæ (od 0.8 do 1.0 w sinusoidalnym cyklu)
    float pulse = 0.8 + 0.2 * sin(time * 4.0);

    // Dodajemy glow tylko do jasnych pikseli tekstury
    float brightness = dot(baseColor.rgb, vec3(0.299, 0.587, 0.114)); // luminancja

    if (brightness > 0.1) {
        vec3 glow = glowColor * pulse;
        FragColor = vec4(baseColor.rgb + glow * 0.4, baseColor.a);
    } else {
        FragColor = baseColor;
    }
}