#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <cassert>
#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/filesystem.hpp>

#include "util.h"
#include "iqm_model.h"
#include "asset.h"
#include "logger.h"
#include "shader.h"

namespace fs = boost::filesystem;

namespace sp {

//------------------------------------------------------------------------------

static glm::mat4 MakeBoneMat(glm::quat rot, glm::vec3 trans, glm::vec3 scale)
{
    glm::mat4 rotation_mat = glm::mat4_cast(glm::normalize(rot));
    glm::mat4 scale_mat = glm::scale(scale);
    glm::mat4 trans_mat = glm::translate(trans);

    glm::mat4 out = trans_mat * rotation_mat * scale_mat;

    return out;
}

//------------------------------------------------------------------------------

IQMModel::~IQMModel()
{
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (ebo)
        glDeleteBuffers(1, &ebo);

    delete buffer;
}

//------------------------------------------------------------------------------

bool IQMModel::LoadModel(const char *filename)
{
    if (!fs::exists(filename)) {
        log::ErrorLog("IQMModel::LoadModel: Failed to load file %s\n", filename);
        return false;
    }

    iqmheader header;

    std::FILE *file = std::fopen(filename, "rb");

    if (std::fread(&header, sizeof(header), 1, file) != 1 ||
            memcmp(header.magic, IQM_MAGIC, sizeof(header.magic))) {
        log::ErrorLog("Error loading file header for %s", filename);	
        return false;
    }

    lilswap(&header.version, (sizeof(header) - sizeof(header.magic))/sizeof(uint));

    assert(header.version == IQM_VERSION);
    assert(header.filesize < (16 << 20));

    num_tris = header.num_triangles;
    num_meshes = header.num_meshes;
    num_joints = header.num_joints;

    buffer = new unsigned char[header.filesize];
    size_t n = std::fread(buffer + sizeof(header), header.filesize - sizeof(header), 1, file);
    if (n != 1) {
        log::ErrorLog("Error loading file buffer for %s", filename);	
        return false;
    }

    std::fclose(file);

    lilswap((uint *)&buffer[header.ofs_vertexarrays],
            header.num_vertexarrays*sizeof(iqmvertexarray)/sizeof(uint));
    lilswap((uint *)&buffer[header.ofs_triangles],
            header.num_triangles*sizeof(iqmtriangle)/sizeof(uint));
    lilswap((uint *)&buffer[header.ofs_meshes],
            header.num_meshes*sizeof(iqmmesh)/sizeof(uint));
    lilswap((uint *)&buffer[header.ofs_joints],
            header.num_joints*sizeof(iqmjoint)/sizeof(uint));

    float *inposition    = NULL;
    float *innormal      = NULL;
    float *intangent     = NULL;
    float *intexcoord    = NULL;
    uchar *inblendindex  = NULL;
    uchar *inblendweight = NULL;

    const char *str = header.ofs_text ? (char *)&buffer[header.ofs_text] : "";

    iqmvertexarray *vertices= (iqmvertexarray *)&buffer[header.ofs_vertexarrays];

    for(int i = 0; i < (int)header.num_vertexarrays; i++)
    {
        iqmvertexarray &vert = vertices[i];
        switch(vert.type)
        {
        case IQM_POSITION:
            if (vert.format != IQM_FLOAT || vert.size != 3) {
                return false;
            }
            inposition = (float *)&buffer[vert.offset];
            lilswap(inposition, 3 * header.num_vertexes);
            break;
        case IQM_NORMAL:
            if (vert.format != IQM_FLOAT || vert.size != 3) {
                return false;
            } innormal = (float *)&buffer[vert.offset];
            lilswap(innormal, 3 * header.num_vertexes);
            break;
        case IQM_TANGENT:
            if (vert.format != IQM_FLOAT || vert.size != 4) {
                return false;
            }
            intangent = (float *)&buffer[vert.offset];
            lilswap(intangent, 4 * header.num_vertexes);
            break;
        case IQM_TEXCOORD:
            if (vert.format != IQM_FLOAT || vert.size != 2) {
                return false;
            }
            intexcoord = (float *)&buffer[vert.offset];
            lilswap(intexcoord, 2 * header.num_vertexes);
            break;
        case IQM_BLENDINDEXES:
            if (vert.format != IQM_UBYTE || vert.size != 4) {
                return false;
            }
            inblendindex = (uchar *)&buffer[vert.offset];
            break;
        case IQM_BLENDWEIGHTS:
            if (vert.format != IQM_UBYTE || vert.size != 4) {
                return false;
            }
            inblendweight = (uchar *)&buffer[vert.offset];
            break;
        }
    }

    meshes = (iqmmesh *)&buffer[header.ofs_meshes];
    joints = (iqmjoint *)&buffer[header.ofs_joints];
    tris   = (iqmtriangle *)&buffer[header.ofs_triangles];

    baseframe.resize(header.num_joints);
    inversebaseframe.resize(header.num_joints);
    textures.resize(header.num_meshes);

    for(int i = 0; i < (int)header.num_joints; i++)
    {
        iqmjoint &j = joints[i];

        glm::quat rot_q = glm::quat(j.rotate[3], j.rotate[0], j.rotate[1], j.rotate[2]);
        glm::vec3 trans = glm::vec3(j.translate[0], j.translate[1], j.translate[2]);
        glm::vec3 scale = glm::vec3(j.scale[0], j.scale[1], j.scale[2]);

        baseframe[i] = MakeBoneMat(rot_q, trans, scale);
        inversebaseframe[i] = glm::inverse(baseframe[i]);

        if(j.parent >= 0) 
        {
            baseframe[i] = baseframe[j.parent] * baseframe[i];
            inversebaseframe[i] *= inversebaseframe[j.parent];
        }
    }

    // Assuming the materials are in the same directory as the iqm file
    fs::path file_path   = filename;
    fs::path parent_path = file_path.parent_path();

    for(int i = 0; i < (int)header.num_meshes; i++)
    {
        iqmmesh &mesh = meshes[i];
        fs::path mat_path(&str[mesh.material]);
        fs::path texture_path = parent_path / mat_path;

        if (!texture_path.has_extension()) {
            texture_path.replace_extension(".tga");
        }

        log::InfoLog("Loaded mesh: %s\n", &str[mesh.name]);

        if (!fs::is_regular_file(texture_path)) {
            log::ErrorLog("%s: There is no material for mesh %s called %s\n",
                          filename, &str[mesh.name],
                          texture_path.string().c_str());
        } else {
            textures[i] = MakeTexture(texture_path.string(), GL_TEXTURE_2D);
            if (!textures[i]) {
                log::ErrorLog("%s: Failed to load texture %s\n",
                              filename, texture_path.string().c_str());
            }
        }
    }

    if (header.num_anims > 0) {
        if ((int)header.num_poses != (int)header.num_joints) {
           return false;
        }

        lilswap((uint *)&buffer[header.ofs_poses], header.num_poses*sizeof(iqmpose)/sizeof(uint));
        lilswap((uint *)&buffer[header.ofs_anims], header.num_anims*sizeof(iqmanim)/sizeof(uint));
        lilswap((ushort *)&buffer[header.ofs_frames], header.num_frames*header.num_framechannels);

        iqmanim *anims     = (iqmanim *)&buffer[header.ofs_anims];
        iqmpose *poses     = (iqmpose *)&buffer[header.ofs_poses];
        ushort  *framedata = (ushort *)&buffer[header.ofs_frames];

        num_frames = header.num_frames;
        frames.resize(header.num_frames * header.num_poses);
        out_frames.resize(header.num_joints);

        for(int i = 0; i < (int)header.num_frames; i++)
        {
            for(int j = 0; j < (int)header.num_poses; j++)
            {
                iqmpose &p = poses[j];
                glm::quat rotate;
                glm::vec3 translate, scale;

                translate.x = p.channeloffset[0]; if(p.mask&0x01) translate.x += *framedata++ * p.channelscale[0];
                translate.y = p.channeloffset[1]; if(p.mask&0x02) translate.y += *framedata++ * p.channelscale[1];
                translate.z = p.channeloffset[2]; if(p.mask&0x04) translate.z += *framedata++ * p.channelscale[2];

                rotate.x = p.channeloffset[3]; if(p.mask&0x08) rotate.x += *framedata++ * p.channelscale[3];
                rotate.y = p.channeloffset[4]; if(p.mask&0x10) rotate.y += *framedata++ * p.channelscale[4];
                rotate.z = p.channeloffset[5]; if(p.mask&0x20) rotate.z += *framedata++ * p.channelscale[5];
                rotate.w = p.channeloffset[6]; if(p.mask&0x40) rotate.w += *framedata++ * p.channelscale[6];

                scale.x = p.channeloffset[7]; if(p.mask&0x80) scale.x += *framedata++ * p.channelscale[7];
                scale.y = p.channeloffset[8]; if(p.mask&0x100) scale.y += *framedata++ * p.channelscale[8];
                scale.z = p.channeloffset[9]; if(p.mask&0x200) scale.z += *framedata++ * p.channelscale[9];

                glm::mat4 m = MakeBoneMat(rotate, translate, scale);

                if (p.parent >= 0) {
                    frames[i * header.num_poses + j] = baseframe[p.parent] * m * inversebaseframe[j];
                } else {
                    frames[i * header.num_poses + j] = m * inversebaseframe[j];
                }
            }
        }

        for(int i = 0; i < (int)header.num_anims; i++)
        {
            iqmanim &a = anims[i];
            log::InfoLog("%s: loaded anim: %s\n", filename, &str[a.name]);
        }
    }

    Vertex verts[header.num_vertexes];
    memset(verts, 0, header.num_vertexes*sizeof(Vertex));
    for(int i = 0; i < (int)header.num_vertexes; i++)
    {
        Vertex &v = verts[i];

        if(inposition) {
            memcpy(v.position, &inposition[i*3], sizeof(v.position));
        }
        if(innormal) {
            memcpy(v.normal, &innormal[i*3], sizeof(v.normal));
        }
        if(intangent) {
            memcpy(v.tangent, &intangent[i*4], sizeof(v.tangent));
        }
        if(intexcoord) {
            memcpy(v.texcoord, &intexcoord[i*2], sizeof(v.texcoord));
        }
        if(inblendindex) {
            memcpy(v.blendindex, &inblendindex[i*4], sizeof(v.blendindex));
        }
        if(inblendweight) {
            memcpy(v.blendweight, &inblendweight[i*4], sizeof(v.blendweight));
        }
    }

    // Abstract out ogl buffer calls?

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ebo);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, header.num_vertexes*sizeof(Vertex), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, header.num_triangles*sizeof(iqmtriangle), tris, GL_STATIC_DRAW);

    SetVertAttribPointers();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

//------------------------------------------------------------------------------

void IQMModel::AnimateIQM(float current_time)
{
    if (!num_frames) {
        return;
    }

    int frame1 = (int)floor(current_time);
    int frame2 = frame1 + 1;

    float frame_offset = current_time - frame1;

    frame1 %= num_frames;
    frame2 %= num_frames;

    glm::mat4 *mat0 = &frames[frame1 * num_joints];
    glm::mat4 *mat1 = &frames[frame2 * num_joints];

    for(int i = 0; i < num_joints; i++)
    {
        glm::mat4 mat = (1 - frame_offset)*mat0[i] + frame_offset*mat1[i];

        if(joints[i].parent >= 0) {
            out_frames[i] = out_frames[joints[i].parent] * mat;
        } else {
            out_frames[i] = mat;
        }
    }
}

//------------------------------------------------------------------------------

void IQMModel::Render()
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glActiveTexture(GL_TEXTURE0);

    for(int i = 0; i < num_meshes; i++)
    {
        iqmmesh &m = meshes[i];
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glDrawElements(GL_TRIANGLES, 3 * m.num_triangles, GL_UNSIGNED_INT, (GLvoid*)(m.first_triangle * sizeof(iqmtriangle)));
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

} // namespace sp
