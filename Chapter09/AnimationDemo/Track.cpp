#include "Track.h"

template Track<float, 1>;
template Track<vec3, 3>;
template Track<quat, 4>;

// namespace下定义一些helper的全局内联函数, 这些代码其实可以放到单独的header里
namespace TrackHelpers
{
	// float的线性插值
	inline float Interpolate(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	// vec3的线性插值
	inline vec3 Interpolate(const vec3& a, const vec3& b, float t)
	{
		return lerp(a, b, t);
	}

	// 四元数的线性插值
	inline quat Interpolate(const quat& a, const quat& b, float t)
	{
		// 虽然这里叫mix函数, 但是本质还是俩vec4的线性插值操作
		quat result = lerp(a, b, t);
		if (dot(a, b) < 0) // Neighborhood
			result = lerp(a, -b, t);

		return normalized(result); //NLerp, not slerp
	}

	// 对于vec3,float和quaternion, 只有quaternion需要矫正为单元四元数
	// 这里为了通用接口, 所有的Properry类型都要实现这个AdjustHermiteResult函数
	inline float AdjustHermiteResult(float f)
	{
		return f;
	}

	inline vec3 AdjustHermiteResult(const vec3& v)
	{
		return v;
	}

	// 归一化
	inline quat AdjustHermiteResult(const quat& q)
	{
		return normalized(q);
	}

	// 只有quaternion的插值有neighborhood问题
	inline void Neighborhood(const float& a, float& b) { }
	inline void Neighborhood(const vec3& a, vec3& b) { }
	inline void Neighborhood(const quat& a, quat& b)
	{
		if (dot(a, b) < 0)
			b = -b;
	}
}; // End Track Helpers namespace

// 默认ctor, 默认插值类型为线性插值
template<typename T, int N>
Track<T, N>::Track()
{
	mInterpolation = Interpolation::Linear;
}


template<typename T, int N>
float Track<T, N>::GetStartTime()
{
	return mFrames[0].mTime;
}

template<typename T, int N>
float Track<T, N>::GetEndTime()
{
	return mFrames[mFrames.size() - 1].mTime;
}

// Sample的时候根据插值类型来
template<typename T, int N>
T Track<T, N>::Sample(float time, bool looping)
{
	if (mInterpolation == Interpolation::Constant)
		return SampleConstant(time, looping);
	else if (mInterpolation == Interpolation::Linear)
		return SampleLinear(time, looping);

	return SampleCubic(time, looping);
}

// 返回第i帧的Frame对象
template<typename T, int N>
Frame<N>& Track<T, N>::operator[](unsigned int index)
{
	return mFrames[index];
}

// vector的resize
template<typename T, int N>
void Track<T, N>::Resize(unsigned int size)
{
	mFrames.resize(size);
}

template<typename T, int N>
unsigned int Track<T, N>::Size()
{
	return mFrames.size();
}

template<typename T, int N>
Interpolation Track<T, N>::GetInterpolation()
{
	return mInterpolation;
}

template<typename T, int N>
void Track<T, N>::SetInterpolation(Interpolation interpolation)
{
	mInterpolation = interpolation;
}

// 这个函数会被SampleCubic函数调用
// 具体的是采样一个Hermite Curve, 返回t比例处的property
template<typename T, int N>
T Track<T, N>::Hermite(float t, const T& p1, const T& s1, const T& _p2, const T& s2)
{
	float tt = t * t;
	float ttt = tt * t;

	// 其实只有T为quaternion时, 才需要对p2进行neighborhood处理
	T p2 = _p2;
	TrackHelpers::Neighborhood(p1, p2);

	// 各个系数是基于t的三次方函数
	float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
	float h2 = -2.0f * ttt + 3.0f * tt;
	float h3 = ttt - 2.0f * tt + t;
	float h4 = ttt - tt;

	// 其实只有T为quaternion时, 才需要对结果进行归一化处理
	T result = p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
	return TrackHelpers::AdjustHermiteResult(result);
}

// 根据时间获取对应的帧数, 其实是返回其左边的关键帧
// 注意这里的frames应该是按照关键帧来存的, 比如有frames里有三个元素, 可能分别对应的时间为
// 0, 4, 10, 那么我input time为5时, 返回的index为1, 代表从第二帧开始
// 这个函数返回值保证会在[0, size - 2]区间内
template<typename T, int N>
int Track<T, N>::FrameIndex(float time, bool looping)
{
	unsigned int size = (unsigned int)mFrames.size();
	if (size <= 1)
		return -1;

	if (looping)
	{
		float startTime = mFrames[0].mTime;
		float endTime = mFrames[size - 1].mTime;
		float duration = endTime - startTime;

		time = fmodf(time - startTime, endTime - startTime);
		if (time < 0.0f)
			time += endTime - startTime;

		time = time + startTime;
	}
	else
	{
		if (time <= mFrames[0].mTime)
			return 0;
		// 注意, 只要大于倒数第二帧的时间, 就返回其帧数
		// 也就是说, 这个函数返回值在[0, size - 2]区间内
		if (time >= mFrames[size - 2].mTime)
			return (int)size - 2;
	}

	// time的下边界确定以后
	// 从后往前遍历所有的帧对应的时间, 找到time的上边界
	for (int i = (int)size - 1; i >= 0; --i)
	{
		if (time >= mFrames[i].mTime)
			return i;
	}

	// Invalid code, we should not reach here!
	return -1;
} // End of FrameIndex

// 其实就是保证time在动画对应的播放时间区间内, loop为false就是Clamp到此区间
// 只是为了方便播放的计算时间的API
// loop为true就是对区间取模
template<typename T, int N>
float Track<T, N>::AdjustTimeToFitTrack(float time, bool looping)
{
	unsigned int size = (unsigned int)mFrames.size();
	if (size <= 1)
		return 0.0f;

	float startTime = mFrames[0].mTime;
	float endTime = mFrames[size - 1].mTime;
	float duration = endTime - startTime;
	if (duration <= 0.0f)
		return 0.0f;
	if (looping)
	{
		time = fmodf(time - startTime, endTime - startTime);
		if (time < 0.0f)
			time += endTime - startTime;

		time = time + startTime;
	}
	else
	{
		if (time <= mFrames[0].mTime)
			time = startTime;
		if (time >= mFrames[size - 1].mTime)
			time = endTime;
	}

	return time;
}

// 举个AdjustTimeToFitTrack的例子:
//Track<float, 1> t;// t是代表时间的Track
//float mAnimTime = 0.0f;
//void Update(float dt)
//{
// 	// dt: delta time of frame
//	mAnimTime = t.AdjustTimeToFitTrack(mAnimTime + dt);
//}


// 三个Cast函数, 用于把数组转型为T实际对应的类型
// 这里认为所有的数据本质上都是float*
// 然后写模板特化, 把float*解析为不同的类型的数据
template<> float Track<float, 1>::Cast(float* value)
{
	return value[0];
}

template<> vec3 Track<vec3, 3>::Cast(float* value)
{
	return vec3(value[0], value[1], value[2]);
}

template<> quat Track<quat, 4>::Cast(float* value)
{
	quat r = quat(value[0], value[1], value[2], value[3]);
	return normalized(r);
}


template<typename T, int N>
T Track<T, N>::SampleConstant(float time, bool looping)
{
	// 获取时间对应的帧数, 取整
	int frame = FrameIndex(time, looping);
	if (frame < 0 || frame >= (int)mFrames.size())
		return T();

	// Constant曲线不需要插值, mFrames里应该只有关键帧的frame数据
	return Cast(&mFrames[frame].mValue[0]);
	// 为啥要转型? 因为mValue是float*类型的数组, 这里的操作是取从数组地址开始, Cast为T类型 
}

template<typename T, int N>
T Track<T, N>::SampleLinear(float time, bool looping)
{
	// 找到左边的关键帧对应的id
	int thisFrame = FrameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= (int)(mFrames.size() - 1))
		return T();

	// 右边的关键帧对应的id	
	int nextFrame = thisFrame + 1;

	// 其实这段代码应该只有looping为true的时候会起作用
	float trackTime = AdjustTimeToFitTrack(time, looping);

	// 后面就是线性插值了
	float frameDelta = mFrames[nextFrame].mTime - mFrames[thisFrame].mTime;
	if (frameDelta <= 0.0f)
		return T();

	float t = (trackTime - mFrames[thisFrame].mTime) / frameDelta;

	// mValue是mFrames里代表关键帧数据的数组, 比如T为vec3时, mValue就是
	// 一个float[3]的数组
	T start = Cast(&mFrames[thisFrame].mValue[0]);
	T end = Cast(&mFrames[nextFrame].mValue[0]);

	return TrackHelpers::Interpolate(start, end, t);
}

// 重点函数
template<typename T, int N>
T Track<T, N>::SampleCubic(float time, bool looping)
{
	// 前面的步骤跟上面的差不多
	// 获取左右关键帧的id
	int thisFrame = FrameIndex(time, looping);
	if (thisFrame < 0 || thisFrame >= (int)(mFrames.size() - 1))
		return T();

	int nextFrame = thisFrame + 1;

	float trackTime = AdjustTimeToFitTrack(time, looping);
	float frameDelta = mFrames[nextFrame].mTime - mFrames[thisFrame].mTime;
	if (frameDelta <= 0.0f)
		return T();

	// t的算法也一样, t代表两帧之间的比例
	float t = (trackTime - mFrames[thisFrame].mTime) / frameDelta;

	// 获取左边关键帧的point和slope, 如果point是vec2, 那么slope也是vec2, 所以类型都是T
	T point1 = Cast(&mFrames[thisFrame].mValue[0]);
	T slope1;// = mFrames[thisFrame].mOut * frameDelta;
	// 注意：这里取的是左边关键帧的Out切线, In切线没有用到, 这里的Out和In
	// 由于是Delta T的切线, 所以跟T类型是一模一样的
	// 而且这里用到的是memcpy函数, 不是Cast函数, 因为对于quaternnion这种类型的T
	// Cast函数会做归一化处理, 而这里是算切线, 不需要归一化
	memcpy(&slope1, mFrames[thisFrame].mOut, N * sizeof(float));
	slope1 = slope1 * frameDelta;

	T point2 = Cast(&mFrames[nextFrame].mValue[0]);
	T slope2;// = mFrames[nextFrame].mIn[0] * frameDelta;
	// 注意：这里取的是右边关键帧的In切线, Out切线没有用到
	memcpy(&slope2, mFrames[nextFrame].mIn, N * sizeof(float));
	slope2 = slope2 * frameDelta;

	// 根据这四个值组成的Curve, 然后输入t部分返回的Property的T的值
	return Hermite(t, point1, slope1, point2, slope2);
}
