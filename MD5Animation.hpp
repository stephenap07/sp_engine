#ifndef SP_MD5_ANIMATION_H_
#define SP_MD5_ANIMATION_H_

#include <string>
#include <vector>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 

// From http://3dgep.com/loading-and-animating-md5-models-with-opengl/

class MD5Animation {
public:
	MD5Animation();
	virtual ~MD5Animation();

	bool LoadAnimation(const std::string &filename);
	void Update(float dt);
	void Render();

	struct JointInfo {
		std::string name;
		int         parent_id;
		int         flags;
		int         start_index;
	};
	typedef std::vector<JointInfo> JointInfoList;

	struct Bound {
		glm::vec3 min;
		glm::vec3 max;
	};
	typedef std::vector<Bound> BoundList;

	struct BaseFrame {
		glm::vec3 pos;
		glm::quat orient;
	};
	typedef std::vector<BaseFrame> BaseFrameList;

	struct FrameData {
		int id;
		std::vector<float> data;
	};
	typedef std::vector<FrameData> FrameDataList;

	struct SkeletonJoint {
		SkeletonJoint()
			:parent(-1),
			 pos(0)
		{}

		SkeletonJoint(const BaseFrame &copy)
			:pos(copy.pos),
			 orient(copy.orient)
		{}

		int       parent;
		glm::vec3 pos;
		glm::quat orient;
	};
	typedef std::vector<SkeletonJoint> SkeletonJointList;

	struct FrameSkeleton {
		SkeletonJointList joints;
	};
	typedef std::vector<FrameSkeleton> FrameSkeletonList;

	const FrameSkeleton& GetSkeleton() const
	{
		return animated_skeleton;
	}

	int GetNumJoints() const
	{
		return num_joints;
	}

	const JointInfo &GetJointInfo(unsigned int index) const
	{
		assert(index < joint_infos.size());
		return joint_infos[index];
	}

protected:
	JointInfoList     joint_infos;
	BoundList         bounds;
	BaseFrameList     base_frames;
	FrameDataList     frames;
	FrameSkeletonList skeletons;
	FrameSkeleton     animated_skeleton;

	void BuildFrameSkeleton(FrameSkeletonList &skeletons,
	                        const JointInfoList &joint_info,
	                        const BaseFrameList &base_frames,
	                        const FrameData &frame_data);

	void InterpolateSkeletons(FrameSkeleton &final_skeleton,
	                          const FrameSkeleton &skeleton0,
	                          const FrameSkeleton &skeleton1,
	                          float interpolate);

private:
	int md5_version;
	int num_frames;
	int num_joints;
	int frame_rate;
	int num_animated_components;

	float anim_duration;
	float frame_duration;
	float anim_time;
};

#endif
