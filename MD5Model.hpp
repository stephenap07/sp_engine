#ifndef SP_MD5_MODEL_H_
#define SP_MD5_MODEL_H_

#include "MD5Animation.hpp"

class MD5Model {

public:
	MD5Model();
	virtual ~MD5Model();

	bool LoadModel(const std::string &filename);
	bool LoadAnim(const std::string &filename);
	void Update(float dt);
	void Render();
	void RenderNormals();

protected:
	typedef std::vector<glm::vec3> PositionBuffer;
	typedef std::vector<glm::vec3> NormalBuffer;
	typedef std::vector<glm::vec2> Tex2DBuffer;
	typedef std::vector<GLuint> IndexBuffer;

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texture0;
		int       start_weight;
		int       weight_count;
	};
	typedef std::vector<Vertex> VertexList;

	struct Triangle {
		int indices[3];		
	};
	typedef std::vector<Triangle> TriangleList;

	struct Weight {
		int       joint_id;
		float     bias;
		glm::vec3 pos;
	};
	typedef std::vector<Weight> WeightList;

	struct Joint {
		std::string name;
		int         parent_id;
		glm::vec3   pos;
		glm::quat   orient;
	};
	typedef std::vector<Joint> JointList;

	struct Mesh {
		std::string  shader;
		VertexList   verts;
		TriangleList tris;
		WeightList   weights;

		GLuint		   vao;
		GLuint		   attr_buffer_id;
		GLuint		   index_buffer_id;
		GLuint         tex_id;

		PositionBuffer position_buffer;
		NormalBuffer   normal_buffer;
		Tex2DBuffer    tex2d_buffer;
		IndexBuffer    index_buffer;
	};

	typedef std::vector<Mesh> MeshList;

	bool PrepareMesh(Mesh &mesh);
	bool PrepareMesh(Mesh &mesh, const MD5Animation::FrameSkeleton& skel);
	bool PrepareNormals(Mesh &mesh);
	void PrepareBuffers(Mesh &mesh);
	void PrepareVAO(Mesh &mesh);

	void RenderMesh(const Mesh &mesh);

	void RenderSkeleton(const JointList &joints);

	bool CheckAnimation(const MD5Animation &animation) const;

private:
	int         md5_version;
	int         num_joints;
	int         num_meshes;
	bool        has_animation;
	JointList   joints;
	MeshList    meshes;
	MD5Animation animation;
	glm::mat4x4 model_mat4;
};

#endif // SP_MD5_MODEL_H_
