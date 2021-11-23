#include "Transform.h"


// a在左边, b在右边
Transform combine(const Transform& a, const Transform& b)
{
	Transform out;
	// scale和rotation直接组合
	out.scale = a.scale * b.scale;
	out.rotation = b.rotation * a.rotation;
	// b相当于是在a的localSpace下的, 其position需要基于a的rotation和scale
	out.position = a.rotation * (a.scale * b.position);
	// 最后加上a的基础position
	out.position = a.position + out.position;
	return out;
}

Transform inverse(const Transform& t)
{
	Transform inv;
	inv.rotation = inverse(t.rotation);
	inv.scale.x = fabs(t.scale.x) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.x;
	inv.scale.y = fabs(t.scale.y) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.y;
	inv.scale.z = fabs(t.scale.z) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.z;
	// position的inverse需要结合rotation的inverse
	vec3 invTrans = t.position * -1.0f;
	inv.position = inv.rotation * (inv.scale * invTrans);
	return inv;
}

Transform mix(const Transform& a, const Transform& b, float t)
{
	// 保证四元数的neighbourhood
	quat bRot = b.rotation;
	if (dot(a.rotation, bRot) < 0.0f)
		bRot = -bRot;

	return Transform(lerp(a.position, b.position, t),
		nlerp(a.rotation, bRot, t),
		lerp(a.scale, b.scale, t));
}


mat4 transformToMat4(const Transform& t)
{
	// 1. 直接用四元数乘以世界坐标系的三个坐标基
	vec3 x = t.rotation * vec3(1, 0, 0);
	vec3 y = t.rotation * vec3(0, 1, 0);
	vec3 z = t.rotation * vec3(0, 0, 1);
	// 2. 新的坐标基各自乘以对应scale的值
	x = x * t.scale.x;
	y = y * t.scale.y;
	z = z * t.scale.z;
	// 3. 位移直接提取就行了, 放到矩阵的第四列
	vec3 p = t.position;
	// Create matrix
	return mat4(x.x, x.y, x.z, 0, // X basis (&Scale)
		y.x, y.y, y.z, 0, // Y basis (&scale)
		z.x, z.y, z.z, 0, // Z basis (& scale)
		p.x, p.y, p.z, 1); // Position;
}

// 把这个矩阵的scale, rotation和position信息提取出来
Transform mat4ToTransform(const mat4& m)
{
	Transform out;
	// 取第四列作为pos
	out.position = vec3(m.v[12], m.v[13], m.v[14]);
	// 第四章写过mat4ToQuat这个函数, 就是把mat的basis vector归一化, 然后重新叉乘得到
	// 调用quaternion.cpp里的函数, 提取出rotation
	out.rotation = mat4ToQuat(m);// 算出R
	// 只取3*3的子矩阵M, M = S*R
	mat4 rotScaleMat(m.v[0], m.v[1], m.v[2], 0,		// 第一列的列向量
		m.v[4], m.v[5], m.v[6], 0,					// 第二列的列向量
		m.v[8], m.v[9], m.v[10], 0,					// 第三列的列向量
		0, 0, 0, 1);
	// 计算R^-1
	mat4 invRotMat = quatToMat4(inverse(out.rotation));
	// M = S*R => S = M * R^-1
	mat4 scaleSkewMat = rotScaleMat * invRotMat;
	out.scale = vec3(scaleSkewMat.v[0],
		scaleSkewMat.v[5],
		scaleSkewMat.v[10]);
	return out;
}
