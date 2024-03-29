#include "GLTFLoader.h"
#include <iostream>
#include "Transform.h"
#include "Track.h"
#include <vector>

namespace GLTFHelpers
{
	cgltf_data* LoadGLTFFile(const char* path)
	{
		// 加载之前, 要创建一个cgltf_options类的对象
		cgltf_options options;
		memset(&options, 0, sizeof(cgltf_options));
		// 使用库文件, 把data和options都读取出来
		cgltf_data* data = NULL;
		// cgltf_result是个枚举
		cgltf_result result = cgltf_parse_file(&options, path, &data);

		// check
		if (result != cgltf_result_success)
		{
			std::cout << "Could not load input file: " << path << "\n";
			return 0;
		}

		// 根据options和path, 把数据读到data里, 这里的options和path传入的都是const 
		result = cgltf_load_buffers(&options, data, path);
		if (result != cgltf_result_success)
		{
			cgltf_free(data);
			std::cout << "Could not load buffers for: " << path << "\n";
			return 0;
		}

		// 再次check
		result = cgltf_validate(data);
		if (result != cgltf_result::cgltf_result_success)
		{
			cgltf_free(data);
			std::cout << "Invalid gltf file: " << path << "\n";
			return 0;
		}
		return data;
	}

	void FreeGLTFFile(cgltf_data* data)
	{
		if (data == 0)
			std::cout << "WARNING: Can't free null data\n";
		else
			cgltf_free(data);
	}

	// 这里默认认为所有的Node都代表一个Joint
	Transform GetLocalTransform(cgltf_node& node) 
	{
		Transform result;

		// gltf的节点可以用Transform或者矩阵两种方式来存储Transform数据
		// 如果有矩阵的信息, 则直接转为Transform
		if (node.has_matrix) 
		{
			mat4 mat(&node.matrix[0]);
			result = mat4ToTransform(mat);
		}

		if (node.has_translation) 
			result.position = vec3(node.translation[0], node.translation[1], node.translation[2]);

		if (node.has_rotation)
			result.rotation = quat(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);

		if (node.has_scale)
			result.scale = vec3(node.scale[0], node.scale[1], node.scale[2]);

		return result;
	}

	// 获取joint对应的node在gltf节点数组里的id, 其实就是线性遍历的查找
	// 类似于数组的FindIndexAt
	int GetNodeIndex(cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes) 
	{
		if (target == 0)
			return -1;
	
		for (unsigned int i = 0; i < numNodes; ++i) 
		{
			if (target == &allNodes[i]) 
				return (int)i;
		}

		return -1;
	}

	// 从gltf accessor里读取float数组的值
	void GetScalarValues(std::vector<float>& outScalars, unsigned int inComponentCount, const cgltf_accessor& inAccessor) 
	{
		outScalars.resize(inAccessor.count * inComponentCount);
		for (cgltf_size i = 0; i < inAccessor.count; ++i) 
			cgltf_accessor_read_float(&inAccessor, i, &outScalars[i * inComponentCount], inComponentCount);
	}

	// 核心函数, 从channel的sampler里获取Track, 其实只用到了channel的input, 没有用到output
	// This function does most of the heavy lifting. It converts a glTF animation channel into a
    // VectorTrack or a QuaternionTrack.
	// animation channel可以参考后面的附录, 本质上就是一个记录sampler和target joint引用的wrapper
	template<typename T, int N>
	void TrackFromChannel(Track<T, N>& inOutTrack, const cgltf_animation_channel& inChannel) 
	{
		// Sampler可以当作一个针对特定Property的有SampleAnimationClip功能的对象
		cgltf_animation_sampler& sampler = *inChannel.sampler;

		// 根据sampler获取
		Interpolation interpolation = Interpolation::Constant;
		if (inChannel.sampler->interpolation == cgltf_interpolation_type_linear) 
			interpolation = Interpolation::Linear;
		else if (inChannel.sampler->interpolation == cgltf_interpolation_type_cubic_spline) 
			interpolation = Interpolation::Cubic;
		bool isSamplerCubic = interpolation == Interpolation::Cubic;
		inOutTrack.SetInterpolation(interpolation);

		// 从sampler里获取俩数组, 一个代表关键帧的时间的float数组, 一个代表Property的关键帧的数组
		// 这俩数组的大小应该是一样的(说的是实际数据大小, 不是float的大小)
		std::vector<float> timelineFloats;
		GetScalarValues(timelineFloats, 1, *sampler.input);

		// output数组是已经在sampler里算好的
		// 如果是Constant和Linear的情况下, 它就是个Property的数组
		// 如果是Cubic情况下, 它是个Property、Property对应的mIn和mOut三个属性组成的对象的数组
		std::vector<float> valueFloats;
		GetScalarValues(valueFloats, N, *sampler.output);

		unsigned int numFrames = (unsigned int)sampler.input->count;
		// property由几个float组成
		unsigned int numberOfValuesPerFrame = valueFloats.size() / timelineFloats.size();
		inOutTrack.Resize(numFrames);
		// 遍历关键帧
		for (unsigned int i = 0; i < numFrames; ++i) 
		{
			int baseIndex = i * numberOfValuesPerFrame;
			// 获取frame的引用, 这里面的值应该是空的
			Frame<N>& frame = inOutTrack[i];
			int offset = 0;

			// sapmler的Input数组里获取每个关键帧的时间
			frame.mTime = timelineFloats[i];

			// 遍历Property的每个float component
			
			// 只有Cubic的采样情况下, 才需要mIn和mOut数据
			for (int component = 0; component < N; ++component) 
				frame.mIn[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
			
			for (int component = 0; component < N; ++component) 
				frame.mValue[component] = valueFloats[baseIndex + offset++];
			
			for (int component = 0; component < N; ++component) 
				frame.mOut[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
		}
	}
} // End of GLTFHelpers

// 接口一实现
// 借助GetLocalTransform和GetNodeIndex函数, 就可以读取出RestPose了
// 由于这里与动画系统逻辑相关, 所以不在GLTFHelpers的namespace里
// Pose的本质就是骨骼hierarchy和一个Transform数组, 与joint一一对应
Pose LoadRestPose(cgltf_data* data) 
{
	// 获取joint的个数
	unsigned int boneCount = (unsigned int)data->nodes_count;
	Pose result(boneCount);

	for (unsigned int i = 0; i < boneCount; ++i) 
	{
		cgltf_node* node = &(data->nodes[i]);

		// 所以读取的Node的默认的LocalTransform就是这里的Rest Pose的值
		Transform transform = GLTFHelpers::GetLocalTransform(data->nodes[i]);
		result.SetLocalTransform(i, transform);

		// 手动设置pose里的joint的父子关系
		int parent = GLTFHelpers::GetNodeIndex(node->parent, data->nodes, boneCount);
		result.SetParent(i, parent);
	}

	return result;
}

// 接口2: 遍历nodes数组, 取其name, 存到vector即可
std::vector<std::string> LoadJointNames(cgltf_data* data) 
{
	unsigned int boneCount = (unsigned int)data->nodes_count;
	std::vector<std::string> result(boneCount, "Not Set");

	for (unsigned int i = 0; i < boneCount; ++i) 
	{
		cgltf_node* node = &(data->nodes[i]);

		if (node->name == 0)
			result[i] = "EMPTY NODE";
	
		else
			result[i] = node->name;
	
	}

	return result;
}

// 接口3: 从data里遍历读取clip, 遍历里面的channel, 每个Joint对应Clip里的一个Joint的TransformTrack
// 从而可以一一填到我这边数据的Track里
std::vector<Clip> LoadAnimationClips(cgltf_data* data) 
{
	// 获取clips的数目
	unsigned int numClips = (unsigned int)data->animations_count;
	// 获取joints的数目
	unsigned int numNodes = (unsigned int)data->nodes_count;

	std::vector<Clip> result;
	result.resize(numClips);

	// 遍历Clip数据
	for (unsigned int i = 0; i < numClips; ++i) 
	{
		// data->animations相当于clips的数组, 这里就是挖数据了
		result[i].SetName(data->animations[i].name);

		// 遍历clips里的每个Channel, 前面提过了channel是个通道, 一端是一个property对应Curve的sampler
		// 另一端是apply property的joint, 每个Channel对应动画Clip里的一条Property Curve
		unsigned int numChannels = (unsigned int)data->animations[i].channels_count;
		// 遍历所有curve
		for (unsigned int j = 0; j < numChannels; ++j) 
		{
			cgltf_animation_channel& channel = data->animations[i].channels[j];
			// channel的Output是Joint Node
			cgltf_node* target = channel.target_node;
			int nodeId = GLTFHelpers::GetNodeIndex(target, data->nodes, numNodes);

			// 看Curve是Transform的哪种类型
			if (channel.target_path == cgltf_animation_path_type_translation) 
			{
				// 获取对应clip的joint的nodeId对应的joint的TransformTrack
				// 之前实现过Clip类的[]重载, 会返回一个TransformTrack&
				// 这里获取的track应该还没有数据
				VectorTrack& track = result[i][nodeId].GetPositionTrack();
				// 使用Helper函数, 把这个channel里的数据提取出来, 把时间数组和Property数组
				// 甚至(mIn和mOut数组)存到track里
				GLTFHelpers::TrackFromChannel<vec3, 3>(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_scale) 
			{
				VectorTrack& track = result[i][nodeId].GetScaleTrack();
				GLTFHelpers::TrackFromChannel<vec3, 3>(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_rotation) 
			{
				QuaternionTrack& track = result[i][nodeId].GetRotationTrack();
				GLTFHelpers::TrackFromChannel<quat, 4>(track, channel);
			}
		}
		result[i].RecalculateDuration();
	}

	return result;
}

