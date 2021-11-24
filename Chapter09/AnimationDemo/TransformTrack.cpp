#include "TransformTrack.h"

TransformTrack::TransformTrack()
{
	mId = 0;
}

unsigned int TransformTrack::GetId()
{
	return mId;
}

void TransformTrack::SetId(unsigned int id)
{
	mId = id;
}

VectorTrack& TransformTrack::GetPositionTrack()
{
	return mPosition;
}

QuaternionTrack& TransformTrack::GetRotationTrack()
{
	return mRotation;
}

VectorTrack& TransformTrack::GetScaleTrack()
{
	return mScale;
}

// 只要三条Track任意一条有数据, 则为Valid
bool TransformTrack::IsValid()
{
	return mPosition.Size() > 1 || mRotation.Size() > 1 || mScale.Size() > 1;
}

// 选择三个Track里各种出现关键帧的最早的时间点, 三个再取最小的
float TransformTrack::GetStartTime()
{
	float result = 0.0f;
	bool isSet = false;

	if (mPosition.Size() > 1)
	{
		result = mPosition.GetStartTime();
		isSet = true;
	}
	if (mRotation.Size() > 1)
	{
		float rotationStart = mRotation.GetStartTime();
		if (rotationStart < result || !isSet)
		{
			result = rotationStart;
			isSet = true;
		}
	}
	if (mScale.Size() > 1)
	{
		float scaleStart = mScale.GetStartTime();
		if (scaleStart < result || !isSet)
		{
			result = scaleStart;
			isSet = true;
		}
	}

	return result;
}

// 取三个Track的EndTime里的最晚的
float TransformTrack::GetEndTime()
{
	float result = 0.0f;
	bool isSet = false;

	if (mPosition.Size() > 1)
	{
		result = mPosition.GetEndTime();
		isSet = true;
	}
	if (mRotation.Size() > 1)
	{
		float rotationEnd = mRotation.GetEndTime();
		if (rotationEnd > result || !isSet)
		{
			result = rotationEnd;
			isSet = true;
		}
	}
	if (mScale.Size() > 1)
	{
		float scaleEnd = mScale.GetEndTime();
		if (scaleEnd > result || !isSet)
		{
			result = scaleEnd;
			isSet = true;
		}
	}

	return result;
}

// 各个Track的Sample, 如果有Track的话
// 由于不是所有的动画都有相同的Property对应的track, 比如说有的只有position, 没有rotation和scale
// 在Sample动画A时，如果要换为Sample动画B，要记得重置人物的pose
Transform TransformTrack::Sample(const Transform& ref, float time, bool looping)
{
	// 每次Sample来播放动画时, 都要记录好这个result数据
	Transform result = ref; // Assign default values

	// 这样的ref, 代表原本角色的Transform, 这样即使对应的Track没动画数据, 也没关系
	if (mPosition.Size() > 1)
	{ // Only assign if animated
		result.position = mPosition.Sample(time, looping);
	}
	if (mRotation.Size() > 1)
	{ // Only assign if animated
		result.rotation = mRotation.Sample(time, looping);
	}
	if (mScale.Size() > 1)
	{ // Only assign if animated
		result.scale = mScale.Sample(time, looping);
	}

	return result;
}
