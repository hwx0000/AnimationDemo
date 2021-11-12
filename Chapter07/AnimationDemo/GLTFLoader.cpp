#include "GLTFLoader.h"
#include <iostream>

cgltf_data* LoadGLTFFile(const char* path)
{
	// 加载之前, 要创建一个cgltf_options类的对象
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	// 使用库文件, 把data和options都读取出来
	cgltf_data* data = NULL;
	// cgltf_result是个枚举
	cgltf_result result = cgltf_parse_file(&options, path, &data);

	// check
	if (result != cgltf_result_success)
	{
		std::cout << "Could not load input file: " << path << "\n";
		return 0;
	}

	// 根据options和path, 把数据读到data里, 这里的options和path传入的都是const 
	result = cgltf_load_buffers(&options, data, path);
	if (result != cgltf_result_success)
	{
		cgltf_free(data);
		std::cout << "Could not load buffers for: " << path << "\n";
		return 0;
	}

	// 再次check
	result = cgltf_validate(data);
	if (result != cgltf_result::cgltf_result_success)
	{
		cgltf_free(data);
		std::cout << "Invalid gltf file: " << path << "\n";
		return 0;
	}
	return data;
}

void FreeGLTFFile(cgltf_data* data)
{
	if (data == 0)
		std::cout << "WARNING: Can't free null data\n";
	else
		cgltf_free(data);
}
