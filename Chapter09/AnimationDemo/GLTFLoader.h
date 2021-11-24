#pragma once


// 简单的两个接口, 一个负责读取, 一个负责销毁读取的缓存
#ifndef _H_GLTFLOADER_
#define _H_GLTFLOADER_

#include "vendor/cgltf.h"

// 返回一个cgltf_data对象, 这个对象包含了所有的glTf里的内容
cgltf_data* LoadGLTFFile(const char* path);
void FreeGLTFFile(cgltf_data* handle);

#endif
