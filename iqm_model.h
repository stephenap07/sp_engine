#ifndef _SP_IQM_MODEL_H_
#define _SP_IQM_MODEL_H_

#include "iqm.h"

namespace sp {

class IQMModel {
public:
	~IQMModel();

	bool LoadModel(const char *filename);
	void Render();

private:
	GLuint ebo;
	GLuint vbo;
	GLuint vao;

	iqmmesh                  *meshes;
	iqmjoint                 *joints;
	iqmtriangle              *tris;

	std::vector<glm::mat3x4> baseframe;
	std::vector<glm::mat3x4> inversebaseframe;
	std::vector<GLuint>      textures;

	unsigned char            *buffer;
	int                      num_tris;
	int                      num_meshes;
};

}

#endif
