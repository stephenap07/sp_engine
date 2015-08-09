#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/filesystem.hpp>

#include "MD5Animation.hpp"

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
    while ((n = str.find('\"')) != std::string::npos) {
        str.erase(n, 1);
    }
}

static glm::vec3 lerp(const glm::vec3 &a, const glm::vec3 &b, float dt)
{
    return a + (b - a) * dt;
}

MD5Animation::MD5Animation()
    : md5_version(-1), num_frames(0), num_joints(0), frame_rate(0),
      num_animated_components(0), anim_duration(0.0f), frame_duration(0.0f),
      anim_time(0.0f)
{
}

MD5Animation::~MD5Animation() {}

bool MD5Animation::LoadAnimation(const std::string &filename)
{
    if (!fs::exists(filename)) {
        std::cerr << "MD5Animation::LoadAnimation: Failed to find file: "
                  << filename << std::endl;
    }

    std::string param, junk;

    std::ifstream file(filename);
    int file_length = fs::file_size(filename);
    assert(file_length > 0);

    joint_infos.clear();
    bounds.clear();
    base_frames.clear();
    frames.clear();
    animated_skeleton.joints.clear();
    num_frames = 0;

    file >> param;

    while (!file.eof()) {
        if (param == "MD5Version") {
            file >> md5_version;
            assert(md5_version == 10);
        } else if (param == "commandline") {
            file.ignore(file_length, '\n');
        } else if (param == "numFrames") {
            file >> num_frames;
            file.ignore(file_length, '\n');
        } else if (param == "numJoints") {
            file >> num_joints;
            file.ignore(file_length, '\n');
        } else if (param == "frameRate") {
            file >> frame_rate;
            file.ignore(file_length, '\n');
        } else if (param == "numAnimatedComponents") {
            file >> num_animated_components;
            file.ignore(file_length, '\n');
        } else if (param == "hierarchy") {
            file >> junk;
            for (int i = 0; i < num_joints; i++) {
                JointInfo joint;
                file >> joint.name >> joint.parent_id >> joint.flags >>
                    joint.start_index;
                RemoveQuotes(joint.name);
                joint_infos.push_back(joint);
                file.ignore(file_length, '\n');
            }
            file >> junk;
        } else if (param == "bounds") {
            file >> junk;
            file.ignore(file_length, '\n');
            for (int i = 0; i < num_frames; i++) {
                Bound bound;
                file >> junk;
                file >> bound.min.x >> bound.min.y >> bound.min.z;
                file >> junk >> junk;
                file >> bound.max.x >> bound.max.y >> bound.max.z;
                // bound.min.z = -bound.min.z;
                // bound.max.z = -bound.max.z;
                bounds.push_back(bound);
                file.ignore(file_length, '\n');
            }
            file >> junk;
            file.ignore(file_length, '\n');
        } else if (param == "baseframe") {
            file >> junk;
            file.ignore(file_length, '\n');
            for (int i = 0; i < num_joints; i++) {
                BaseFrame base_frame;
                file >> junk;
                file >> base_frame.pos.x >> base_frame.pos.y >>
                    base_frame.pos.z;
                file >> junk >> junk;
                file >> base_frame.orient.x >> base_frame.orient.y >>
                    base_frame.orient.z;
                file.ignore(file_length, '\n');
                // base_frame.pos.z = -base_frame.pos.z;
                // base_frame.orient.z = -base_frame.orient.z;
                base_frames.push_back(base_frame);
            }
            file >> junk;
            file.ignore(file_length, '\n');
        } else if (param == "frame") {
            FrameData frame;
            file >> frame.id >> junk;
            file.ignore(file_length, '\n');
            for (int i = 0; i < num_animated_components; i++) {
                float frame_data;
                file >> frame_data;
                frame.data.push_back(frame_data);
            }
            frames.push_back(frame);
            BuildFrameSkeleton(skeletons, joint_infos, base_frames, frame);
            file >> junk;
            file.ignore(file_length, '\n');
        }
        file >> param;
    }

    animated_skeleton.joints.assign(num_joints, SkeletonJoint());

    frame_duration = 1.0f / (float)frame_rate;
    anim_duration = (frame_duration * (float)num_frames);
    anim_time = 0.0f;

    assert(joint_infos.size() == (unsigned)num_joints);
    assert(bounds.size() == (unsigned)num_frames);
    assert(base_frames.size() == (unsigned)num_joints);
    assert(frames.size() == (unsigned)num_frames);
    assert(skeletons.size() == (unsigned)num_frames);

    return true;
}

void MD5Animation::BuildFrameSkeleton(FrameSkeletonList &skeletons,
                                      const JointInfoList &joint_infos,
                                      const BaseFrameList &base_frames,
                                      const FrameData &frame_data)
{
    FrameSkeleton skeleton;
    for (unsigned i = 0; i < joint_infos.size(); i++) {
        unsigned j = 0;
        const JointInfo &joint_info = joint_infos[i];
        SkeletonJoint animated_joint = base_frames[i];
        animated_joint.parent = joint_info.parent_id;

        if (joint_info.flags & 1) {
            animated_joint.pos.x =
                frame_data.data[joint_info.start_index + j++];
        }
        if (joint_info.flags & 2) {
            animated_joint.pos.y =
                frame_data.data[joint_info.start_index + j++];
        }
        if (joint_info.flags & 4) {
            animated_joint.pos.z =
                frame_data.data[joint_info.start_index + j++];
        }
        if (joint_info.flags & 8) {
            animated_joint.orient.x =
                frame_data.data[joint_info.start_index + j++];
        }
        if (joint_info.flags & 16) {
            animated_joint.orient.y =
                frame_data.data[joint_info.start_index + j++];
        }
        if (joint_info.flags & 32) {
            animated_joint.orient.z =
                frame_data.data[joint_info.start_index + j++];
        }

        ComputeQuatW(animated_joint.orient);

        if (animated_joint.parent >= 0) {
            SkeletonJoint &parent_joint =
                skeleton.joints[animated_joint.parent];
            glm::vec3 rot_pos = parent_joint.orient * animated_joint.pos;

            animated_joint.pos = parent_joint.pos + rot_pos;
            animated_joint.orient = parent_joint.orient * animated_joint.orient;
            animated_joint.orient = glm::normalize(animated_joint.orient);
        }

        skeleton.joints.push_back(animated_joint);
    }

    skeletons.push_back(skeleton);
}

void MD5Animation::Update(float dt)
{
    if (num_frames < 1)
        return;

    anim_time += dt;

    while (anim_time > anim_duration)
        anim_time -= anim_duration;
    while (anim_time < 0.0f)
        anim_time += anim_duration;

    float frame_num = anim_time * (float)frame_rate;
    int frame0 = (int)floorf(frame_num) % num_frames;
    int frame1 = (int)ceilf(frame_num) % num_frames;

    float interpolate = fmodf(anim_time, frame_duration) / frame_duration;
    InterpolateSkeletons(animated_skeleton, skeletons[frame0],
                         skeletons[frame1], interpolate);
}

void MD5Animation::InterpolateSkeletons(FrameSkeleton &final_skeleton,
                                        const FrameSkeleton &skeleton0,
                                        const FrameSkeleton &skeleton1,
                                        float interpolate)
{
    for (int i = 0; i < num_joints; i++) {
        SkeletonJoint &final_joint = final_skeleton.joints[i];
        const SkeletonJoint &joint0 = skeleton0.joints[i];
        const SkeletonJoint &joint1 = skeleton1.joints[i];

        final_joint.parent = joint0.parent;
        final_joint.pos = lerp(joint0.pos, joint1.pos, interpolate);
        final_joint.orient =
            glm::mix(joint0.orient, joint1.orient, interpolate);
    }
}

void MD5Animation::Render() {}
