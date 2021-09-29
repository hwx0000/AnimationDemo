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
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) { return NULL; }

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

	// Win32的窗口登记
	RegisterClassEx(&wndclass);

	// 获取屏幕尺寸, 我这个电脑返回的是1920*1080
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);//CX
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);//CY

	int clientWidth = 800;
	int clientHeight = 600;
	RECT windowRect;
	// 四个值代表离屏幕四个方向上的间距, 顺序是左上右下, 这样写是为了让窗口在屏幕正中心
	SetRect(&windowRect, (screenWidth / 2) - (clientWidth / 2),
		(screenHeight / 2) - (clientHeight / 2),
		(screenWidth / 2) + (clientWidth / 2),
		(screenHeight / 2) + (clientHeight / 2));

	// WS的意思是Window Style, 这些Flag都是一些窗口UI的设置
	// WS_MINIMIZEBOX: 窗口有最小化的按钮
	// WS_SYSMENU: The window has a window menu on its title bar
	DWORD style = (WS_OVERLAPPED | WS_CAPTION |
		WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	// | WS_THICKFRAME to resize

	// 调整Window的Rect
	AdjustWindowRectEx(&windowRect, style, FALSE, 0);
	// 根据WindowInfo和rect创建真正的Window
	HWND hwnd = CreateWindowEx(0, wndclass.lpszClassName, "Game Window", wndclass.style, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom -
		windowRect.top, NULL, NULL, hInstance, szCmdLine);
	HDC hdc = GetDC(hwnd);// DC应该是Device吧

	// 创建pixel format descriptor, 主要是进行绘制的相关设置
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
	// 这个函数会尝试去在输入的device context里寻找, 为了找到与输入的pfd最匹配的pixel format
	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	// 设置device context的pfd
	SetPixelFormat(hdc, pixelFormat, &pfd);

	// create a temporary OpenGL context using wglCreateContext
	HGLRC tempRC = wglCreateContext(hdc);// 创建Render Context
	wglMakeCurrent(hdc, tempRC);// 绑定到device上
	// PFNWGLCREATECONTEXTATTRIBSARBPROC代表了wglCreateContextAttribsARB的函数签名
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;// 创建一个空的函数指针
	// 通过wglGetProcAddress对函数寻址, 然后赋值
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateCon textAttribsARB");

	// A temporary OpenGL context exists and is bound, so call the wglCreateContextAttribsARB function next.
	// This function will return an OpenGL 3.3 Core context profile, bind it,
	// and delete the legacy context :
	// 这是创建OpenGL Contex的相关参数
	const int attribList[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,0, };
	// 调用获取来的函数指针
	HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
	// 重新绑定到Null的Render Context上
	wglMakeCurrent(NULL, NULL);
	// 删除遗留的临时Render Context
	// 问题, 这里创建一个临时的RC, 然后创建真正的RC之后, 又删掉这个临时的RC, 这是何必?
	wglDeleteContext(tempRC);
	// 绑定到新创建的RenderContext上
	wglMakeCurrent(hdc, hglrc);


	if (!gladLoadGL())
		std::cout << "Could not initialize GLAD\n";
	else
		std::cout << "OpenGL Version " << GLVersion.major << "." << GLVersion.minor << "\n";

	// 设置vsync
	// 对应的设置函数不属于核心函数, 所以要去寻址, 名字是wglGetExtensionsStringEXT
	PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT =
		(PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
	// 这个函数会返回一个string, 这里通过是否以"WGL_EXT_swap_control"结尾来判断是否支持vsync
	// strstr: Returns a pointer to the first occurrence of str2 in str1, or a null pointer if str2 is not part of str1.
	// 会返回右边的字符串在左边字符串的位置的指针
	bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;

	int vsynch = 0;
	if (swapControlSupported)
	{
		// 获取两个函数的地址: wglSwapIntervalEXT和wglGetSwapIntervalEXT
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
			(PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT =
			(PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");		if (wglSwapIntervalEXT(1))
		{
			std::cout << "Enabled vsynch\n";
			vsynch = wglGetSwapIntervalEXT();
		}
		else
			std::cout << "Could not enable vsynch\n";
	}
	else
	{
		//!swapControlSupported
		std::cout << "WGL_EXT_swap_control not supported\n";
	}	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	gApplication->Init();	DWORD lastTick = GetTickCount();
	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);			DispatchMessage(&msg);
		}		DWORD thisTick = GetTickCount();
		float dt = float(thisTick - lastTick) * 0.001f;
		lastTick = thisTick;
		if (gApplication != 0)
		{
			gApplication->Update(dt);
		}		if (gApplication != 0)
		{
			RECT clientRect;
			GetClientRect(hwnd, &clientRect);
			clientWidth = clientRect.right - clientRect.left;
			clientHeight = clientRect.bottom - clientRect.top;
			glViewport(0, 0, clientWidth, clientHeight);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glPointSize(5.0f);
			glBindVertexArray(gVertexArrayObject);
			glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			float aspect = (float)clientWidth / (float)clientHeight;
			gApplication->Render(aspect);
		}		if (gApplication != 0)
		{
			SwapBuffers(hdc);
			if (vsynch != 0)
				glFinish();
		}
	}// End Game Loop	if (gApplication != 0)
	{
		std::cout << "Expectedapplication to be null on exit\n";
		delete gApplication;
	}

	return (int)msg.wParam;}

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


