// creates #define constants that reduce the amount of code that is brought in by including <windows.h>:
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "glad.h"
#include <windows.h>
#include <iostream>
#include "Application.h"

// �����ǰ�������OpenGL Context��ͷ�ļ����õ�������
#pragma region From <wglext.h>
	// ��Щ��������������OpenGL 3.3 Core context, ֻ�ǰ�glew��ĺ����������˳���
	#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
	#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
	#define WGL_CONTEXT_FLAGS_ARB 0x2094 
	#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001 
	#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126 
	
	// ������wglCreateContextAttribsARB�ĺ���ǩ��, ������#define WINAPI      __stdcall
	typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) 
	(HDC, HGLRC, const int*);
#pragma endregion

// �����ǰ�������VSync��SwapInterval(Ӧ����SwapBuffer)��ͷ�ļ����õ�������
#pragma region From <wgl.h>
	typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void); 
	typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int); 
	typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);
#pragma endregion


// ʹ����������window messages
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// ��������Ψ�����õ���ȫ�ֱ���, ����Application��VAO
Application* gApplication = 0; 
GLuint gVertexArrayObject = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
//int main(int* argc, char** argv)
{
	// 1. ����Application��
	gApplication = new Application();

	// 2. ���WNDCLASSEX���ʵ��, ���ʾ�����ڴ洢������window�������Ϣ
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(WNDCLASSEX);// cb����count bytes

	// ��flag��ʾ���style, wdnclass��flag��0011, ��window��ˮƽ����ֱ����size�ı�ʱ
	// ʹ��Redraw���»��ƴ���
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	// lpfnWndProc: Long Pointer to the Windows Procedure function. ��һ������ָ��
	// WndProc��ǰ��������, ��ʾwndclass�洢��һ������ϵͳ�����ĺ���ָ��(����Ŀǰ�ǿյ�?)
	wndclass.lpfnWndProc = WndProc;

	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;

	// h����handle, �����Instanceָ��������Application��ʵ��, ��WinMain�ﴫ������
	wndclass.hInstance = hInstance;

	// hIcon is a handle to the application icon. 
	// ������Ӧ���Ǹ�ָ��ṹ���ָ��, ����ṹ��ֻ��һ��int��Ϊpublic��Ա
	// �ҷ��������Handle������ô��װ��, ���ʾ���һ��int
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	
	// hCursor is a handle to the mouse cursor.
	// hCursor��������ʵ����HICON
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	
	// hbrBackground is a handle to the brush which will be used to fill the window background. 
	// We used pre-defined window color to paint background of our window.
	// h����handle, br����brush, �����������Window������brush, ������Ǵ��ڵı�����ɫ
	// COLOR_BTNFACE����һ����ɫ����
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	
	// ����menu��, ��ʱ��֪��ʲô��menu
	wndclass.lpszMenuName = 0;
	
	// ��������, lpszClassName�Ǹ�ָ��const string��ָ��
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

#pragma comment(lib, "opengl32.lib")


