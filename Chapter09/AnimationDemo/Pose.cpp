#include "Pose.h"

Pose::Pose() { }

Pose::Pose(unsigned int numJoints)
{
	Resize(numJoints);
}

// copy ctor居然这么写, 这里做的是浅拷贝, 拷贝的是引用(感觉是在当C#在用, 不过这里
// 其实没有手动在heap上进行分配, 用默认的copy ctor也是可以的)
Pose::Pose(const Pose& p)
{
	*this = p;
}

// overload copy assignment operator
Pose& Pose::operator=(const Pose& p)
{
	if (&p == this)
		return *this;

	// 同步this的两个数组的size
	if (mParents.size() != p.mParents.size())
		mParents.resize(p.mParents.size());

	if (mJoints.size() != p.mJoints.size())
		mJoints.resize(p.mJoints.size());

	// 使用memcpy去copy(直接使用vector的=不好么), 哦, 那样好像是shallow copy
	if (mParents.size() != 0)
		memcpy(&mParents[0], &p.mParents[0], sizeof(int) * mParents.size());

	if (mJoints.size() != 0)
		memcpy(&mJoints[0], &p.mJoints[0], sizeof(Transform) * mJoints.size());

	return *this;
}

void Pose::Resize(unsigned int size)
{
	mParents.resize(size);
	mJoints.resize(size);
}

unsigned int Pose::Size()
{
	return mJoints.size();
}

Transform Pose::GetLocalTransform(unsigned int index)
{
	return mJoints[index];
}

void Pose::SetLocalTransform(unsigned int index, const Transform& transform)
{
	mJoints[index] = transform;
}

// 按Hierarchy层层Combine
Transform Pose::GetGlobalTransform(unsigned int index)
{
	Transform result = mJoints[index];
	// 从底部往上Combine, 因为每个Joint的Parent Joint在vector里的id是可以获取到的
	for (int parent = mParents[index]; parent >= 0; parent = mParents[parent])
		// Combine这个函数是之前在mat4.cpp里实现的全局函数, 其实就是return matA*matB
		result = combine(mJoints[parent], result);

	return result;
}

Transform Pose::operator[](unsigned int index)
{
	return GetGlobalTransform(index);
}

// vector<Transform> globalTrans 转化为mat4数组
void Pose::GetMatrixPalette(std::vector<mat4>& out)
{
	unsigned int size = Size();
	if (out.size() != size)
		out.resize(size);

	for (unsigned int i = 0; i < size; ++i)
	{
		Transform t = GetGlobalTransform(i);
		out[i] = transformToMat4(t);
	}
}

int Pose::GetParent(unsigned int index)
{
	return mParents[index];
}

void Pose::SetParent(unsigned int index, int parent)
{
	mParents[index] = parent;
}

// 判断两个Pose是否相等
bool Pose::operator==(const Pose& other)
{
	// 先判断size
	if (mJoints.size() != other.mJoints.size())
		return false;

	if (mParents.size() != other.mParents.size())
		return false;

	unsigned int size = (unsigned int)mJoints.size();
	// 遍历每个Joint, 其实就是遍历俩vector
	for (unsigned int i = 0; i < size; ++i)
	{
		Transform thisLocal = mJoints[i];
		Transform otherLocal = other.mJoints[i];

		int thisParent = mParents[i];
		int otherParent = other.mParents[i];

		if (thisParent != otherParent)
			return false;

		if (thisLocal != otherLocal)
			return false;
	}
	return true;
}

bool Pose::operator!=(const Pose& other)
{
	return !(*this == other);
}
