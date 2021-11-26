#ifndef _H_CLIP_
#define _H_CLIP_

#include <vector>
#include <string>
#include "TransformTrack.h"
#include "Pose.h"

class Clip
{
protected:
	// 本质就是TransformTracks
	std::vector<TransformTrack> mTracks;
	// 一些用于辅助播放的metadata
	std::string mName;
	float mStartTime;
	float mEndTime;
	bool mLooping;
protected:
	float AdjustTimeToFitRange(float inTime);
public:
	Clip();
	// 获取第index个Track对应的Joint的id
	unsigned int GetIdAtIndex(unsigned int index);
	// 设置第index个Track对应的Joint的id
	void SetIdAtIndex(unsigned int index, unsigned int id);
	unsigned int Size();
	// 这里有两种方法从Clip里获取数据, 一个是直接Sample, 返回Pose, 另一个是直接取第i个Joint的TransformTrack
	// Sample Animation Clip, 结果是一个Pose, 这里的return值表示把inTime进行调整后的结果
	float Sample(Pose& outPose, float inTime);
	// 如果没有TransformTrack数据, 会创建一个默认的TransformTrack对象
	TransformTrack& operator[](unsigned int index);
	// 这个函数会遍历Clip里面的Track数据, 计算出mStartTime和mEndTime
	void RecalculateDuration();
	std::string& GetName();
	void SetName(const std::string& inNewName);
	float GetDuration();
	float GetStartTime();
	float GetEndTime();
	bool GetLooping();
	void SetLooping(bool inLooping);
};

#endif 
