//
// Created by lukas on 25.06.2025.
//

#include "BunController.h"
#include "Scene.h"

#include "BunController.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath> // For fmod, std::abs, std::min, glm::sign
#include <chrono> // Dla std::chrono::high_resolution_clock

void BunController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
    auto& transformSystem = scene->getTransformSystem();
    auto& transform = scene->getComponent<Transform>(id);

    GLFWgamepadstate state;
    glfwGetGamepadState(GLFW_JOYSTICK_1, &state);

    bool advanceButton = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ||
        state.buttons[GLFW_GAMEPAD_BUTTON_A] ||
        state.buttons[GLFW_GAMEPAD_BUTTON_B] ||
        state.buttons[GLFW_GAMEPAD_BUTTON_X] ||
        state.buttons[GLFW_GAMEPAD_BUTTON_Y];

    if(isOpen) {
        if(advanceButton) {
            scene->destroyEntity(id);
            scene->playerLock = false;
        }
        return;
    }

    if(basePosition == glm::vec3(0.0f)) {
        basePosition = transform.translation;
    }

    // --- Logika drżenia ---
    if (isShaking) {
        shakeDuration -= deltaTime;
        if (shakeDuration <= 0.0f) {
            isShaking = false;
            EntityID ent = scene->instantiatePrefab("Kranczips")[0];
            if(!joke.empty()) {
                scene->getComponent<ImageComponent>(ent).texturePath = joke;
                scene->getComponent<BunController>(ent).isOpen = true;
            }
            scene->destroyEntity(id);
            return;
        } else {
            // Aplikuj losowe przesunięcia
            float offsetX = distribution(randomEngine);
            float offsetY = distribution(randomEngine);
            //float offsetZ = distribution(randomEngine);

            // Zastosuj przesunięcie do transform.translation
            // Ważne: to nadpisuje wszelkie inne modyfikacje translation w tej samej klatce
            // i drżenie będzie sumowało się z ruchem.
           // transformSystem.translateEntity(id, transform.translation + glm::vec3(offsetX, offsetY, 0));
            // Możliwa alternatywa: zapisywać oryginalną pozycję i dodawać do niej offset.
            // Aby uniknąć "dryfu" pozycji, najlepiej drgać wokół oryginalnej, nie modyfikować bezpośrednio.
            // PRZYKŁAD LEPSZEGO PODEJŚCIA (Wymaga zapamiętania bazowej pozycji, np. basePosition)
            transformSystem.translateEntity(id, basePosition + glm::vec3(offsetX, offsetY, 0));
        }
    }

    // --- Obsługa wejścia dla spacji (drżenie) ---
    bool currentSpaceState = (advanceButton);
    if (currentSpaceState && !lastSpaceState) { // Sprawdzamy, czy spacja została *naciśnięta* (nie przytrzymana)
        if (currentScale >= TARGET_SCALE) { // Drżenie tylko po zakończeniu animacji skalowania
            isShaking = true;
            shakeDuration = MAX_SHAKE_DURATION;
            scene->getAudioSystem().playSound("res/sounds/otwieranieBulki.wav");
            // Tutaj możesz opcjonalnie zapisać transform.translation do zmiennej basePosition,
            // jeśli chcesz, by drżenie było zawsze wokół statycznej bazowej pozycji.
        }
    }
    lastSpaceState = currentSpaceState; // Zapisz aktualny stan na następną klatkę


    if (currentScale < TARGET_SCALE)
    {
        // Increase scale
        currentScale += scaleSpeed * deltaTime;
        currentScale = glm::min(currentScale, TARGET_SCALE);

        // Apply scaling
        transformSystem.scaleEntity(id, glm::vec3(currentScale));

        // Rotate around Z-axis while scaling
        currentRotationZ += rotationSpeed * deltaTime;
        currentRotationZ = fmod(currentRotationZ, 360.0f);

        glm::quat rotationQuat = glm::angleAxis(glm::radians(currentRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
        transformSystem.rotateEntity(id, rotationQuat);
    }
    else // currentScale is 1.0 or greater, animation is complete
    {
        // Ensure scale is exactly 1.0
        if (currentScale != TARGET_SCALE) {
            currentScale = TARGET_SCALE;
            transformSystem.scaleEntity(id, glm::vec3(TARGET_SCALE));
        }

        // Smoothly rotate to targetRotationZ (0 degrees) if not already there
        if (currentRotationZ != TARGET_ROTATION_Z)
        {
            float rotationDiff = TARGET_ROTATION_Z - currentRotationZ;
            if (rotationDiff > 180.0f) rotationDiff -= 360.0f;
            if (rotationDiff < -180.0f) rotationDiff += 360.0f;

            float rotationStep = rotationSpeed * deltaTime;

            if (std::abs(rotationDiff) <= rotationStep) {
                currentRotationZ = TARGET_ROTATION_Z;
            } else {
                currentRotationZ += glm::sign(rotationDiff) * rotationStep;
            }

            currentRotationZ = fmod(currentRotationZ, 360.0f);
            if (currentRotationZ < 0) currentRotationZ += 360.0f;

            glm::quat rotationQuat = glm::angleAxis(glm::radians(currentRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
            transformSystem.rotateEntity(id, rotationQuat);
        }
        else
        {
            glm::quat rotationQuat = glm::angleAxis(glm::radians(TARGET_ROTATION_Z), glm::vec3(0.0f, 0.0f, 1.0f));
            transformSystem.rotateEntity(id, rotationQuat);
        }
    }
}