#ifndef _SP_IQM_MODEL_H_
#define _SP_IQM_MODEL_H_

#include "VertexBuffer.hpp"
#include "IQM.hpp"

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
    IQMModel() : meshes(nullptr), joints(nullptr), tris(nullptr), buffer(nullptr),
    current_skeleton_id(0), num_tris(0), num_joints(0), num_meshes(0), num_frames(0)
    {}
    ~IQMModel();

    bool LoadModel(const char *filename);
    void Animate(float current_time);
    void Render();
    std::vector<glm::mat4> &GetBones();

    VertexBuffer v_buffer;
private:

    std::vector<glm::mat4x4> baseframe;
    std::vector<glm::mat4x4> inversebaseframe;
    std::vector<glm::mat4x4> frames;
    std::vector<Skeleton> skeletons;
    std::vector<GLuint> textures;

    IQMMesh *meshes;
    IQMJoint *joints;
    IQMTriangle *tris;
    unsigned char *buffer;

    int current_skeleton_id;
    int num_tris;
    int num_joints;
    int num_meshes;
    int num_frames;
};

} // namespace sp

#endif
