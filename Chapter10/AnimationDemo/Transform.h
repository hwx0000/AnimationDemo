#include "quat.h"
#include "mat4.h"

#ifndef _H_TRANSFORM_
#define _H_TRANSFORM_

// Unity里的Transform还有一些Parent相关的父子引用的数据, 这里的Tranform没有定义这些内容
struct Transform
{
	vec3 position;
	quat rotation;
	vec3 scale;

	Transform() : position(vec3(0, 0, 0)),
		rotation(quat(0, 0, 0, 1)),
		scale(vec3(1, 1, 1))
	{}

	Transform(const vec3& p, const quat& r, const vec3& s) :
		position(p), rotation(r), scale(s) {}
};

// 全是全局函数啊
Transform combine(const Transform& a, const Transform& b);
Transform inverse(const Transform& t);
Transform mix(const Transform& a, const Transform& b, float t);
mat4 transformToMat4(const Transform& t);
Transform mat4ToTransform(const mat4& m);
bool operator==(const Transform& a, const Transform& b);
bool operator!=(const Transform& a, const Transform& b);
#endif