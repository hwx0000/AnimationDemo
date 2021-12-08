#include "Mesh.h"
#include "Draw.h"
#include "Transform.h"

Mesh::Mesh()
{
	mPosAttrib = new Attribute<vec3>();
	mNormAttrib = new Attribute<vec3>();
	mUvAttrib = new Attribute<vec2>();
	mWeightAttrib = new Attribute<vec4>();
	mInfluenceAttrib = new Attribute<ivec4>();
	mIndexBuffer = new IndexBuffer();
}

Mesh::Mesh(const Mesh& other)
{
	mPosAttrib = new Attribute<vec3>();
	mNormAttrib = new Attribute<vec3>();
	mUvAttrib = new Attribute<vec2>();
	mWeightAttrib = new Attribute<vec4>();
	mInfluenceAttrib = new Attribute<ivec4>();
	mIndexBuffer = new IndexBuffer();
	*this = other;
}

Mesh& Mesh::operator=(const Mesh& other)
{
	if (this == &other)
		return *this;

	// 同步CPU上的数据
	mPosition = other.mPosition;
	mNormal = other.mNormal;
	mTexCoord = other.mTexCoord;
	mWeights = other.mWeights;
	mInfluences = other.mInfluences;
	mIndices = other.mIndices;

	// 再同步GPU上的数据
	UpdateOpenGLBuffers();
	return *this;
}

Mesh::~Mesh()
{
	delete mPosAttrib;
	delete mNormAttrib;
	delete mUvAttrib;
	delete mWeightAttrib;
	delete mInfluenceAttrib;
	delete mIndexBuffer;
}

// 一堆Get函数用于读取Mesh里对应的顶点属性数组
std::vector<vec3>& Mesh::GetPosition()
{
	return mPosition;
}

std::vector<vec3>& Mesh::GetNormal()
{
	return mNormal;
}

std::vector<vec2>& Mesh::GetTexCoord()
{
	return mTexCoord;
}

std::vector<vec4>& Mesh::GetWeights()
{
	return mWeights;
}

std::vector<ivec4>& Mesh::GetInfluences()
{
	return mInfluences;
}

std::vector<unsigned int>& Mesh::GetIndices()
{
	return mIndices;
}

// 调用各个Attribute的Set函数, 其实就是取对应的数组
// 然后使用glBindBuffer和glBufferData, 上传到GPU
void Mesh::UpdateOpenGLBuffers()
{
	// 把这些数据重新上传到GPU, 上传到GL_ARRAY_BUFFER上
	if (mPosition.size() > 0)
		mPosAttrib->Set(mPosition);
	if (mNormal.size() > 0)
		mNormAttrib->Set(mNormal);
	if (mTexCoord.size() > 0)
		mUvAttrib->Set(mTexCoord);
	if (mWeights.size() > 0)
		mWeightAttrib->Set(mWeights);
	if (mInfluences.size() > 0)
		mInfluenceAttrib->Set(mInfluences);
	if (mIndices.size() > 0)
		mIndexBuffer->Set(mIndices);
}

// 这五个参数叫做Bind slot indices, 其实就是对每个顶点属性, 分别调用
// 	glBindBuffer(GL_ARRAY_BUFFER, mHandle);
//	glEnableVertexAttribArray(slot);
//  glVertexAttribIPointer(slot, 1, GL_INT, 0, (void*)0);// 具体的类型会根据slot对应的类型改变
// 三个函数
void Mesh::Bind(int position, int normal, int texCoord, int weight, int influcence)
{
	// 调用对应Attribute的BindTo函数, 其实就是指定VAO的顶点属性的layout
	if (position >= 0)
		mPosAttrib->BindTo(position);
	if (normal >= 0)
		mNormAttrib->BindTo(normal);
	if (texCoord >= 0)
		mUvAttrib->BindTo(texCoord);
	if (weight >= 0)
		mWeightAttrib->BindTo(weight);
	if (influcence >= 0)
		mInfluenceAttrib->BindTo(influcence);
}

// Mesh类的Draw函数会调用之前写的全局的Draw函数, 其实就是调用
// glDrawArrays(DrawModeToGLEnum(mode), 0, vertexCount)
void Mesh::Draw()
{
	if (mIndices.size() > 0)
		::Draw(*mIndexBuffer, DrawMode::Triangles);
	else
		::Draw(mPosition.size(), DrawMode::Triangles);
}

// Mesh类的DrawInstanced函数会调用全局的DrawInstanced函数, 其实就是调用
// glDrawArraysInstanced(DrawModeToGLEnum(mode), 0, vertexCount, numInstances);
void Mesh::DrawInstanced(unsigned int numInstances)
{
	if (mIndices.size() > 0)
		::DrawInstanced(*mIndexBuffer, DrawMode::Triangles, numInstances);
	else
		::DrawInstanced(mPosition.size(), DrawMode::Triangles, numInstances);
}

// 调用各个Attribute的UnBindFrom函数
// 本质是调用glBindBuffer加上glDisableVertexAttribArray
void Mesh::UnBind(int position, int normal, int texCoord, int weight, int influcence)
{
	if (position >= 0)
		mPosAttrib->UnBindFrom(position);
	if (normal >= 0)
		mNormAttrib->UnBindFrom(normal);
	if (texCoord >= 0)
		mUvAttrib->UnBindFrom(texCoord);
	if (weight >= 0)
		mWeightAttrib->UnBindFrom(weight);
	if (influcence >= 0)
		mInfluenceAttrib->UnBindFrom(influcence);
}

#if 1
// pose应该是动起来的人物的pose(为啥这俩参数不是const&)
void Mesh::CPUSkin(const Skeleton& skeleton, const Pose& pose)
{
	unsigned int numVerts = (unsigned int)mPosition.size();
	if (numVerts == 0)
		return;

	// 设置size
	mSkinnedPosition.resize(numVerts);
	mSkinnedNormal.resize(numVerts);

	// 这个函数会获取Pose里的每个Joint的WorldTransform, 存到mPosePalette这个mat4组成的vector数组里
	pose.GetMatrixPalette(mPosePalette);
	// 获取bindPose的数据
	std::vector<mat4> invPosePalette = skeleton.GetInvBindPose();

	// 遍历每个顶点
	for (unsigned int i = 0; i < numVerts; ++i)
	{
		ivec4& j = mInfluences[i];// 点受影响的四块Bone的id
		vec4& w = mWeights[i];

		// 矩阵应该从右往左看, 先乘以invPosePalette, 转换到Bone的LocalSpace
		// 再乘以Pose对应Joint的WorldTransform
		mat4 m0 = (mPosePalette[j.x] * invPosePalette[j.x]) * w.x;
		mat4 m1 = (mPosePalette[j.y] * invPosePalette[j.y]) * w.y;
		mat4 m2 = (mPosePalette[j.z] * invPosePalette[j.z]) * w.z;
		mat4 m3 = (mPosePalette[j.w] * invPosePalette[j.w]) * w.w;

		mat4 skin = m0 + m1 + m2 + m3;

		// 计算最终矩阵对Point和Normal的影响
		mSkinnedPosition[i] = transformPoint(skin, mPosition[i]);
		mSkinnedNormal[i] = transformVector(skin, mNormal[i]);
	}

	// 同步GPU端数据
	mPosAttrib->Set(mSkinnedPosition);
	mNormAttrib->Set(mSkinnedNormal);
}

#else
// 俩input, Pose应该是此刻动画的Pose, 俩应该是const&把
void Mesh::CPUSkin(const Skeleton& skeleton, const Pose& pose)
{
	// 前面的部分没变
	unsigned int numVerts = (unsigned int)mPosition.size();
	if (numVerts == 0)
		return;

	// 设置size, 目的是填充mSkinnedPosition和mSkinnedNormal数组
	mSkinnedPosition.resize(numVerts);
	mSkinnedNormal.resize(numVerts);

	// 之前这里是获取输入的Pose的WorldTrans的矩阵数组和BindPose里的InverseTrans矩阵数组
	// 但这里直接获取BindPose就停了
	const Pose& bindPose = skeleton.GetBindPose();

	// 同样遍历每个顶点
	for (unsigned int i = 0; i < numVerts; ++i)
	{
		ivec4& joint = mInfluences[i];
		vec4& weight = mWeights[i];

		// 之前是矩阵取Combine, 现在是算出来的点和向量, 再最后取Combine
		// 虽然Pose里Joint存的都是LocalTrans, 但是重载的[]运算符会返回GlobalTrans
		Transform skin0 = combine(pose[joint.x], inverse(bindPose[joint.x]));
		vec3 p0 = transformPoint(skin0, mPosition[i]);
		vec3 n0 = transformVector(skin0, mNormal[i]);

		Transform skin1 = combine(pose[joint.y], inverse(bindPose[joint.y]));
		vec3 p1 = transformPoint(skin1, mPosition[i]);
		vec3 n1 = transformVector(skin1, mNormal[i]);

		Transform skin2 = combine(pose[joint.z], inverse(bindPose[joint.z]));
		vec3 p2 = transformPoint(skin2, mPosition[i]);
		vec3 n2 = transformVector(skin2, mNormal[i]);

		Transform skin3 = combine(pose[joint.w], inverse(bindPose[joint.w]));
		vec3 p3 = transformPoint(skin3, mPosition[i]);
		vec3 n3 = transformVector(skin3, mNormal[i]);
		mSkinnedPosition[i] = p0 * weight.x + p1 * weight.y + p2 * weight.z + p3 * weight.w;
		mSkinnedNormal[i] = n0 * weight.x + n1 * weight.y + n2 * weight.z + n3 * weight.w;
	}

	mPosAttrib->Set(mSkinnedPosition);
	mNormAttrib->Set(mSkinnedNormal);
}

#endif