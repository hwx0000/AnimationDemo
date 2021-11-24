#include "quat.h"
#include "mat4.h"

const float QUAT_EPSILON = 0.00001f;

quat angleAxis(float angle, const vec3& axis)
{
	vec3 norm = normalized(axis);
	float s = sinf(angle * 0.5f);

	return quat(norm.x * s,
		norm.y * s,
		norm.z * s,
		cosf(angle * 0.5f));
}

quat fromTo(const vec3& from, const vec3& to)
{
	// 保证输入的两个向量是归一化的
	vec3 f = normalized(from);
	vec3 t = normalized(to);

	// 两个向量相同，则不作任何旋转
	if (f == t)
		return quat();

	// If this edge case happens, find the most perpendicular vector between
	// the two vectors to create a pure quaternion.
	// 两个向量反向时，相当于只有一个向量，这里会自己取一个新的向量
	// If they are opposite vectors, the most orthogonal axis of 
	// the from vector can be used to create a pure quaternion
	// 然后计算叉乘, 得到旋转轴
	else if (f == t * -1.0f)
	{
		// 取一个正交向量
		vec3 ortho = vec3(1, 0, 0);
		// ？？？除了说防止ortho和f重合以外，这样做还有啥原因remain
		if (fabsf(f.y) && fabs(f.z) < fabsf(f.x))
			ortho = vec3(0, 0, 1);

		// 基于from和找的正交基(好像是随便找的)进行叉乘	
		vec3 axis = normalized(cross(f, ortho));
		// 注意，两个向量反向时，返回的是Pure Quaternion
		return quat(axis.x, axis.y, axis.z, 0);
	}

	// 算出f到t的半程向量
	vec3 half = normalized(f + t);
	// 计算叉乘时，结果向量的方向由f和t组成的平面决定，而具体向量的大小由夹角决定和两个Input的向量的长度决定
	// 算出此时的旋转轴，这样做结果就直接有sin(θ/2), 就避免了复杂的反三角函数的运算了
	vec3 axis = cross(f, half);// axis * sin(θ/2)
	return 	quat(axis.x, axis.y, axis.z, dot(f, half));
}


vec3 getAxis(const quat& quat)
{
	// quat的xyz其实是sin(θ/2) * axis, 
	// 但归一化之后这个sin(θ/2)就会被消除了
	return normalized(vec3(quat.x, quat.y, quat.z));
}

float getAngle(const quat& quat)
{
	// 直接取arccos, 乘以2就行了
	return 2.0f * acosf(quat.w);
}

// 加减操作其实就是简单的四个值的加减
quat operator+(const quat& a, const quat& b)
{
	return quat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

quat operator-(const quat& a, const quat& b)
{
	return quat(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

// 四元数与float的乘积
quat operator*(const quat& a, float b)
{
	return quat(a.x * b, a.y * b, a.z * b, a.w * b);
}

// 四元数取负就是四个值都取负
quat operator-(const quat& q)
{
	return quat(-q.x, -q.y, -q.z, -q.w);
}

// 注意这里用的是四元数的QUAT_EPSILON, 而不是之前的EPSILON
bool operator==(const quat& left, const quat& right)
{
	return (fabsf(left.x - right.x) <= QUAT_EPSILON && fabsf(left.y - right.y) <= QUAT_EPSILON && fabsf(left.z - right.z) <= QUAT_EPSILON && fabsf(left.w - left.w) <= QUAT_EPSILON);
}

bool operator!=(const quat& a, const quat& b)
{
	return !(a == b);
}

bool sameOrientation(const quat& left, const quat& right)
{
	// 如果两个四元数的xyzw完全相同, 返回true
	return (fabsf(left.x - right.x) <= QUAT_EPSILON && fabsf(left.y - right.y) <= QUAT_EPSILON && fabsf(left.z - right.z) <= QUAT_EPSILON && fabsf(left.w - left.w) <= QUAT_EPSILON)
		// 或者两个四元数的xyzw均各自互为相反数, 返回true
		|| (fabsf(left.x + right.x) <= QUAT_EPSILON && fabsf(left.y + right.y) <= QUAT_EPSILON && fabsf(left.z + right.z) <= QUAT_EPSILON && fabsf(left.w + left.w) <= QUAT_EPSILON);
}

float dot(const quat& a, const quat& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float lenSq(const quat& q) 
{
	return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

float len(const quat& q) 
{
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON) 
		return 0.0f;

	return sqrtf(lenSq);
}

void normalize(quat& q) 
{
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON) {
		return;
	}
	float i_len = 1.0f / sqrtf(lenSq);

	q.x *= i_len;
	q.y *= i_len;
	q.z *= i_len;
	q.w *= i_len;
}

quat normalized(const quat& q) 
{
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON)
		return quat();

	float i_len = 1.0f / sqrtf(lenSq);

	return quat(
		q.x * i_len,
		q.y * i_len,
		q.z * i_len,
		q.w * i_len
	);
}

// 直接旋转轴取负, 就是其共轭四元数
quat conjugate(const quat& q) 
{
	return quat(-q.x, -q.y, -q.z, q.w);
}

// 如果已经知道q是归一化的了, 那么可以直接调用conjugate函数, 代替这个函数
quat inverse(const quat& q) 
{
	// 归一化的四元数的共轭就是它的逆
	// 但是q可能不是归一化的, 所以可以先求共轭, 再归一化
	float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	if (lenSq < QUAT_EPSILON)
		return quat();

	float recip = 1.0f / lenSq;

	// conjugate / norm
	return quat(-q.x * recip, -q.y * recip, -q.z * recip, q.w * recip);
}
quat operator*(const quat& Q1, const quat& Q2)
{
	return quat(
		Q2.x * Q1.w + Q2.y * Q1.z - Q2.z * Q1.y + Q2.w * Q1.x,
		-Q2.x * Q1.z + Q2.y * Q1.w + Q2.z * Q1.x + Q2.w * Q1.y,
		Q2.x * Q1.y - Q2.y * Q1.x + Q2.z * Q1.w + Q2.w * Q1.z,
		-Q2.x * Q1.x - Q2.y * Q1.y - Q2.z * Q1.z + Q2.w * Q1.w// 三个xyz各自相乘, 取负, 然后加上w的平方
	);
}

// 这种写法比算qvq-1更高效
vec3 operator*(const quat& q, const vec3& v)
{
	return q.vector * 2.0f * dot(q.vector, v)
		+ v * (q.scalar * q.scalar - dot(q.vector, q.vector))
		+ cross(q.vector, v) * 2.0f * q.scalar;
}

quat lerp(const quat& from, const quat& to, float t) 
{
	return from * (1.0f - t) + to * t;
}

quat slerp(const quat& start, const quat& end, float t)
{
	// 使用点积计算两个四元数的相似程度(应该要保证输入单位四元数吧)，其实就是计算两个vec4的点积
	if (fabsf(dot(start, end)) > 1.0f - QUAT_EPSILON)
		return nlerp(start, end, t);

	// 这里的start其实是归一化的四元数, 可以用conjugate代替inverse函数
	quat delta = inverse(start) * end;// 为啥是这样啊，这样是start * delta. = end了，这是右乘
	return normalized((delta ^ t) * start);
}

quat nlerp(const quat& from, const quat& to, float t) 
{
	return normalized(from + (to - from) * t);
}

quat operator^(const quat& q, float f) 
{
	// 解析成轴向角
	float angle = 2.0f * acosf(q.scalar);
	vec3 axis = normalized(q.vector);
	float halfCos = cosf(f * angle * 0.5f);
	float halfSin = sinf(f * angle * 0.5f);
	return quat(axis.x * halfSin,
		axis.y * halfSin,
		axis.z * halfSin,
		halfCos);
}

// 输入为目标的orientation, 输入的方式跟设置摄像机的朝向的方式差不多
// 注意这个up是世界坐标系的Up, 只是为了帮助构建正交基, 代表最终的
quat lookRotation(const vec3& direction, const vec3& up)
{
	// 基于输入, 创建目标orientation对应的三个正交基, 也就是三个local轴, 这点跟创建View矩阵差不多
	vec3 f = normalized(direction); // Object Forward
	vec3 u = normalized(up); // Desired Up
	vec3 r = cross(u, f); // Object Right
	u = cross(f, r); // Object Up

	// deltaRotation只需要算一个forward向量的前后delta就行了
	// From world forward to object forward
	quat worldToObject = fromTo(vec3(0, 0, 1), f);

	// 根据计算的deltaRot, 计算其它的local轴
	// 算出local up
	vec3 objectUp = worldToObject * vec3(0, 1, 0);
	// From object up to desired up
	quat u2u = fromTo(objectUp, u);

	// Rotate to forward direction first
	// then twist to correct up
	quat result = worldToObject * u2u;
	// Don't forget to normalize the result
	return normalized(result);
}

mat4 quatToMat4(const quat& q)
{
	vec3 r = q * vec3(1, 0, 0);
	vec3 u = q * vec3(0, 1, 0);
	vec3 f = q * vec3(0, 0, 1);
	return mat4(r.x, r.y, r.z, 0,
		u.x, u.y, u.z, 0,
		f.x, f.y, f.z, 0,
		0, 0, 0, 1);
}
