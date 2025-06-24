//
// Created by lukas on 16.05.2025.
//

#ifndef ANIMATOR_H
#define ANIMATOR_H
#include "Animation.h"

class Animator {
public:
    Animator(Animation* animation);

    void UpdateAnimation(float dt);
    void PlayAnimation(Animation* pAnimation);
    void PlayCurrentAnimation();
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
    std::vector<glm::mat4> GetFinalBoneMatrices();
    std::string GetCurrentAnimationName() const {
        if (m_CurrentAnimation != nullptr) {
            return m_CurrentAnimation->path;
        }
        return "No Animation";
    }
    void SetLoop(bool loop) { m_loop = loop; }
    bool IsPlaying() const { return m_isPlaying; }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
    bool m_loop = false;
    bool m_isPlaying = false;
};

#endif //ANIMATOR_H
