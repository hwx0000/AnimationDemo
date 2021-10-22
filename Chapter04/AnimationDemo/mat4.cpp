#include "mat4.h"
#include <cmath>
#include <iostream>
// 16个浮点数的判断
bool operator==(const mat4& a, const mat4& b)
{
	for (int i = 0; i < 16; ++i)
	{
		if (fabsf(a.v[i] - b.v[i]) > MAT4_EPSILON)
			return false;
	}
	return true;
}

bool operator!=(const mat4& a, const mat4& b)
{
	return !(a == b);
}

// 矩阵乘以浮点数, 需要注意参数为const&
mat4 operator*(const mat4& m, float f) {
	return mat4(
		m.xx * f, m.xy * f, m.xz * f, m.xw * f,
		m.yx * f, m.yy * f, m.yz * f, m.yw * f,
		m.zx * f, m.zy * f, m.zz * f, m.zw * f,
		m.tx * f, m.ty * f, m.tz * f, m.tw * f
	);
}

mat4 operator+(const mat4& a, const mat4& b) {
	return mat4(
		a.xx + b.xx, a.xy + b.xy, a.xz + b.xz, a.xw + b.xw,
		a.yx + b.yx, a.yy + b.yy, a.yz + b.yz, a.yw + b.yw,
		a.zx + b.zx, a.zy + b.zy, a.zz + b.zz, a.zw + b.zw,
		a.tx + b.tx, a.ty + b.ty, a.tz + b.tz, a.tw + b.tw
	);
}

// 创建一个简单的宏, 帮助计算, 总之是取得矩阵a的第m行和矩阵b的第n列的相乘的结果
// 注意这里的r行和c列, 对应的元素应该是c*4+r(而不是r*4+c)
#define M4D(aRow, bCol) \
    a.v[0 * 4 + aRow] * b.v[bCol * 4 + 0] + \
    a.v[1 * 4 + aRow] * b.v[bCol * 4 + 1] + \
    a.v[2 * 4 + aRow] * b.v[bCol * 4 + 2] + \
    a.v[3 * 4 + aRow] * b.v[bCol * 4 + 3]

mat4 operator*(const mat4& a, const mat4& b) {
	return mat4(
		// 构造函数是按列向量输入的
		// 新矩阵的第一列, 是a的每一行与b的第一列的M4D的结果
		M4D(0, 0), M4D(1, 0), M4D(2, 0), M4D(3, 0), // Column 0
		M4D(0, 1), M4D(1, 1), M4D(2, 1), M4D(3, 1), // Column 1
		M4D(0, 2), M4D(1, 2), M4D(2, 2), M4D(3, 2), // Column 2
		M4D(0, 3), M4D(1, 3), M4D(2, 3), M4D(3, 3)  // Column 3
	);
}

// 同上, 这次是做了矩阵第mRow行与一个vec4相乘的宏
#define M4V4D(mRow, x, y, z, w) \
    x * m.v[0 * 4 + mRow] + \
    y * m.v[1 * 4 + mRow] + \
    z * m.v[2 * 4 + mRow] + \
    w * m.v[3 * 4 + mRow]

// 4*4矩阵乘以vec4时, 左边矩阵的每一行会去乘以v
vec4 operator*(const mat4& m, const vec4& v) {
	return vec4(
		M4V4D(0, v.x, v.y, v.z, v.w),
		M4V4D(1, v.x, v.y, v.z, v.w),
		M4V4D(2, v.x, v.y, v.z, v.w),
		M4V4D(3, v.x, v.y, v.z, v.w)
	);
}

// 把vec3当vec4处理, 然后用返回矩阵乘以它的结果, 注意3D向量转为4D向量时, w为0
vec3 transformVector(const mat4& m, const vec3& v) {
	return vec3(
		M4V4D(0, v.x, v.y, v.z, 0.0f),
		M4V4D(1, v.x, v.y, v.z, 0.0f),
		M4V4D(2, v.x, v.y, v.z, 0.0f)
	);
}

// 注意, 3D的点转为4D的点时, w为1 (但是不管w为1还是0, 返回的结果好像没有任何影响?)
// (所以和transformVector的区别在哪)
vec3 transformPoint(const mat4& m, const vec3& v)
{
	// 其实就是取矩阵的前三行各自与(v, 1.0f)的乘积结果
	return vec3(M4V4D(0, v.x, v.y, v.z, 1.0f),
		M4V4D(1, v.x, v.y, v.z, 1.0f),
		M4V4D(2, v.x, v.y, v.z, 1.0f));
}

// 交换两个浮点数的宏
#define M4SWAP(x, y) \
    {float t = x; x = y; y = t; }

// 将矩阵N变为转置矩阵
void transpose(mat4& m) {
	M4SWAP(m.yx, m.xy);
	M4SWAP(m.zx, m.xz);
	M4SWAP(m.tx, m.xw);
	M4SWAP(m.zy, m.yz);
	M4SWAP(m.ty, m.yw);
	M4SWAP(m.tz, m.zw);
}

// 求矩阵N的转置矩阵
mat4 transposed(const mat4& m) {
	return mat4(
		m.xx, m.yx, m.zx, m.tx,
		m.xy, m.yy, m.zy, m.ty,
		m.xz, m.yz, m.zz, m.tz,
		m.xw, m.yw, m.zw, m.tw
	);
}

// 应该是算出4*4矩阵里的三行和三列组成的子矩阵的行列式
#define M4_3X3MINOR(c0, c1, c2, r0, r1, r2) \
    (m.v[c0 * 4 + r0] * (m.v[c1 * 4 + r1] * m.v[c2 * 4 + r2] - m.v[c1 * 4 + r2] * m.v[c2 * 4 + r1]) - \
     m.v[c1 * 4 + r0] * (m.v[c0 * 4 + r1] * m.v[c2 * 4 + r2] - m.v[c0 * 4 + r2] * m.v[c2 * 4 + r1]) + \
     m.v[c2 * 4 + r0] * (m.v[c0 * 4 + r1] * m.v[c1 * 4 + r2] - m.v[c0 * 4 + r2] * m.v[c1 * 4 + r1]))

// 算出4*4矩阵的行列式
float determinant(const mat4& m) {
	return  m.v[0] * M4_3X3MINOR(1, 2, 3, 1, 2, 3)
		- m.v[4] * M4_3X3MINOR(0, 2, 3, 1, 2, 3)
		+ m.v[8] * M4_3X3MINOR(0, 1, 3, 1, 2, 3)
		- m.v[12] * M4_3X3MINOR(0, 1, 2, 1, 2, 3);
}

// 返回伴随矩阵A*
mat4 adjugate(const mat4& m) {
	// Cofactor(M[i, j]) = Minor(M[i, j]] * pow(-1, i + j)
	mat4 cofactor;

	cofactor.v[0] = M4_3X3MINOR(1, 2, 3, 1, 2, 3);
	cofactor.v[1] = -M4_3X3MINOR(1, 2, 3, 0, 2, 3);
	cofactor.v[2] = M4_3X3MINOR(1, 2, 3, 0, 1, 3);
	cofactor.v[3] = -M4_3X3MINOR(1, 2, 3, 0, 1, 2);

	cofactor.v[4] = -M4_3X3MINOR(0, 2, 3, 1, 2, 3);
	cofactor.v[5] = M4_3X3MINOR(0, 2, 3, 0, 2, 3);
	cofactor.v[6] = -M4_3X3MINOR(0, 2, 3, 0, 1, 3);
	cofactor.v[7] = M4_3X3MINOR(0, 2, 3, 0, 1, 2);

	cofactor.v[8] = M4_3X3MINOR(0, 1, 3, 1, 2, 3);
	cofactor.v[9] = -M4_3X3MINOR(0, 1, 3, 0, 2, 3);
	cofactor.v[10] = M4_3X3MINOR(0, 1, 3, 0, 1, 3);
	cofactor.v[11] = -M4_3X3MINOR(0, 1, 3, 0, 1, 2);

	cofactor.v[12] = -M4_3X3MINOR(0, 1, 2, 1, 2, 3);
	cofactor.v[13] = M4_3X3MINOR(0, 1, 2, 0, 2, 3);
	cofactor.v[14] = -M4_3X3MINOR(0, 1, 2, 0, 1, 3);
	cofactor.v[15] = M4_3X3MINOR(0, 1, 2, 0, 1, 2);

	return transposed(cofactor);
}

// 返回逆矩阵
mat4 inverse(const mat4& m)
{
	// 先算矩阵的行列式
	float det = determinant(m);

	if (det == 0.0f) 
	{ // Epsilon check would need to be REALLY small
		std::cout << "WARNING: Trying to invert a matrix with a zero determinant\n";
		return mat4();
	}

	// 计算A*伴随矩阵
	mat4 adj = adjugate(m);

	// A^-1 = A*/(|A|)
	return adj * (1.0f / det);
}

// 把m变为它的逆矩阵
void invert(mat4& m)
{
	float det = determinant(m);

	if (det == 0.0f) {
		std::cout << "WARNING: Trying to invert a matrix with a zero determinant\n";
		m = mat4();
		return;
	}

	m = adjugate(m) * (1.0f / det);
}

// 创建frustum代表的透视投影矩阵
mat4 frustum(float l, float r, float b, float t, float n, float f)
{
	if (l == r || t == b || n == f) 
	{
		std::cout << "WARNING: Trying to create invalid frustum\n";
		return mat4(); // Error
	}

	return mat4((2.0f * n) / (r - l), 0, 0, 0,
		0, (2.0f * n) / (t - b), 0, 0,
		(r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
		0, 0, (-2 * f * n) / (f - n), 0);
}

// 更直观的创建投影矩阵, 底层还是调用frustum函数
mat4 perspective(float fov, float aspect, float znear, float zfar) 
{
	// 根据fov和aspect算出frustum的参数
	float ymax = znear * tanf(fov * 3.14159265359f / 360.0f);
	float xmax = ymax * aspect;

	return frustum(-xmax, xmax, -ymax, ymax, znear, zfar);
}

// 正交投影
mat4 ortho(float l, float r, float b, float t, float n, float f)
{
	if (l == r || t == b || n == f)
		return mat4(); // Error
	
	return mat4(2.0f / (r - l), 0, 0, 0,
		0, 2.0f / (t - b), 0, 0,
		0, 0, -2.0f / (f - n), 0,
		-((r + l) / (r - l)), -((t + b) / (t - b)), -((f + n) / (f - n)), 1);
}

// position是相机位置, target是相机看的点
mat4 lookAt(const vec3& position, const vec3& target, const vec3& up) 
{
	// 相机始终在世界坐标系的0,0,0点, 看的方向为Z轴负方向

	// f代表看向的目标到相机的方向, 也就是相机看过去的负方向
	vec3 f = normalized(target - position) * -1.0f;// 新的相机坐标系的z轴应该是f
	vec3 r = cross(up, f); // Right handed 新的相机坐标系的x轴应该是r
	if (r == vec3(0, 0, 0))
		return mat4(); // Error
	
	normalize(r);
	// 为啥还要cross一次, 输入的up直接normalize不就行了么?
	// 这是因为, 不可以保证相机看到方向和Up方向正好是正交的, 而且cross(up, f)并不需要
	// 相机看的方向和Up方向是正交的, 只要取得垂直与这个面的向量作为r就行了
	vec3 u = normalized(cross(f, r)); // Right handed

	vec3 t = vec3(-dot(r, position),
		-dot(u, position),
		-dot(f, position));

	return mat4(// Transpose upper 3x3 matrix to invert it
		r.x, u.x, f.x, 0,
		r.y, u.y, f.y, 0,
		r.z, u.z, f.z, 0,
		t.x, t.y, t.z, 1);
}