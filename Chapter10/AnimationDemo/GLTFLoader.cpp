#include "GLTFLoader.h"
#include <iostream>
#include "Transform.h"
#include "vec2.h"
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
		size_t numberOfValuesPerFrame = valueFloats.size() / timelineFloats.size();
		inOutTrack.Resize(numFrames);
		// 遍历关键帧
		for (unsigned int i = 0; i < numFrames; ++i)
		{
			size_t baseIndex = i * numberOfValuesPerFrame;
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

	// 针对glTF文件里的每个subMesh(primitive)的每个attribute, 调用此函数, 填充到Mesh对应的顶点属性的数组里
	// nodes是glTF里的所有Nodes的数组
	// attribute是一个node对应mesh上的其中一个primitive的attribute数组
	// skin是一个node对应的skin
	void MeshFromAttribute(Mesh& outMesh, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, unsigned int nodeCount)
	{
		// 获取顶点属性对应的类型
		cgltf_attribute_type attribType = attribute.type;
		// 获取accessor
		cgltf_accessor& accessor = *attribute.data;

		// 判断attribute的数据由几个float组成
		unsigned int componentCount = 0;
		if (accessor.type == cgltf_type_vec2)
			componentCount = 2;
		else if (accessor.type == cgltf_type_vec3)
			componentCount = 3;
		else if (accessor.type == cgltf_type_vec4)
			componentCount = 4;

		// 转化为float数组
		std::vector<float> values;
		GetScalarValues(values, componentCount, accessor);

		// 虽然把这些属性都列出来了, 但每次调用这个函数时, 都只会用到下面的一种
		// 具体是哪种需要根据attribute.type决定
		std::vector<vec3>& positions = outMesh.GetPosition();
		std::vector<vec3>& normals = outMesh.GetNormal();
		std::vector<vec2>& texCoords = outMesh.GetTexCoord();
		std::vector<ivec4>& influences = outMesh.GetInfluences();
		std::vector<vec4>& weights = outMesh.GetWeights();

		// 遍历这个subMesh的每个accessor, 来处理每一部分的Vertex Attribute
		for (unsigned int i = 0; i < (unsigned int)accessor.count; ++i)
		{
			int index = i * componentCount;// componentCount是从accessor.type里读取的
			// 根据类型挖出对应的数据
			switch (attribType)
			{
			case cgltf_attribute_type_position:
				positions.push_back(vec3(values[index + 0], values[index + 1], values[index + 2]));
				break;
			case cgltf_attribute_type_texcoord:
				texCoords.push_back(vec2(values[index + 0], values[index + 1]));
				break;
			case cgltf_attribute_type_weights:
				weights.push_back(vec4(values[index + 0], values[index + 1], values[index + 2], values[index + 3]));
				break;
			case cgltf_attribute_type_normal:
			{
				vec3 normal = vec3(values[index + 0], values[index + 1], values[index + 2]);
				if (lenSq(normal) < 0.000001f)
					normal = vec3(0, 1, 0);

				normals.push_back(normalized(normal));
			}
			break;
			case cgltf_attribute_type_joints:
			{
				// These indices are skin relative. This function has no information about the
				// skin that is being parsed. Add +0.5f to round, since we can't read ints
				ivec4 joints((int)(values[index + 0] + 0.5f),
					(int)(values[index + 1] + 0.5f),
					(int)(values[index + 2] + 0.5f),
					(int)(values[index + 3] + 0.5f)
				);

				joints.x = std::max(0, GetNodeIndex(skin->joints[joints.x], nodes, nodeCount));
				joints.y = std::max(0, GetNodeIndex(skin->joints[joints.y], nodes, nodeCount));
				joints.z = std::max(0, GetNodeIndex(skin->joints[joints.z], nodes, nodeCount));
				joints.w = std::max(0, GetNodeIndex(skin->joints[joints.w], nodes, nodeCount));

				influences.push_back(joints);
			}
			break;
			} // End switch statement
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

// 接口4
Pose LoadBindPose(cgltf_data* data)
{
	// 1. 根据restPose得到一个Transform数组, 每个元素代表Joint的GlobalTransform
	Pose restPose = LoadRestPose(data);
	unsigned int numBones = restPose.Size();
	std::vector<Transform> worldBindPose(numBones);
	for (unsigned int i = 0; i < numBones; ++i)
		worldBindPose[i] = restPose.GetGlobalTransform(i);

	// 2. 遍历每个Skin节点, 这里只有一个模型, 应该只有一个skin对象
	// 基于Skin里的矩阵, 矫正前面得到的Joint的GlobalTransform数组
	unsigned int numSkins = (unsigned int)data->skins_count;
	for (unsigned int i = 0; i < numSkins; ++i)
	{
		// 每个skin节点, 都有一个float数组, 每16个浮点数, 代表一个矩阵
		cgltf_skin* skin = &(data->skins[i]);
		std::vector<float> invBindAccessor;
		GLTFHelpers::GetScalarValues(invBindAccessor, 16, *skin->inverse_bind_matrices);

		unsigned int numJoints = (unsigned int)skin->joints_count;
		for (unsigned int j = 0; j < numJoints; ++j)
		{
			// 读取inverse矩阵, 取逆后转成Transform, 得到世界坐标系下的Trans
			// Read the ivnerse bind matrix of the joint
			float* matrix = &(invBindAccessor[j * 16]);
			mat4 invBindMatrix = mat4(matrix);
			// invert, convert to transform
			mat4 bindMatrix = inverse(invBindMatrix);
			Transform bindTransform = mat4ToTransform(bindMatrix);

			// 基于id, 存到对应的vector的位置上
			cgltf_node* jointNode = skin->joints[j];
			int jointIndex = GLTFHelpers::GetNodeIndex(jointNode, data->nodes, numBones);
			worldBindPose[jointIndex] = bindTransform;
		}
	}


	// 3. 把得到BindPose的GlobalTransform数组, 转换成BindPose的LocalTransform数组
	Pose bindPose = restPose;
	for (unsigned int i = 0; i < numBones; ++i)
	{
		// 3.1 获取该joint的World Transform
		Transform current = worldBindPose[i];

		int p = bindPose.GetParent(i);
		// 如果有parent, 说明其Transform为Local Transform
		if (p >= 0)
		{
			// 3.2 获取该joint的parent的worldTransform
			Transform parent = worldBindPose[p];

			// 3.3 根据俩worldTransform算出相对Parent的Localtransform
			// 要想把A的WorldTrans变为B的LocalTrans, 用Wb.Inverse() * Wa即可
			current = combine(inverse(parent), current);
		}

		bindPose.SetLocalTransform(i, current);
	}

	// 所以说, 这里的Rest Pose里的Joint其实也要转换成Global Transform
	// 否则没有意义, 因为LocalTransform的Parent对应的joint很可能被Bind Pose改变了
	// 这样LocalTransform数据就不对了, 而WorldTransform下, Joint的位置永远是对的

	return bindPose;
}


Skeleton LoadSkeleton(cgltf_data* data)
{
	return Skeleton(LoadRestPose(data), LoadBindPose(data), LoadJointNames(data));
}


// 输入data, 输出Mesh数组(glTF里的每个node都可以有一个Mesh)
std::vector<Mesh> LoadMeshes(cgltf_data* data)
{
	std::vector<Mesh> result;
	cgltf_node* nodes = data->nodes;
	unsigned int nodeCount = (unsigned int)data->nodes_count;

	// 遍历data里的每个node
	for (unsigned int i = 0; i < nodeCount; ++i)
	{
		cgltf_node* node = &nodes[i];

		// 查找既有mesh, 又有skin的节点, 这里一个node只有一个Mesh和Skin
		if (node->mesh == 0 || node->skin == 0)
			continue;

		// 这里的Mesh不是用顶点存储, 而是用的primitive来的, primitive可以理解为subMesh, 里面存了PrimitiveType
		unsigned int numPrims = (unsigned int)node->mesh->primitives_count;

		// 遍历node的Mesh上的每个subMesh, 实际上我看书里的例子里
		// 对于一个人物, 这里只有一个node既有mesh又有skin, 而且它只有一个primitive
		for (unsigned int j = 0; j < numPrims; ++j)
		{
			// 每个subMesh对应我这边的一个Mesh类对象
			result.push_back(Mesh());
			Mesh& mesh = result[result.size() - 1];

			cgltf_primitive* primitive = &node->mesh->primitives[j];

			// 遍历subMesh里的每个attribute
			// 其实就是类似OpenGL里挖取每个顶点属性到VAO的对应槽位里
			unsigned int numAttributes = (unsigned int)primitive->attributes_count;
			for (unsigned int k = 0; k < numAttributes; ++k)
			{
				cgltf_attribute* attribute = &primitive->attributes[k];
				// 对attribute调用MeshFromAttribute函数
				GLTFHelpers::MeshFromAttribute(mesh, *attribute, node->skin, nodes, nodeCount);
			}

			// primitive里存了个index buffer
			if (primitive->indices != 0)
			{
				unsigned int indexCount = (unsigned int)primitive->indices->count;
				std::vector<unsigned int>& indices = mesh.GetIndices();
				indices.resize(indexCount);

				// 读取到Mesh的indices数组里
				for (unsigned int k = 0; k < indexCount; ++k)
					indices[k] = (unsigned int)cgltf_accessor_read_index(primitive->indices, k);
			}
			mesh.UpdateOpenGLBuffers();
		}
	}

	return result;
}
