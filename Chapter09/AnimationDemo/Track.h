#ifndef _H_TRACK_
#define _H_TRACK_

#include <vector>
#include "Frame.h"
#include "vec3.h"
#include "quat.h"
#include "Interpolation.h"

// 这里认为所有的T类型都是由float组成的, N是float的个数
// 比如Track<vec3, 3>
template<typename T, int N>
class Track
{
protected:
	// 一个Track本质上只有这两样东西: frames的集合和插值类型
	std::vector<Frame<N>> mFrames;
	Interpolation mInterpolation;
protected:
	// protected下面提供的是一些helper函数
	T SampleConstant(float time, bool looping);
	T SampleLinear(float time, bool looping);
	T SampleCubic(float time, bool looping);
	T Hermite(float time, const T& point1, const T& slope1, const T& point2, const T& slope2);
	int FrameIndex(float time, bool looping);
	float AdjustTimeToFitTrack(float time, bool looping);
	T Cast(float* value);
public:
	Track();
	void Resize(unsigned int size);
	unsigned int Size();
	Interpolation GetInterpolation();
	void SetInterpolation(Interpolation interpolation);
	// 获取第一帧的时间
	float GetStartTime();
	// 获取最后一帧的时间
	float GetEndTime();
	// 对Curve进行采样
	T Sample(float time, bool looping);
	// 重载[], 方便返回第i帧的Frame数据
	Frame<N>& operator[](unsigned int index);
};

typedef Track<float, 1> ScalarTrack;
typedef Track<vec3, 3> VectorTrack;
typedef Track<quat, 4> QuaternionTrack;

#endif 
