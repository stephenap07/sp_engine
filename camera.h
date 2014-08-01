#ifndef _SP_CAMERA_H_
#define _SP_CAMERA_H_

namespace sp {

class Camera {
public:

    Camera();
    void Init();
    void Init(const glm::mat3x3 &camera_mat);
    void Update();
    void FreeRoam(float delta);
    glm::mat4 LookAt();

    virtual void MoveForward(float dt);
    virtual void MoveBackward(float dt);
    virtual void MoveRight(float dt);
    virtual void MoveLeft(float dt);
    virtual void HandleMouse(int x, int y, float dt);

    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 up;
    glm::vec3 look;
};

} // namespace sp

#endif
