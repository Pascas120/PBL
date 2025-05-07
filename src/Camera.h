//
// Created by Łukasz Moskwin on 29/03/2025.
//

#ifndef PBL_CAMERA_H
#define PBL_CAMERA_H


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

struct Plane
{
    glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
    float     distance = 0.0f;

    Plane() = default;

    Plane(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)),
        distance(glm::dot(normal, p1))
    {}

    float getSignedDistanceToPlane(const glm::vec3& point) const
    {
        return glm::dot(normal, point) - distance;
    }
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    float zNear = 0.1f;
    float zFar = 100.0f;

    Frustum frustum;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    glm::mat4 getViewMatrix() const;
    void setViewMatrix(const glm::mat4& view);
    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void processMouseScroll(float yoffset);

    void createFrustum(float aspect)
    {
        const float halfVSide = zFar * tanf(Zoom * .5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * Front;

        frustum.nearFace = { Position + zNear * Front, Front };
        frustum.farFace = { Position + frontMultFar, -Front };
        frustum.rightFace = { Position,
                                glm::cross(frontMultFar - Right * halfHSide, Up) };
        frustum.leftFace = { Position,
                                glm::cross(Up,frontMultFar + Right * halfHSide) };
        frustum.topFace = { Position,
                                glm::cross(Right, frontMultFar - Up * halfVSide) };
        frustum.bottomFace = { Position,
                                glm::cross(frontMultFar + Up * halfVSide, Right) };

    }
    void updateCameraVectors();
};

#endif //PBL_CAMERA_H
