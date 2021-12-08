#ifndef _H_GLTFLOADER_
#define _H_GLTFLOADER_

#include "vendor/cgltf.h"
#include "Pose.h"
#include "Clip.h"
#include <vector>
#include <string>
#include "Skeleton.h"

namespace GLTFHelpers
{
	// 这是之前已经加好的接口
	cgltf_data* LoadGLTFFile(const char* path);
	void FreeGLTFFile(cgltf_data* handle);
}

// 新加的接口一: 从data里读取rest pose
Pose LoadRestPose(cgltf_data* data);
// 新加的接口二: 从data里读取skeleton, 每个节点的数据用string表示name
std::vector<std::string> LoadJointNames(cgltf_data* data);
// 新加的接口三: 从data里读取AnimationClips
std::vector<Clip> LoadAnimationClips(cgltf_data* data);
// 新加的接口四: 从data里读取bind pose
Pose LoadBindPose(cgltf_data* data);

Skeleton LoadSkeleton(cgltf_data* data);

#endif
