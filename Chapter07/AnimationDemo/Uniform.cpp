#include "Uniform.h"
#include "glad.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "quat.h"
#include "mat4.h"

// 由于这些Uniform的模板实现在宏里, 为了使用到的时候通过编译, 需要加这些东西
template Uniform<int>;
template Uniform<ivec4>;
template Uniform<ivec2>;
template Uniform<float>;
template Uniform<vec2>;
template Uniform<vec3>;
template Uniform<vec4>;
template Uniform<quat>;
template Uniform<mat4>;

// 为tType类型实现Uniform的模板偏特化
#define UNIFORM_IMPL(gl_func, tType, dType) \
template<> \
void Uniform<tType>::Set(unsigned int slot, tType* data, unsigned int length) { \
    gl_func(slot, (GLsizei)length, (dType*)&data[0]); \
}

//第一个int是uniform对应的数据类型, 第二个int是uniform对应的数据在内存里的类型
UNIFORM_IMPL(glUniform1iv, int, int)
// 上面这个宏等同于:
// template<> // int类型数据的模板特化
// void Uniform<int>::Set(unsigned int slot, int* data, unsigned int length) 
// {
//    glUniform1iv(slot, (GLsizei)length, (int*)&data[0]);
// }

// ivec4由4个int组成, 内存上本质上是int数组
UNIFORM_IMPL(glUniform4iv, ivec4, int)
UNIFORM_IMPL(glUniform2iv, ivec2, int)
UNIFORM_IMPL(glUniform1fv, float, float)
UNIFORM_IMPL(glUniform2fv, vec2, float)
UNIFORM_IMPL(glUniform3fv, vec3, float)
UNIFORM_IMPL(glUniform4fv, vec4, float)
UNIFORM_IMPL(glUniform4fv, quat, float)

// 上面的uniform上传函数都是三个参数, 所以统一用宏, 但是这里的mat4由四个参数, 所以单独特化
template<>
void Uniform<mat4>::Set(unsigned int slot, mat4* inputArray, unsigned int arrayLength)
{
	glUniformMatrix4fv(slot, (GLsizei)arrayLength, false, (float*)&inputArray[0]);
}

// 保留只有两个参数的模板, 此时都是上传的单个Uniform数据
template <typename T>
void Uniform<T>::Set(unsigned int slot, const T& value)
{
	Set(slot, (T*)&value, 1);
}

// 支持上传vector<T>的数组数据
template <typename T>
void Uniform<T>::Set(unsigned int slot, std::vector<T>& value)
{
	Set(slot, &value[0], (unsigned int)value.size());
}
