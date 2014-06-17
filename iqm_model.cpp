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
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace sp {

//------------------------------------------------------------------------------

void ErrorLog(const char* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
}

struct Vertex {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat tangent[4];
    GLfloat texcoord[2];
    GLubyte blendindex[4];
    GLubyte blendweight[4];
};

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
		ErrorLog("MD5Model::LoadModel: Failed to load file %s\n", filename);
		return false;
	}

	iqmheader header;

	std::FILE *file = std::fopen(filename, "rb");

	if (std::fread(&header, sizeof(header), 1, file) != 1 ||
	      memcmp(header.magic, IQM_MAGIC, sizeof(header.magic))) {
		ErrorLog("Error loading file header for %s", filename);	
		return false;
	}

    lilswap(&header.version, (sizeof(header) - sizeof(header.magic))/sizeof(uint));

	assert(header.version == IQM_VERSION);
	assert(header.filesize < (16 << 20));

	num_tris = header.num_triangles;
	num_meshes = header.num_meshes;

	buffer = new unsigned char[header.filesize];
	size_t n = std::fread(buffer + sizeof(header), header.filesize - sizeof(header), 1, file);
	if (n != 1) {
		ErrorLog("Error loading file buffer for %s", filename);	
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
			if(vert.format != IQM_FLOAT || vert.size != 3) {
				return false;
			}
			inposition = (float *)&buffer[vert.offset];
			lilswap(inposition, 3*header.num_vertexes);
			break;
        case IQM_NORMAL:
			if(vert.format != IQM_FLOAT || vert.size != 3) {
				return false;
			} innormal = (float *)&buffer[vert.offset];
			lilswap(innormal, 3*header.num_vertexes);
			break;
        case IQM_TANGENT:
			if(vert.format != IQM_FLOAT || vert.size != 4) {
				return false;
			}
			intangent = (float *)&buffer[vert.offset];
			lilswap(intangent, 4*header.num_vertexes);
			break;
        case IQM_TEXCOORD:
			if(vert.format != IQM_FLOAT || vert.size != 2) {
				return false;
			}
			intexcoord = (float *)&buffer[vert.offset];
			lilswap(intexcoord, 2*header.num_vertexes);
			break;
        case IQM_BLENDINDEXES:
			if(vert.format != IQM_UBYTE || vert.size != 4) {
				return false;
			}
			inblendindex = (uchar *)&buffer[vert.offset];
			break;
        case IQM_BLENDWEIGHTS:
			if(vert.format != IQM_UBYTE || vert.size != 4) {
				return false;
			}
			inblendweight = (uchar *)&buffer[vert.offset];
			break;
        }
    }

	meshes = (iqmmesh  *)&buffer[header.ofs_meshes];
	joints = (iqmjoint *)&buffer[header.ofs_joints];
	tris   = (iqmtriangle *)&buffer[header.ofs_triangles];

	baseframe.reserve(header.num_joints);
	inversebaseframe.reserve(header.num_joints);
	textures.reserve(header.num_meshes);

	/*
    for(int i = 0; i < (int)header.num_joints; i++)
    {
        iqmjoint &j = joints[i];
        //baseframe[i] = glm::mat3x4(glm::normalize(glm::quat(j.rotate)), glm::vec3(j.translate), glm::vec3(j.scale));
		// inversebaseframe[i] = glm::invert(baseframe[i]);
        if(j.parent >= 0) 
        {
            // baseframe[i] = baseframe[j.parent] * baseframe[i];
            // inversebaseframe[i] *= inversebaseframe[j.parent];
        }
    }
	*/

	// Assuming the materials are in the same directory as the iqm file
	fs::path file_path   = filename;
	fs::path parent_path = file_path.parent_path();

    for(int i = 0; i < (int)header.num_meshes; i++)
    {
		iqmmesh &m = meshes[i];
		fs::path mat_path(&str[m.material]);
		fs::path texture_path = parent_path / mat_path;

		if (!fs::is_regular_file(texture_path)) {
			ErrorLog("%s: There is no material for this file\n", filename, texture_path.string().c_str());
		} else {
			textures[i] = MakeTexture(texture_path.string());
			if (!textures[i]) {
				ErrorLog("%s: Failed to load texture %s\n", filename, texture_path.string().c_str());
			}
		}
    }

    Vertex verts[header.num_vertexes];
    memset(verts, 0, header.num_vertexes*sizeof(Vertex));
    for(int i = 0; i < (int)header.num_vertexes; i++)
    {
        Vertex &v = verts[i];

        if(inposition)
			memcpy(v.position, &inposition[i*3], sizeof(v.position));
        if(innormal)
			memcpy(v.normal, &innormal[i*3], sizeof(v.normal));
        if(intangent)
			memcpy(v.tangent, &intangent[i*4], sizeof(v.tangent));
        if(intexcoord)
			memcpy(v.texcoord, &intexcoord[i*2], sizeof(v.texcoord));
        if(inblendindex)
			memcpy(v.blendindex, &inblendindex[i*4], sizeof(v.blendindex));
        if(inblendweight)
			memcpy(v.blendweight, &inblendweight[i*4], sizeof(v.blendweight));
    }

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ebo);
	glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, header.num_vertexes*sizeof(Vertex), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, header.num_triangles*sizeof(iqmtriangle), tris, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	// Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
	// Tangent
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
	// Texcoord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(10 * sizeof(GLfloat)));
	// Blend Index
	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (GLvoid*)(12 * sizeof(GLfloat)));
	// Blend Weight
	glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (GLvoid*)(12 * sizeof(GLfloat) + 4 * sizeof(GLubyte)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

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
