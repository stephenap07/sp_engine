#ifndef _SP_IQM_MODEL_H_
#define _SP_IQM_MODEL_H_

#include "iqm.h"
#include "buffer.h"

namespace sp {

struct MeshTri {
    unsigned int vertex[3];
};

struct Skeleton {
    std::vector<glm::mat4> frames;
};

struct Mesh {
    GLuint texture_id;
    std::vector<MeshTri> vertices; 
};

class IQMModel {
public:
    ~IQMModel();

    bool LoadModel(const char *filename);
    void Animate(float current_time);
    void Render();
    std::vector<glm::mat4> &GetBones();

private:
    VertexBuffer v_buffer;

    IQMMesh *meshes;
    IQMJoint *joints;
    IQMTriangle *tris;

    std::vector<glm::mat4x4> baseframe;
    std::vector<glm::mat4x4> inversebaseframe;
    std::vector<GLuint> textures;
    std::vector<glm::mat4x4> frames;

    std::vector<Skeleton> skeletons;
    int current_skeleton_id;

    unsigned char *buffer;
    int num_tris;
    int num_joints;
    int num_meshes;
    int num_frames;
};

} // namespace sp

#endif
