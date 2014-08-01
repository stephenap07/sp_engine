#ifndef _SP_IQM_MODEL_H_
#define _SP_IQM_MODEL_H_

#include "iqm.h"

namespace sp {

class IQMModel {
public:
    ~IQMModel();

    bool LoadModel(const char *filename);
    void AnimateIQM(float current_time);
    void Render();

    std::vector<glm::mat4> out_frames;

private:
    GLuint ebo;
    GLuint vbo;
    GLuint vao;

    iqmmesh                *meshes;
    iqmjoint               *joints;
    iqmtriangle            *tris;

    std::vector<glm::mat4x4> baseframe;
    std::vector<glm::mat4x4> inversebaseframe;
    std::vector<GLuint>    textures;

    std::vector<glm::mat4x4> frames;

    unsigned char          *buffer;
    int                    num_tris;
    int                    num_joints;
    int                    num_meshes;
    int                    num_frames;
};

} // namespace sp

#endif
