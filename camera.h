#ifndef _SP_CAMERA_H_
#define _SP_CAMERA_H_

namespace sp {

class Camera {
public:

    Camera();
    void InitCamera();
    void MoveCameraForward(float dt);
    void MoveCameraBackward(float dt);
    void MoveCameraRight(float dt);
    void MoveCameraLeft(float dt);
    void HandleMouse(int x, int y, float dt);
    void UpdateCamera();
    void FreeRoamCamera(float delta);
    glm::mat4 CameraLookAt();

private:
    glm::vec3 camera_pos;
    glm::vec3 camera_dir;
    glm::vec3 camera_up;
    glm::vec3 camera_look;
};

} // namespace sp

#endif
