#include "Clip.h"

Clip::Clip()
{
	mName = "No name given";
	mStartTime = 0.0f;
	mEndTime = 0.0f;
	mLooping = true;
}

// 这里的Sample函数还对输入的Pose有要求, 因为Clip里的Track如果没有涉及到每个Component的
// 动画, 则会按照输入Pose的值来播放, 所以感觉outPose输入的时候要为(rest Pose(T-Pose or A-Pose))
float Clip::Sample(Pose& outPose, float time)
{
	if (GetDuration() == 0.0f)
		return 0.0f;

	time = AdjustTimeToFitRange(time);// 调用Clip自己实现的函数

	unsigned int size = mTracks.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		unsigned int joint = mTracks[i].GetId();
		Transform local = outPose.GetLocalTransform(joint);
		Transform animated = mTracks[i].Sample(local, time, mLooping);
		outPose.SetLocalTransform(joint, animated);
	}

	return time;
}

// 不仅仅是Track实现了这个函数, 这里的Clip也实现了这个函数
float Clip::AdjustTimeToFitRange(float inTime)
{
	// 其实就是根据mLooping, mStartTime和mEndTime对时间进行处理
	if (mLooping)
	{
		float duration = mEndTime - mStartTime;
		if (duration <= 0)
			return 0.0f;

		inTime = fmodf(inTime - mStartTime, mEndTime - mStartTime);
		if (inTime < 0.0f)
			inTime += mEndTime - mStartTime;

		inTime = inTime + mStartTime;
	}
	else
	{
		if (inTime < mStartTime)
			inTime = mStartTime;

		if (inTime > mEndTime)
			inTime = mEndTime;
	}
	return inTime;
}

// 其实就是遍历所有的Joints的TransformTrack, 找到最早和最晚的关键帧的出现时间
// 设置给mStartTime和mEndTime
void Clip::RecalculateDuration()
{
	mStartTime = 0.0f;
	mEndTime = 0.0f;
	// 这俩相当于first进入的flag
	bool startSet = false;
	bool endSet = false;
	unsigned int tracksSize = (unsigned int)mTracks.size();
	// 遍历每个Joint的TransformTrack
	for (unsigned int i = 0; i < tracksSize; ++i)
	{
		if (mTracks[i].IsValid())
		{
			float trackStartTime = mTracks[i].GetStartTime();
			float trackEndTime = mTracks[i].GetEndTime();

			// 如果trackStartTime小于mStartTime或者startSet为false
			if (trackStartTime < mStartTime || !startSet)
			{
				// 所以mStartTime的默认值其实是第一个Track的GetStartTime的结果
				mStartTime = trackStartTime;
				startSet = true;
			}

			if (trackEndTime > mEndTime || !endSet)
			{
				mEndTime = trackEndTime;
				endSet = true;
			}
		}
	}
}


TransformTrack& Clip::operator[](unsigned int joint)
{
	// 遍历所有的TransformTrack, 获取里面存的Joint的id, 如果找到了指定id的joint
	// 则返回在mTracks里的ID
	for (unsigned int i = 0, size = (unsigned int)mTracks.size(); i < size; ++i)
	{
		if (mTracks[i].GetId() == joint)
			return mTracks[i];
	}

	// 如果没有这个Joint的动画数据, 说明它在动画里没变过, 这里就创建一个默认的TransformTrack返回
	// 感觉好像没必要啊? 既然没变过, 创建的默认的TransformTrack也不对啊, 会改变它的Transform成默认状态的
	// 除非该Joint本身就是单位矩阵对应的Transform, 不然会有问题
	mTracks.push_back(TransformTrack());
	mTracks[mTracks.size() - 1].SetId(joint);
	return mTracks[mTracks.size() - 1];
}

std::string& Clip::GetName()
{
	return mName;
}

void Clip::SetName(const std::string& inNewName)
{
	mName = inNewName;
}

unsigned int Clip::GetIdAtIndex(unsigned int index)
{
	return mTracks[index].GetId();
}

void Clip::SetIdAtIndex(unsigned int index, unsigned int id)
{
	return mTracks[index].SetId(id);
}

unsigned int Clip::Size()
{
	return (unsigned int)mTracks.size();
}

float Clip::GetDuration()
{
	return mEndTime - mStartTime;
}

float Clip::GetStartTime()
{
	return mStartTime;
}

float Clip::GetEndTime()
{
	return mEndTime;
}

bool Clip::GetLooping()
{
	return mLooping;
}

void Clip::SetLooping(bool inLooping)
{
	mLooping = inLooping;
}
