#ifndef _SP_CAMERA_H_
#define _SP_CAMERA_H_

namespace sp {

class Camera {
public:

    Camera();
    void Init();
    void MoveForward(float dt);
    void MoveBackward(float dt);
    void MoveRight(float dt);
    void MoveLeft(float dt);
    void HandleMouse(int x, int y, float dt);
    void Update();
    void FreeRoam(float delta);
    glm::mat4 LookAt();

private:
    glm::vec3 camera_pos;
    glm::vec3 camera_dir;
    glm::vec3 camera_up;
    glm::vec3 camera_look;
};

} // namespace sp

#endif
