#ifndef _SP_CAMERA_H_
#define _SP_CAMERA_H_

namespace sp {

void InitCamera();
void MoveCameraForward(float dt);
void MoveCameraBackward(float dt);
void MoveCameraRight(float dt);
void MoveCameraLeft(float dt);
void HandleMouse(int x, int y, float dt);
void UpdateCamera();

glm::mat4 CameraLookAt();

} // namespace sp

#endif
