//
// Created by lukas on 25.06.2025.
//

#ifndef BULKACONTROLLER_H
#define BULKACONTROLLER_H
#include <random>

#include "ECS/Components.h"


class Scene;

struct BunController {
  EntityID id = (EntityID)-1;

  bool isOpen = false;

  float currentScale = 0.0f;
  float currentRotationZ = 0.0f;


  float rotationSpeed = 720.0f;
  float scaleSpeed = 1.5f;
  float TARGET_SCALE = 1.0f;
  float TARGET_ROTATION_Z = 180.0f;

  bool isShaking = false;
  float shakeDuration = 0.0f;
  float MAX_SHAKE_DURATION = 1.0f;
  float SHAKE_INTENSITY = 10.0f;
  glm::vec3 basePosition = glm::vec3(0.0f);
  std::default_random_engine randomEngine;
  std::uniform_real_distribution<float> distribution{-SHAKE_INTENSITY, SHAKE_INTENSITY};

  std::string joke = "";

  bool lastSpaceState = false;

  BunController() {
    randomEngine.seed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
  }



  void update(GLFWwindow* window, Scene* scene, float deltaTime);
};


#endif //BULKACONTROLLER_H
