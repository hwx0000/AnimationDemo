#ifndef _H_ATTRIBUTE_
#define _H_ATTRIBUTE_

#include <vector>

// 模板类, 可以存储任何顶点属性的数据
template<typename T>
class Attribute
{
protected:
	unsigned int mHandle;
	unsigned int mCount;
private:
	// 同样是不允许Attribute对象复制, 因为它传给GPU就可以了
	Attribute(const Attribute& other);
	Attribute& operator=(const Attribute& other);
	void SetAttribPointer(unsigned int slot);// 调用glVertexAttribIPointer
public:
	Attribute();// 构造函数里调用glGenBuffers, 得到的handle赋给mHandle
	~Attribute();

	// Set函数Set的是GPU里的数据, 其实是Upload CPU数据到GPU
	// 内部调用的是glBindBuffer和glBufferData
	// Set函数接受两种形式的T类型的数组
	void Set(T* inputArray, unsigned int arrayLength);
	void Set(std::vector<T>& input);

	// 底层应该是调用glVertexArrayAttrib
	void BindTo(unsigned int slot);
	void UnBindFrom(unsigned int slot);

	unsigned int Count();// return mCount
	unsigned int GetHandle();// return mHandle
};

#endif
