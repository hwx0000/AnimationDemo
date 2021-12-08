#ifndef _H_SHADER_
#define _H_SHADER_

#include <map>
#include <string>

class Shader
{
private:
	unsigned int mHandle;

	std::map<std::string, unsigned int> mAttributes;
	std::map<std::string, unsigned int> mUniforms;
private:
	std::string ReadFile(const std::string& path);
	// compile shader source code and return an OpenGL handle. 
	unsigned int CompileVertexShader(const std::string& vertex);
	unsigned int CompileFragmentShader(const std::string& fragment);
	// link two shaders into a shader program
	bool LinkShaders(unsigned int vertex, unsigned int fragment);

	// populate有填充的意思, 这俩函数是取OpenGL里的Attribs和Uniforms, 存到两个map里面
	void PopulateAttributes();
	void PopulateUniforms();
private:
	// 通过私有的copy ctor和copy assignment operator禁止Shader对象的拷贝
	Shader(const Shader&);
	Shader& operator=(const Shader&);
public:
	Shader();
	//  The overload constructor will call the Load method, which loads shaders from files and compiles them. 
	Shader(const std::string& vertex, const std::string& fragment);
	// The destructor will release the OpenGL shader handle that the Shader class is holding on
	~Shader();

	void Load(const std::string& vertex, const std::string& fragment);

	// Before a shader is used, it will need to be bound with the Bind function.
	void Bind();
	void UnBind();

	// perform lookups in the appropriate dictionaries
	unsigned int GetAttribute(const std::string& name);
	unsigned int GetUniform(const std::string& name);
	// returns the shader's OpenGL handle
	unsigned int GetHandle();
};

#endif
