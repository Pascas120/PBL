#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

out vec2 texCoords;
out vec3 normal;
out vec3 fragPos;
out vec4 fragPosLightSpace;

out vec3 fragViewPos;
out vec3 fragViewNormal;

out vec4 currClipPos;
out vec4 prevClipPos;

layout(std140) uniform Camera
{
    vec3 viewPos;
    mat4 view;
    mat4 invView;
    mat4 projection;
    mat4 invProjection;
    mat4 viewProjection;
    mat4 invViewProjection;
    mat4 lightProjection;
    mat4 lightView;
    mat4 prevViewProjection;
};

uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform mat4 prevModel;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1)
        continue;
        if(boneIds[i] >=MAX_BONES)
        {
            totalPosition = vec4(aPos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
        totalPosition += localPosition * weights[i];
    }

    if(totalPosition == vec4(0.0f))
    {
        totalPosition = vec4(aPos, 1.0f);
    }

    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;
    texCoords = aTexCoords;
    fragPosLightSpace = lightProjection * lightView * vec4(fragPos, 1.0);

    mat4 modelView = view * model;
    fragViewPos = vec3(modelView * vec4(aPos, 1.0));
    fragViewNormal = mat3(transpose(inverse(modelView))) * aNormal;

    currClipPos = viewProjection * model * totalPosition;
    prevClipPos = prevViewProjection * prevModel * vec4(aPos, 1.0);

    gl_Position = currClipPos;
}