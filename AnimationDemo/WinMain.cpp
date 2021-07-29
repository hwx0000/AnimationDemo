// ʹ����Щdefine������ʹ��windows.h��Ҫ��д�Ķ���Ĵ���
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "glad.h"
#include <windows.h>
#include <iostream>
#include "Application.h"

// ��Щ��������������OpenGL 3.3 Core context, ֻ�ǰ�glew��ĺ����������˳���
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
#define WGL_CONTEXT_FLAGS_ARB 0x2094 
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001 
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126 

// ������wglCreateContextAttribsARB�ĺ���ǩ��
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) 
(HDC, HGLRC, const int*);

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#if _DEBUG
// �����Debug״̬��, ��subsystem���õ�console
// ���ڿ���Main���ڵ�ͬʱ, ��main���������WinMain����
// �ٿ���һ��Win32�Ĵ���
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

