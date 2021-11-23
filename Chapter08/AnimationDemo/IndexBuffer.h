// 头文件
#ifndef _H_INDEXBUFFER_
#define _H_INDEXBUFFER_

#include <vector>

class IndexBuffer
{
public:
	unsigned int mHandle;
	unsigned int mCount;
private:
	// 不允许Copy
	IndexBuffer(const IndexBuffer& other);
	IndexBuffer& operator=(const IndexBuffer& other);
public:
	IndexBuffer();// glGenBuffers(1, &mHandle)
	~IndexBuffer();// glDeleteBuffers(1, &mHandle)

	// Set就是把IndexBuffer上传给GPU, 就是调用glBindBuffer和glBufferData
	void Set(unsigned int* inputArray, unsigned int arrayLengt);
	void Set(std::vector<unsigned int>& input);

	unsigned int Count();
	unsigned int GetHandle();
};

#endif

