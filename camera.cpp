#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>

#include "camera.h"

namespace sp {

Camera::Camera() {
    Init();
}

//------------------------------------------------------------------------------

void Camera::Init()
{
    pos  = glm::vec3(1.0f, 1.0f, 1.0f);
    dir  = glm::vec3(0.0f, 0.0f, -1.0f);
    up   = glm::vec3(0.0f, 1.0f, 0.0f);

    look = glm::vec3(pos + dir);
}

//------------------------------------------------------------------------------

void Camera::Init(const glm::mat3x3 &camera_mat)
{
    pos  = camera_mat[0];
    dir  = camera_mat[1];
    up   = camera_mat[2];

    look = glm::vec3(pos + dir);
}

//------------------------------------------------------------------------------

void Camera::MoveForward(float dt)
{
    pos += 10.0f * dt * dir;
}

//------------------------------------------------------------------------------

void Camera::MoveBackward(float dt)
{
    pos -= 10.0f * dt * dir;
}

//------------------------------------------------------------------------------

void Camera::MoveRight(float dt)
{
    pos += 10.0f * dt * glm::cross(dir, up);
}

//------------------------------------------------------------------------------

void Camera::MoveLeft(float dt)
{
    pos -= 10.0f * dt * glm::cross(dir, up);
}

//------------------------------------------------------------------------------

void Camera::HandleMouse(int x, int y, float dt)
{
    glm::vec3 axis = glm::normalize(glm::cross(dir, up));
    glm::quat pitch_quat = glm::angleAxis(-10.0f * y * dt, axis);
    dir = glm::rotate(pitch_quat, dir);

    glm::quat heading_quat = glm::angleAxis(-10.0f * x * dt, glm::vec3(0.0f, 1.0f, 0.0f));
    dir = glm::rotate(heading_quat, dir);
}

//------------------------------------------------------------------------------

void Camera::Update()
{
    look = pos + dir;
    dir = glm::normalize(look - pos);
}

//------------------------------------------------------------------------------

void Camera::FreeRoam(float delta)
{
    const unsigned char *state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_W]) {
        MoveForward(delta);
    }
    if (state[SDL_SCANCODE_S]) {
        MoveBackward(delta);
    }
    if (state[SDL_SCANCODE_D]) {
        MoveRight(delta);
    }
    if (state[SDL_SCANCODE_A]) {
        MoveLeft(delta);
    } 
}

//------------------------------------------------------------------------------

glm::mat4 Camera::LookAt()
{
    Update();
    return glm::lookAt(pos, look, up);
}

} // namespace sp
