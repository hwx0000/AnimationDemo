#ifndef _H_POSE_
#define _H_POSE_

#include <vector>
#include "Transform.h"

class Pose
{
protected:
	// 本质数据就是两个vector, 一个代表Joints的hierarchy, 一个代表Joints的数据
	std::vector<Transform> mJoints;
	std::vector<int> mParents;
public:
	// 拥有Ctor, copy constructor和copy assignment operator
	Pose();
	Pose(unsigned int numJoints);
	Pose(const Pose& p);
	Pose& operator=(const Pose& p);
	void Resize(unsigned int size);
	unsigned int Size() const;
	Transform GetLocalTransform(unsigned int index) const;
	void SetLocalTransform(unsigned int index, const Transform& transform);
	Transform GetGlobalTransform(unsigned int index) const;
	Transform operator[](unsigned int index) const;// 使用[]会默认返回Joint的Global Transform
	// palette是调色板的意思, 这个函数是为了把Pose数据传给OpenGL
	// 由于OpenGL只接受linear array of matrices, 它可以用矩阵的方式接受Transform
	// 这个函数会根据自己的Transform数组, 创建一个mat4的数组并返回
	void GetMatrixPalette(std::vector<mat4>& out) const;
	int GetParent(unsigned int index) const;
	void SetParent(unsigned int index, int parent);

	bool operator==(const Pose& other);
	bool operator!=(const Pose& other);
};

#endif // !_H_POSE_
