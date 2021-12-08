#ifndef _H_MAT4_
#define _H_MAT4_

#include "vec3.h"
#include "vec4.h"

#define MAT4_EPSILON 0.000001f
struct quat;
struct Transform;

struct mat4
{
	union {
		// 方法1. 16个float的数组
		float v[16];
		// 方法2. 四个列向量对应的vec4, 组成一个struct对象
		struct {
			vec4 right;
			vec4 up;
			vec4 forward;
			vec4 position;
		};
		// 方法3. 16个不同命名的float值, 组成一个struct对象(其实就是16个float都加上名字)
		struct {
			//            row 1     row 2     row 3     row 4
			/* column 1 */float xx; float xy; float xz; float xw;
			/* column 2 */float yx; float yy; float yz; float yw;
			/* column 3 */float zx; float zy; float zz; float zw;
			/* column 4 */float tx; float ty; float tz; float tw;
		};
		// 方法4. 使用列在前, 行在后的方式, 取得数组中的元素
		struct {
			// 注意第一行的c都是c0, 而不是r为r0, 这是因为存的是列
			float c0r0; float c0r1; float c0r2; float c0r3;
			float c1r0; float c1r1; float c1r2; float c1r3;
			float c2r0; float c2r1; float c2r2; float c2r3;
			float c3r0; float c3r1; float c3r2; float c3r3;
		};
		// 方法5. 就是把方法4的变量名掉了个顺序
		struct {
			float r0c0; float r1c0; float r2c0; float r3c0;
			float r0c1; float r1c1; float r2c1; float r3c1;
			float r0c2; float r1c2; float r2c2; float r3c2;
			float r0c3; float r1c3; float r2c3; float r3c3;
		};
	};
	// Include constructors here

	inline mat4() :
		xx(1), xy(0), xz(0), xw(0),
		yx(0), yy(1), yz(0), yw(0),
		zx(0), zy(0), zz(1), zw(0),
		tx(0), ty(0), tz(0), tw(1) {}

	inline mat4(float* fv) :
		xx(fv[0]), xy(fv[1]), xz(fv[2]), xw(fv[3]),
		yx(fv[4]), yy(fv[5]), yz(fv[6]), yw(fv[7]),
		zx(fv[8]), zy(fv[9]), zz(fv[10]), zw(fv[11]),
		tx(fv[12]), ty(fv[13]), tz(fv[14]), tw(fv[15]) { }

	// 按照列向量的方式输入
	inline mat4(
		float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33) :
		xx(_00), xy(_01), xz(_02), xw(_03),
		yx(_10), yy(_11), yz(_12), yw(_13),
		zx(_20), zy(_21), zz(_22), zw(_23),
		tx(_30), ty(_31), tz(_32), tw(_33) {}
}; // end mat4 struct

// 这些函数作为全局函数, 定义在mat4类外
bool operator==(const mat4& a, const mat4& b);
bool operator!=(const mat4& a, const mat4& b);
mat4 operator*(const mat4& m, float f);
mat4 operator+(const mat4& a, const mat4& b);
mat4 operator*(const mat4& a, const mat4& b);
vec4 operator*(const mat4& m, const vec4& v);
vec3 transformVector(const mat4& m, const vec3& v);
vec3 transformVector(const Transform& m, const vec3& v);
vec3 transformPoint(const mat4& m, const vec3& v);
vec3 transformPoint(const Transform& a, const vec3& b);
vec3 transformPoint(const mat4& m, const vec3& v, float& w);
void transpose(mat4& m);
mat4 transposed(const mat4& m);
float determinant(const mat4& m);
mat4 adjugate(const mat4& m);
mat4 inverse(const mat4& m);
void invert(mat4& m);
// 1.1 数学方式直接创建透视投影矩阵
mat4 frustum(float l, float r, float b, float t, float n, float f);
// 1.2 通过更直观的参数来创建透视投影矩阵
mat4 perspective(float fov, float aspect, float znear, float zfar);
// 2. 通过直接的数学运算计算正交投影矩阵, 一般用于2D相机
mat4 ortho(float l, float r, float b, float t, float n, float f);
// 创建View矩阵
mat4 lookAt(const vec3& position, const vec3& target, const vec3& up);
quat mat4ToQuat(const mat4& m);
#endif