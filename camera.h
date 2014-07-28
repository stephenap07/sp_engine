#ifndef _SP_CAMERA_H_
#define _SP_CAMERA_H_

namespace sp {

class Camera {
public:

    Camera();
    void Init();
    void Init(const glm::mat3x3 &camera_mat);
    void MoveForward(float dt);
    void MoveBackward(float dt);
    void MoveRight(float dt);
    void MoveLeft(float dt);
    void HandleMouse(int x, int y, float dt);
    void Update();
    void FreeRoam(float delta);
    glm::mat4 LookAt();

    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 up;
    glm::vec3 look;
};

} // namespace sp

#endif
