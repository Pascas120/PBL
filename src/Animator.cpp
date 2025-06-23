//
// Created by lukas on 16.05.2025.
//

#include "Animator.h"
#include "spdlog/spdlog.h"

Animator::Animator(Animation* animation)
{
    m_CurrentTime = 0.0;
    m_CurrentAnimation = animation;

    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float dt)
{
    m_DeltaTime = dt;
    if (m_CurrentAnimation)
    {
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
    }
}

void Animator::PlayAnimation(Animation* pAnimation)
{
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* bone = m_CurrentAnimation->FindBone(nodeName);

    if (bone)
    {
        bone->Update(m_CurrentTime);
        //spdlog::info(bone->toString());
        nodeTransform = bone->GetLocalTransform();
        // Debug: Log aktualizacji kości
        // spdlog::info("Bone '{}' updated. Current time: {}", nodeName, m_CurrentTime);
        // spdlog::info("Node Transform Matrix:");
        // for (int row = 0; row < 4; ++row)
        // {
        //     spdlog::info("[{}, {}, {}, {}]",
        //                  nodeTransform[row][0],
        //                  nodeTransform[row][1],
        //                  nodeTransform[row][2],
        //                  nodeTransform[row][3]);
        // }
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
        // Debug: Log aktualizacji macierzy końcowej
        spdlog::info("Bone '{}' final matrix updated at index {}.", nodeName, index);
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
{
    return m_FinalBoneMatrices;
}