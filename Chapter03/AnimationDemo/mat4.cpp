#include "mat4.h"
#include <cmath>
#include <iostream>
// 16�����������ж�
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

// ������Ը�����, ��Ҫע�����Ϊconst&
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

// ����һ���򵥵ĺ�, ��������, ��֮��ȡ�þ���a�ĵ�m�к;���b�ĵ�n�е���˵Ľ��
// ע�������r�к�c��, ��Ӧ��Ԫ��Ӧ����c*4+r(������r*4+c)
#define M4D(aRow, bCol) \
    a.v[0 * 4 + aRow] * b.v[bCol * 4 + 0] + \
    a.v[1 * 4 + aRow] * b.v[bCol * 4 + 1] + \
    a.v[2 * 4 + aRow] * b.v[bCol * 4 + 2] + \
    a.v[3 * 4 + aRow] * b.v[bCol * 4 + 3]

mat4 operator*(const mat4& a, const mat4& b) {
	return mat4(
		// ���캯���ǰ������������
		// �¾���ĵ�һ��, ��a��ÿһ����b�ĵ�һ�е�M4D�Ľ��
		M4D(0, 0), M4D(1, 0), M4D(2, 0), M4D(3, 0), // Column 0
		M4D(0, 1), M4D(1, 1), M4D(2, 1), M4D(3, 1), // Column 1
		M4D(0, 2), M4D(1, 2), M4D(2, 2), M4D(3, 2), // Column 2
		M4D(0, 3), M4D(1, 3), M4D(2, 3), M4D(3, 3)  // Column 3
	);
}

// ͬ��, ��������˾����mRow����һ��vec4��˵ĺ�
#define M4V4D(mRow, x, y, z, w) \
    x * m.v[0 * 4 + mRow] + \
    y * m.v[1 * 4 + mRow] + \
    z * m.v[2 * 4 + mRow] + \
    w * m.v[3 * 4 + mRow]

// 4*4�������vec4ʱ, ��߾����ÿһ�л�ȥ����v
vec4 operator*(const mat4& m, const vec4& v) {
	return vec4(
		M4V4D(0, v.x, v.y, v.z, v.w),
		M4V4D(1, v.x, v.y, v.z, v.w),
		M4V4D(2, v.x, v.y, v.z, v.w),
		M4V4D(3, v.x, v.y, v.z, v.w)
	);
}

// ��vec3��vec4����, Ȼ���÷��ؾ���������Ľ��, ע��3D����תΪ4D����ʱ, wΪ0
vec3 transformVector(const mat4& m, const vec3& v) {
	return vec3(
		M4V4D(0, v.x, v.y, v.z, 0.0f),
		M4V4D(1, v.x, v.y, v.z, 0.0f),
		M4V4D(2, v.x, v.y, v.z, 0.0f)
	);
}

// ��vec3��vec4����, Ȼ���÷��ؾ���������Ľ��, ע��3D����תΪ4D����ʱ, wΪ0
vec3 transformVector(const mat4& m, const vec3& v)
{
	// ��ʵ����ȡ�����ǰ���и�����(v, 0.0f)�ĳ˻����
	return vec3(M4V4D(0, v.x, v.y, v.z, 0.0f),// ��ֵ��m�ĵ�һ����(v, 0.0f)�˻��Ľ��
		M4V4D(1, v.x, v.y, v.z, 0.0f),
		M4V4D(2, v.x, v.y, v.z, 0.0f));
}

// ע��, 3D�ĵ�תΪ4D�ĵ�ʱ, wΪ1 (���ǲ���wΪ1����0, ���صĽ������û���κ�Ӱ��?)
// (���Ժ�transformVector����������)
vec3 transformPoint(const mat4& m, const vec3& v)
{
	// ��ʵ����ȡ�����ǰ���и�����(v, 1.0f)�ĳ˻����
	return vec3(M4V4D(0, v.x, v.y, v.z, 1.0f),
		M4V4D(1, v.x, v.y, v.z, 1.0f),
		M4V4D(2, v.x, v.y, v.z, 1.0f));
}

// ���������������ĺ�
#define M4SWAP(x, y) \
    {float t = x; x = y; y = t; }

// ������N��Ϊת�þ���
void transpose(mat4& m) {
	M4SWAP(m.yx, m.xy);
	M4SWAP(m.zx, m.xz);
	M4SWAP(m.tx, m.xw);
	M4SWAP(m.zy, m.yz);
	M4SWAP(m.ty, m.yw);
	M4SWAP(m.tz, m.zw);
}

// �����N��ת�þ���
mat4 transposed(const mat4& m) {
	return mat4(
		m.xx, m.yx, m.zx, m.tx,
		m.xy, m.yy, m.zy, m.ty,
		m.xz, m.yz, m.zz, m.tz,
		m.xw, m.yw, m.zw, m.tw
	);
}

// Ӧ�������4*4����������к�������ɵ��Ӿ��������ʽ
#define M4_3X3MINOR(c0, c1, c2, r0, r1, r2) \
    (m.v[c0 * 4 + r0] * (m.v[c1 * 4 + r1] * m.v[c2 * 4 + r2] - m.v[c1 * 4 + r2] * m.v[c2 * 4 + r1]) - \
     m.v[c1 * 4 + r0] * (m.v[c0 * 4 + r1] * m.v[c2 * 4 + r2] - m.v[c0 * 4 + r2] * m.v[c2 * 4 + r1]) + \
     m.v[c2 * 4 + r0] * (m.v[c0 * 4 + r1] * m.v[c1 * 4 + r2] - m.v[c0 * 4 + r2] * m.v[c1 * 4 + r1]))

// ���4*4���������ʽ
float determinant(const mat4& m) {
	return  m.v[0] * M4_3X3MINOR(1, 2, 3, 1, 2, 3)
		- m.v[4] * M4_3X3MINOR(0, 2, 3, 1, 2, 3)
		+ m.v[8] * M4_3X3MINOR(0, 1, 3, 1, 2, 3)
		- m.v[12] * M4_3X3MINOR(0, 1, 2, 1, 2, 3);
}

// ���ذ������A*
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

// ���������
mat4 inverse(const mat4& m)
{
	// ������������ʽ
	float det = determinant(m);

	if (det == 0.0f) 
	{ // Epsilon check would need to be REALLY small
		std::cout << "WARNING: Trying to invert a matrix with a zero determinant\n";
		return mat4();
	}

	// ����A*�������
	mat4 adj = adjugate(m);

	// A^-1 = A*/(|A|)
	return adj * (1.0f / det);
}

// ��m��Ϊ���������
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

// ����frustum�����͸��ͶӰ����
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

// ��ֱ�۵Ĵ���ͶӰ����, �ײ㻹�ǵ���frustum����
mat4 perspective(float fov, float aspect, float znear, float zfar) 
{
	// ����fov��aspect���frustum�Ĳ���
	float ymax = znear * tanf(fov * 3.14159265359f / 360.0f);
	float xmax = ymax * aspect;

	return frustum(-xmax, xmax, -ymax, ymax, znear, zfar);
}

// ����ͶӰ
mat4 ortho(float l, float r, float b, float t, float n, float f)
{
	if (l == r || t == b || n == f)
		return mat4(); // Error
	
	return mat4(2.0f / (r - l), 0, 0, 0,
		0, 2.0f / (t - b), 0, 0,
		0, 0, -2.0f / (f - n), 0,
		-((r + l) / (r - l)), -((t + b) / (t - b)), -((f + n) / (f - n)), 1);
}

// position�����λ��, target��������ĵ�
mat4 lookAt(const vec3& position, const vec3& target, const vec3& up) 
{
	// ���ʼ������������ϵ��0,0,0��, ���ķ���ΪZ�Ḻ����

	// f�������Ŀ�굽����ķ���, Ҳ�����������ȥ�ĸ�����
	vec3 f = normalized(target - position) * -1.0f;// �µ��������ϵ��z��Ӧ����f
	vec3 r = cross(up, f); // Right handed �µ��������ϵ��x��Ӧ����r
	if (r == vec3(0, 0, 0))
		return mat4(); // Error
	
	normalize(r);
	// Ϊɶ��Ҫcrossһ��, �����upֱ��normalize��������ô?
	// ������Ϊ, �����Ա�֤������������Up����������������, ����cross(up, f)������Ҫ
	// ������ķ����Up������������, ֻҪȡ�ô�ֱ��������������Ϊr������
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