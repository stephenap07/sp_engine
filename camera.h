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

    void Rotate(float angle, glm::vec3 vec);
    void MoveForward(float dt);
    void MoveBackward(float dt);
    void MoveRight(float dt);
    void MoveLeft(float dt);
    void HandleMouse(int x, int y, float dt);

    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 up;
    glm::vec3 look;

private:
    glm::quat rot;
};

} // namespace sp

#endif
