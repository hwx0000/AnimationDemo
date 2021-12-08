#ifndef _H_TRANSFORMTRACK_
#define _H_TRANSFORMTRACK_

#include "Track.h"
#include "Transform.h"

class TransformTrack
{
protected:
	unsigned int mId;// 对应Bone的Id
	// 居然不是写的Track<vec3, float>, 而是又自己定义了VectorTrack类和QuaternionTrack类
	VectorTrack mPosition;
	QuaternionTrack mRotation;
	VectorTrack mScale;
public:
	TransformTrack();
	unsigned int GetId();
	void SetId(unsigned int id);
	VectorTrack& GetPositionTrack();
	QuaternionTrack& GetRotationTrack();
	VectorTrack& GetScaleTrack();
	float GetStartTime();
	float GetEndTime();
	bool IsValid();
	Transform Sample(const Transform& ref, float time, bool looping);
};

#endif