// creates #define constants that reduce the amount of code that is brought in by including <windows.h>:
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "glad.h"
#include <windows.h>
#include <iostream>
#include "Application.h"

// 这里是帮助构建OpenGL Context的头文件里用到的内容
#pragma region From <wglext.h>
	// 这些常量帮助构建起OpenGL 3.3 Core context, 只是把glew里的核心内容拿了出来
	#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
	#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
	#define WGL_CONTEXT_FLAGS_ARB 0x2094 
	#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001 
	#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126 
	
	// 代表了wglCreateContextAttribsARB的函数签名, 这里有#define WINAPI      __stdcall
	typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) 
	(HDC, HGLRC, const int*);
#pragma endregion

// 这里是帮助设置VSync和SwapInterval(应该是SwapBuffer)的头文件里用到的内容
#pragma region From <wgl.h>
	typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void); 
	typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int); 
	typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);
#pragma endregion


// 使用它来处理window messages
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 整个程序唯二会用到的全局变量, 代表Application和VAO
Application* gApplication = 0; 
GLuint gVertexArrayObject = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
//int main(int* argc, char** argv)
{
	// 1. 创建Application类
	gApplication = new Application();

	// 2. 填充WNDCLASSEX类的实例, 这个示例用于存储创建的window的相关信息
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(WNDCLASSEX);// cb代表count bytes

	// 用flag表示类的style, wdnclass的flag是0011, 当window的水平或竖直方向size改变时
	// 使用Redraw重新绘制窗口
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	// lpfnWndProc: Long Pointer to the Windows Procedure function. 是一个函数指针
	// WndProc是前面声明的, 表示wndclass存储了一个调用系统函数的函数指针(好像目前是空的?)
	wndclass.lpfnWndProc = WndProc;

	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;

	// h代表handle, 这里的Instance指的是整个Application的实例, 从WinMain里传进来的
	wndclass.hInstance = hInstance;

	// hIcon is a handle to the application icon. 
	// 本质上应该是个指向结构体的指针, 这个结构体只有一个int作为public成员
	// 我发现这里的Handle都是这么包装的, 本质就是一个int
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	
	// hCursor is a handle to the mouse cursor.
	// hCursor的类型其实就是HICON
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	
	// hbrBackground is a handle to the brush which will be used to fill the window background. 
	// We used pre-defined window color to paint background of our window.
	// h代表handle, br代表brush, 代表用于填充Window背景的brush, 好像就是窗口的背景颜色
	// COLOR_BTNFACE代表一种颜色类型
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	
	// 代表menu名, 暂时不知道什么是menu
	wndclass.lpszMenuName = 0;
	
	// 代表类名, lpszClassName是个指向const string的指针
	wndclass.lpszClassName = "Win32 Game Window";
	RegisterClassEx(&wndclass);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int clientWidth = 800;
	int clientHeight = 600;
	RECT windowRect;
	SetRect(&windowRect, (screenWidth / 2) - (clientWidth / 2),
		(screenHeight / 2) - (clientHeight / 2),
		(screenWidth / 2) + (clientWidth / 2),
		(screenHeight / 2) + (clientHeight / 2));

	AdjustWindowRectEx(&windowRect, wndclass.style, FALSE, 0);
	HWND hwnd = CreateWindowEx(0, wndclass.lpszClassName, "Game Window", wndclass.style, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom -
		windowRect.top, NULL, NULL, hInstance, szCmdLine);
	HDC hdc = GetDC(hwnd);

	// 6
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 32;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixelFormat, &pfd);

	//7.create a temporary OpenGL context using wglCreateContext
	HGLRC tempRC = wglCreateContext(hdc); wglMakeCurrent(hdc, tempRC);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateCon textAttribsARB");

	//8. A temporary OpenGL context exists and is bound, so call the wglCreateContextAttribsARB function next.This function will return an OpenGL 3.3 Core context profile, bind it,
	//and delete the legacy context :
	const int attribList[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,0, };
	HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tempRC);
	wglMakeCurrent(hdc, hglrc);

	if (!gladLoadGL()) { std::cout << "Could not initialize GLAD\n"; }
	else {
		std::cout << "OpenGL Version " << GLVersion.major << "." << GLVersion.minor << "\n";
	}
}

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

#pragma comment(lib, "opengl32.lib")


