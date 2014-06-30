#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>

#include "camera.h"

namespace sp {

Camera::Camera() {
    InitCamera();
}

//------------------------------------------------------------------------------

void Camera::InitCamera()
{
    camera_pos  = glm::vec3(1.0f, 1.0f, 1.0f);
    camera_dir  = glm::vec3(0.0f, 0.0f, -1.0f);
    camera_up   = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_look = glm::vec3(camera_pos + camera_dir);
}

//------------------------------------------------------------------------------

void Camera::MoveCameraForward(float dt)
{
    camera_pos += 10.0f * dt * camera_dir;
}

//------------------------------------------------------------------------------

void Camera::MoveCameraBackward(float dt)
{
    camera_pos -= 10.0f * dt * camera_dir;
}

//------------------------------------------------------------------------------

void Camera::MoveCameraRight(float dt)
{
    camera_pos += 10.0f * dt * glm::cross(camera_dir, camera_up);
}

//------------------------------------------------------------------------------

void Camera::MoveCameraLeft(float dt)
{
    camera_pos -= 10.0f * dt * glm::cross(camera_dir, camera_up);
}

//------------------------------------------------------------------------------

void Camera::HandleMouse(int x, int y, float dt)
{
    glm::vec3 axis = glm::normalize(glm::cross(camera_dir, camera_up));
    glm::quat pitch_quat = glm::angleAxis(-10.0f * y * dt, axis);
    camera_dir = glm::rotate(pitch_quat, camera_dir);

    glm::quat heading_quat = glm::angleAxis(-10.0f * x * dt, glm::vec3(0.0f, 1.0f, 0.0f));
    camera_dir = glm::rotate(heading_quat, camera_dir);
}

//------------------------------------------------------------------------------

void Camera::UpdateCamera()
{
    camera_look = camera_pos + camera_dir;
    camera_dir = glm::normalize(camera_look - camera_pos);
}

//------------------------------------------------------------------------------

void Camera::FreeRoamCamera(float delta)
{
    const unsigned char *state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_W]) {
        MoveCameraForward(delta);
    }
    if (state[SDL_SCANCODE_S]) {
        MoveCameraBackward(delta);
    }
    if (state[SDL_SCANCODE_D]) {
        MoveCameraRight(delta);
    }
    if (state[SDL_SCANCODE_A]) {
        MoveCameraLeft(delta);
    } 
}

//------------------------------------------------------------------------------

glm::mat4 Camera::CameraLookAt()
{
    UpdateCamera();
    return glm::lookAt(camera_pos, camera_look, camera_up);
}

} // namespace sp
