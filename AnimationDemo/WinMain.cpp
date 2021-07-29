// 使用这些define来减少使用windows.h需要书写的额外的代码
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "glad.h"
#include <windows.h>
#include <iostream>
#include "Application.h"

// 这些常量帮助构建起OpenGL 3.3 Core context, 只是把glew里的核心内容拿了出来
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
#define WGL_CONTEXT_FLAGS_ARB 0x2094 
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001 
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126 

// 代表了wglCreateContextAttribsARB的函数签名
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) 
(HDC, HGLRC, const int*);

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#if _DEBUG
// 如果在Debug状态下, 把subsystem设置到console
// 能在开启Main窗口的同时, 在main函数里调用WinMain函数
// 再开启一个Win32的窗口
#pragma comment(linker, "/subsystem:console")
int main(int argc, const char** argv)
{
	return WinMain(GetModuleHandle(NULL),
		NULL, GetCommandLineA(), SW_SHOWDEFAULT);
}
#else
#pragma comment(linker, "/subsystem:windows")
#endif

//#pragma comment(lib, "opengl32.lib")

