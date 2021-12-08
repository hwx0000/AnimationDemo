#ifndef _H_MESH_
#define _H_MESH_

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat4.h"
#include <vector>
#include "Attribute.h"
#include "IndexBuffer.h"
#include "Skeleton.h"
#include "Pose.h"

// 准确的说, 是Mesh和Skinned MeshRenderer的集合, 因为这个Mesh自带对SkinnedMesh的Draw函数
class Mesh
{
	// CPU上的Static Mesh数据
protected:
	// 都是些顶点属性, 还不如封装一个Vertex类呢
	std::vector<vec3> mPosition;
	std::vector<vec3> mNormal;
	std::vector<vec2> mTexCoord;
	std::vector<vec4> mWeights;
	std::vector<ivec4> mInfluences;
	std::vector<unsigned int> mIndices;
	// GPU上的Mesh数据
protected:
	// Attribute类本质就一个int数据(作为Handle), 实际的东西都存在GPU这边
	// 然后Attribute类还有个额外的int, 用于记录GPU这边的数组的size
	Attribute<vec3>* mPosAttrib;
	Attribute<vec3>* mNormAttrib;
	Attribute<vec2>* mUvAttrib;
	Attribute<vec4>* mWeightAttrib;
	Attribute<ivec4>* mInfluenceAttrib;
	IndexBuffer* mIndexBuffer;
protected:
	// 这些数据是只在CPU Skinning里用到的, 是CPU这边的额外保留的一份Copy数据
	// 用作动态的SkinnedMesh(原本的BindPose的Mesh是Static Mesh)
	std::vector<vec3> mSkinnedPosition;
	std::vector<vec3> mSkinnedNormal;
	std::vector<mat4> mPosePalette;// 一个临时对象, 用于Cache从runtime动画的Pose里读取的每个Joint的WorldTransform矩阵
public:
	// 涉及到堆上的操作, 这里居然都开始自己管理了
	Mesh();
	Mesh(const Mesh&);
	Mesh& operator=(const Mesh&);
	~Mesh();

	// 获取顶点属性数组的一些Get函数
	std::vector<vec3>& GetPosition();
	std::vector<vec3>& GetNormal();
	std::vector<vec2>& GetTexCoord();
	std::vector<vec4>& GetWeights();
	std::vector<ivec4>& GetInfluences();
	std::vector<unsigned int>& GetIndices();

	// 实现CPU Skinning的函数, 说实话放到Mesh类里感觉很奇怪, 放到叫SkinnedMeshRenderer这种名字的类里还差不多
	void CPUSkin(const Skeleton& skeleton, const Pose& pose);
	// 当改变存在CPU这边的顶点属性数据时, 调用此函数同步GPU上的数据
	void UpdateOpenGLBuffers();
	// 这些是干啥的?
	void Bind(int position, int normal, int texCoord, int weight, int influcence);
	void Draw();
	void DrawInstanced(unsigned int numInstances);
	void UnBind(int position, int normal, int texCoord, int weight, int influcence);
};

#endif // !_H_MESH_
;

