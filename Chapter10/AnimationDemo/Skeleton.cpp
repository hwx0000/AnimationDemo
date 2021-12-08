#include "Skeleton.h"

Skeleton::Skeleton() { }

// Ctor
Skeleton::Skeleton(const Pose& rest, const Pose& bind, const std::vector<std::string>& names)
{
	Set(rest, bind, names);
}

// 相当于Init
void Skeleton::Set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names)
{
	mRestPose = rest;
	mBindPose = bind;
	mJointNames = names;
	UpdateInverseBindPose();// Set完更新InverseBindPose
}

void Skeleton::UpdateInverseBindPose()
{
	unsigned int size = mBindPose.Size();//Pose函数提供了Size函数, 返回joints的个数
	mInvBindPose.resize(size);

	for (unsigned int i = 0; i < size; ++i)
	{
		// 获取每个Joint的Global Transform, 转化为矩阵, 取逆存下来
		Transform world = mBindPose.GetGlobalTransform(i);
		mInvBindPose[i] = inverse(transformToMat4(world));
	}
}

Pose& Skeleton::GetBindPose()
{
	return mBindPose;
}

const Pose& Skeleton::GetBindPose() const
{
	return mBindPose;
}

Pose& Skeleton::GetRestPose()
{
	return mRestPose;
}

const Pose& Skeleton::GetRestPose() const
{
	return mRestPose;
}

std::vector<mat4>& Skeleton::GetInvBindPose()
{
	return mInvBindPose;
}

const std::vector<mat4>& Skeleton::GetInvBindPose() const
{
	return mInvBindPose;
}

std::vector<std::string>& Skeleton::GetJointNames()
{
	return mJointNames;
}

std::string& Skeleton::GetJointName(unsigned int idx)
{
	return mJointNames[idx];
}
