// creates #define constants that reduce the amount of code that is brought in by including <windows.h>:
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "glad.h"
#include <windows.h>
#include <iostream>
#include "Application.h"

#pragma region From <wglext.h>
	// ��Щ��������������OpenGL 3.3 Core context, ֻ�ǰ�glew��ĺ����������˳���
	#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
	#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
	#define WGL_CONTEXT_FLAGS_ARB 0x2094 
	#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001 
	#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126 
	
	// ������wglCreateContextAttribsARB�ĺ���ǩ��
	typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) 
	(HDC, HGLRC, const int*);
#pragma endregion

#pragma region From <wgl.h>
	typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void); 
	typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int); 
	typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);
#pragma endregion


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global variables
Application* gApplication = 0; 
GLuint gVertexArrayObject = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
//int main(int* argc, char** argv)
{
	gApplication = new Application();

	// 1. ���WNDCLASSEX���ʵ��
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	// ��flag��ʾ���style, wdnclass��flag��0011
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	// ǰ���������������ָ��, lpfnWndProc: Long Pointer to the Windows Procedure function.
	// ��ʾwndclass�洢��һ������ϵͳ�����ĺ���ָ��
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszMenuName = 0;
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


