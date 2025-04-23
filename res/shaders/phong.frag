#version 410 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec3 viewPos;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform vec3 lightPos;
uniform vec3 lightColor;


//uniform vec3 objectColor;
uniform float shininess;


void main()
{

    vec3 norm = normalize(normal);


    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);


    vec3 ambient = 0.1 * lightColor;


    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;


    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    vec3 result = (ambient + diffuse + specular);

    FragColor = texture(texture_diffuse1, TexCoords) * vec4(result, 1.0);
}