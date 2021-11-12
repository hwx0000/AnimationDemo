#ifndef _H_UNIFORM_
#define _H_UNIFORM_

#include <vector>

template <typename T>
class Uniform
{
private:
	// 私有构造函数, 防止类实例化(感觉是不是也可以用纯虚函数)
	Uniform();
	Uniform(const Uniform&);
	Uniform& operator=(const Uniform&);
	~Uniform();
public:
	// 感觉用Upload的函数命名是不是比Set好一些
	static void Set(unsigned int slot, const T& value);
	static void Set(unsigned int slot, T* inputArray, unsigned int arrayLength);
	static void Set(unsigned int slot, std::vector<T>& inputArray);
};

#endif
