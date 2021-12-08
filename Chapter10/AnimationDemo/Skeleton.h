#ifndef _H_SKELETON_
#define _H_SKELETON_

#include "Pose.h"
#include "mat4.h"
#include <vector>
#include <string>

class Skeleton
{
protected:
	// Hierarchy的数据其实可以从RestPose或者BindPose里获取
	Pose mRestPose;
	Pose mBindPose;
	// 额外的Skeleton的信息
	std::vector<mat4> mInvBindPose;// 这个数组会基于mBindPose计算得到
	std::vector<std::string> mJointNames;
protected:
	// 计算Inverse矩阵
	void UpdateInverseBindPose();
public:
	Skeleton();
	// 构造函数, 需要一个Pose和joints names
	Skeleton(const Pose& rest, const Pose& bind, const std::vector<std::string>& names);

	void Set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names);

	Pose& GetBindPose();
	const Pose& GetBindPose() const;

	Pose& GetRestPose();
	const Pose& GetRestPose() const;

	std::vector<mat4>& GetInvBindPose();
	const std::vector<mat4>& GetInvBindPose() const;
	std::vector<std::string>& GetJointNames();
	std::string& GetJointName(unsigned int index);
};

#endif 
