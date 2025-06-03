#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D colorTex;  
uniform sampler2D depthTex;  


const float focalDistance = 5.0;
const float focalRange    = 2.0;

uniform vec2 screenSize;    

const int MAX_KERNEL_SIZE = 3; 

float LinearizeDepth(float depthValue)
{
   
    float near = 0.1;  
    float far  = 100.0;
    float z = depthValue * 2.0 - 1.0; 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float depthValue  = texture(depthTex, texCoord).r;
    float linearDepth = LinearizeDepth(depthValue);

    float depthDiff  = abs(linearDepth - focalDistance);
    float blurFactor = clamp(depthDiff / focalRange, 0.0, 1.0);

    int radius = int(blurFactor * float(MAX_KERNEL_SIZE));
    if (radius <= 0) {
        
        FragColor = texture(colorTex, texCoord);
        return;
    }

    vec2 texelSize = 1.0 / screenSize; // [1/width, 1/height]

  
    vec3 colSum = vec3(0.0);
    int count = 0;
    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            colSum += texture(colorTex, texCoord + offset).rgb;
            count++;
        }
    }
    vec3 blurredColor = colSum / float(count);

    vec3 original   = texture(colorTex, texCoord).rgb;
    vec3 finalColor = mix(original, blurredColor, blurFactor);

    FragColor = vec4(finalColor, 1.0);
}
