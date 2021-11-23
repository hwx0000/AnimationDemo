// 头文件
#ifndef _H_TEXTURE_
#define _H_TEXTURE_

class Texture
{
protected:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mChannels;
	unsigned int mHandle;
private:
	Texture(const Texture& other);
	Texture& operator=(const Texture& other);
public:
	Texture();// glGenTextures
	Texture(const char* path);// glGenTextures and load image(call Load function)
	~Texture();// glDeleteTextures(1, &mHandle);

	void Load(const char* path);

	// bind并把texture作为uniform传给GPU
	void Set(unsigned int uniformIndex, unsigned int textureIndex);
	// 绑定到GL_TEXTURE0的第0槽位
	void UnSet(unsigned int textureIndex);
	unsigned int GetHandle();
};
#endif
