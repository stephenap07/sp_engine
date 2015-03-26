#include <iostream>
#include <fstream>
#include <limits>
#include <vector>
#include <string>
#include <cmath>

#include <GL/glew.h>
#include <boost/filesystem.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MD5Model.hpp"

#include "Asset.hpp"

namespace fs = boost::filesystem;

static void ComputeQuatW(glm::quat &quat)
{
    float t = 1.0f - (quat.x * quat.x) - (quat.y * quat.y) - (quat.z * quat.z);
    if (t < 0.0f) {
        quat.w = 0.0f;
    } else {
        quat.w = -sqrtf(t);
    }
}

static void RemoveQuotes(std::string &str)
{
    size_t n;
    while (( n = str.find('\"')) != std::string::npos ) {
        str.erase(n, 1);
    }
}

MD5Model::MD5Model():
    md5_version(-1),
    num_joints(0),
    num_meshes(0),
    has_animation(false),
    model_mat4(1)
{}

MD5Model::~MD5Model()
{
    for (Mesh &mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.vao);
        glDeleteBuffers(1, &mesh.attr_buffer_id);
        glDeleteBuffers(1, &mesh.index_buffer_id);
    }
}

bool MD5Model::LoadModel(const std::string &filename)
{
    if (!fs::exists(filename)) {
        std::cerr << "MD5Model::LoadModel: Failed to load file " << filename << std::endl;
        return false;
    }

    fs::path file_path = filename;
    fs::path parent_path = file_path.parent_path();
    std::string param, junk;

    std::ifstream file(filename);
    uintmax_t file_length = fs::file_size(filename);
    assert(file_length > 0);

    joints.clear();
    meshes.clear();

    file >> param;
    while (!file.eof()) {
        if (param == "MD5Version") {
            file >> md5_version;
            assert (md5_version == 10);
        } else if (param == "commandline") {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else if (param == "numJoints") {
            file >> num_joints;
            joints.reserve(num_joints);
        } else if (param == "numMeshes") {
            file >> num_meshes;
            meshes.reserve(num_meshes);
        } else if (param == "joints") {
            Joint joint;
            file >> junk;
            for (int i = 0; i < num_joints; ++i) {
                file >> joint.name >> joint.parent_id >> junk
                    >> joint.pos.x >> joint.pos.y >> joint.pos.z >> junk >> junk
                    >> joint.orient.x >> joint.orient.y >> joint.orient.z >> junk;

                RemoveQuotes(joint.name);
                ComputeQuatW(joint.orient);
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                joints.push_back(joint);
            }
            file >> junk;
        } else if (param == "mesh") {
            Mesh mesh;
            int num_verts, num_tris, num_weights;
            file >> junk;
            file >> param;
            while (param != "}") {
                if (param == "shader") {
                    file >> mesh.shader;
                    RemoveQuotes(mesh.shader);

                    fs::path shader_path(mesh.shader);
                    fs::path texture_path;

                    if (shader_path.has_parent_path()) {
                        texture_path = shader_path;
                    } else {
                        texture_path = parent_path / shader_path;
                    }

                    if (!texture_path.has_extension()) {
                        texture_path.replace_extension(".tga");
                    }

                    mesh.tex_id = sp::MakeTexture(texture_path.string(), GL_TEXTURE_2D);

                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                } else if (param == "numverts") {
                    file >> num_verts;
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    for (int i = 0; i < num_verts; i++) {
                        Vertex vert;
                        file >> junk >> junk >> junk
                            >> vert.texture0.x  >> vert.texture0.y >> junk
                            >> vert.start_weight >> vert.weight_count;

                        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                        mesh.verts.push_back(vert);
                        mesh.tex2d_buffer.push_back(vert.texture0);
                    }
                } else if (param == "numtris") {
                    file >> num_tris;
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    for (int i = 0; i < num_tris; i++) {
                        Triangle tri;
                        // Turn counter-clockwise for BACK_FACE culling
                        file >> junk >> junk >> tri.indices[2] >> tri.indices[1] >> tri.indices[0];

                        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                        mesh.tris.push_back(tri);
                        mesh.index_buffer.push_back((GLuint)tri.indices[0]);
                        mesh.index_buffer.push_back((GLuint)tri.indices[1]);
                        mesh.index_buffer.push_back((GLuint)tri.indices[2]);
                    }
                } else if (param == "numweights") {
                    file >> num_weights;
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    for (int i = 0; i < num_weights; i++) {
                        Weight weight;
                        file >> junk >> junk >> weight.joint_id >> weight.bias >> junk
                            >> weight.pos.x >> weight.pos.y >> weight.pos.z >> junk;
                        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        // weight.pos.z = -weight.pos.z;	
                        mesh.weights.push_back(weight);
                    }
                } else {
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                file >> param;
            }

            PrepareMesh(mesh);
            PrepareNormals(mesh);
            PrepareBuffers(mesh);
            PrepareVAO(mesh);

            meshes.push_back(mesh);
        }

        file >> param;
    }

    assert((int)joints.size() == num_joints);
    assert((int)meshes.size() == num_meshes);

    return true;
}

bool MD5Model::LoadAnim(const std::string &filename)
{
    if (animation.LoadAnimation(filename)) {
        has_animation = CheckAnimation(animation);
    }

    return has_animation;
}

bool MD5Model::CheckAnimation(const MD5Animation &animation) const
{
    if (num_joints != animation.GetNumJoints()) {
        return false;
    }

    for (unsigned i = 0; i < joints.size(); i++) {
        const Joint &mesh_joint = joints[i];
        const MD5Animation::JointInfo &anim_joint = animation.GetJointInfo(i);
        if (mesh_joint.name != anim_joint.name ||
                mesh_joint.parent_id != anim_joint.parent_id) {
            return false;
        }
    }

    return true;
}

bool MD5Model::PrepareMesh(Mesh &mesh)
{
    mesh.position_buffer.clear();
    mesh.tex2d_buffer.clear();

    for (Vertex &vert : mesh.verts) {
        glm::vec3 final_pos(0);
        vert.pos = glm::vec3(0);
        vert.normal = glm::vec3(0);

        for (int i = 0; i < vert.weight_count; i++) {
            Weight &weight = mesh.weights[vert.start_weight + i];
            Joint &joint = joints[weight.joint_id];

            glm::vec3 rot_pos = joint.orient * weight.pos;
            vert.pos += (joint.pos + rot_pos) * weight.bias;
        }

        mesh.position_buffer.push_back(vert.pos);
        mesh.tex2d_buffer.push_back(vert.texture0);
    }

    return true;
}

bool MD5Model::PrepareMesh(Mesh &mesh, const MD5Animation::FrameSkeleton &skeleton)
{
    for (unsigned i = 0; i < mesh.verts.size(); i++) {
        const Vertex &vert = mesh.verts[i];
        glm::vec3 &pos = mesh.position_buffer[i];
        glm::vec3 &normal = mesh.normal_buffer[i];

        pos = glm::vec3(0);
        normal = glm::vec3(0);

        for (int j = 0; j < vert.weight_count; j++) {
            const Weight &weight = mesh.weights[vert.start_weight + j];
            const MD5Animation::SkeletonJoint &joint = skeleton.joints[weight.joint_id];

            glm::vec3 rot_pos = joint.orient * weight.pos;
            pos += (joint.pos + rot_pos) * weight.bias;
            normal += (joint.orient * vert.normal) * weight.bias;
        }
    }

    return true;
}

bool MD5Model::PrepareNormals(Mesh &mesh)
{
    mesh.normal_buffer.clear();

    for (Triangle &tri : mesh.tris) {
        glm::vec3 v0 = mesh.verts[tri.indices[0]].pos;
        glm::vec3 v1 = mesh.verts[tri.indices[1]].pos;
        glm::vec3 v2 = mesh.verts[tri.indices[2]].pos;

        glm::vec3 normal = glm::cross(v2 - v0, v1 - v0);

        mesh.verts[tri.indices[0]].normal += normal;
        mesh.verts[tri.indices[1]].normal += normal;
        mesh.verts[tri.indices[2]].normal += normal;
    }

    for (Vertex &vert : mesh.verts) {
        glm::vec3 normal = glm::normalize(vert.normal);
        mesh.normal_buffer.push_back(normal);
        vert.normal = glm::vec3(0);

        for (int i = 0; i < vert.weight_count; i++) {
            const Weight &weight = mesh.weights[vert.start_weight + i];
            const Joint &joint = joints[weight.joint_id];
            vert.normal += (normal * joint.orient) * weight.bias;
        }
    }

    return true;
}

void MD5Model::PrepareBuffers(Mesh &mesh)
{
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.attr_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.attr_buffer_id);

    size_t vec3_size = sizeof(glm::vec3);
    size_t vec2_size = sizeof(glm::vec2);

    glBufferData(GL_ARRAY_BUFFER,
            vec3_size * (mesh.position_buffer.size() +
                mesh.normal_buffer.size()) +
            (vec2_size * mesh.tex2d_buffer.size()) + 
            (vec3_size * 2 * mesh.position_buffer.size()),
            NULL, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MD5Model::PrepareVAO(Mesh &mesh)
{
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.attr_buffer_id);

    size_t vec3_size = sizeof(glm::vec3);
    size_t vec2_size = sizeof(glm::vec2);

    std::vector<glm::vec3> lines;

    for (size_t i = 0; i < mesh.position_buffer.size(); i++) {
        lines.push_back(mesh.position_buffer[i]);
        lines.push_back(mesh.position_buffer[i] + mesh.normal_buffer[i] * 2.0f);
    }

    size_t offset = 0;

    // Vertex Buffer Data
    glBufferSubData(GL_ARRAY_BUFFER,
            offset,
            vec3_size * mesh.position_buffer.size(),
            &mesh.position_buffer[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    offset += vec3_size * mesh.position_buffer.size();
    glEnableVertexAttribArray(0);

    // Normal Buffer Data
    glBufferSubData(GL_ARRAY_BUFFER,
            offset,
            vec3_size * mesh.normal_buffer.size(),
            &mesh.normal_buffer[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset);
    offset += vec3_size * mesh.normal_buffer.size();
    glEnableVertexAttribArray(1);

    // Texture Buffer Data
    glBufferSubData(GL_ARRAY_BUFFER,
            offset,
            vec2_size * mesh.tex2d_buffer.size(),
            &mesh.tex2d_buffer[0]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset);
    offset += vec2_size * mesh.tex2d_buffer.size();
    glEnableVertexAttribArray(2);

    // Normal Lines Buffer data
    glBufferSubData(GL_ARRAY_BUFFER,
            offset,
            vec3_size * mesh.position_buffer.size() * 2,
            &lines[0]);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset);
    glEnableVertexAttribArray(3);

    // Element Buffer Data
    glGenBuffers(1, &mesh.index_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer_id);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof(GLuint) * mesh.index_buffer.size(),
            &mesh.index_buffer[0],
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MD5Model::Render()
{
    for (Mesh &mesh : meshes) {
        RenderMesh(mesh);
    }
}

void MD5Model::RenderMesh(const Mesh &mesh)
{
    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.attr_buffer_id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh.tex_id);

    glDrawElements(GL_TRIANGLES, mesh.index_buffer.size(), GL_UNSIGNED_INT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MD5Model::RenderNormals()
{
    for (Mesh &mesh : meshes) {
        glBindVertexArray(mesh.vao);
        glDrawArrays(GL_LINES, 0, mesh.position_buffer.size() * 2);
        glBindVertexArray(0);
    }
}

void MD5Model::Update(float dt)
{
    if (has_animation) {
        animation.Update(dt);
        const MD5Animation::FrameSkeleton &skeleton = animation.GetSkeleton();
        for (Mesh &mesh : meshes) {
            PrepareMesh(mesh, skeleton);
            PrepareNormals(mesh);
            PrepareVAO(mesh);
        }
    }
}
