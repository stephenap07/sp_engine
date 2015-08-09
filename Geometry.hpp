#ifndef _SP_GEOMETRY_H_
#define _SP_GEOMETRY_H_

namespace sp
{

class VertexBuffer;

void MakeCube(VertexBuffer *buffer, bool has_normals = false);
void MakeQuad(VertexBuffer *buffer);
void MakeTexturedQuad(VertexBuffer *buffer, GLuint gl_hint = GL_STATIC_DRAW);

} // namespace sp

#endif
