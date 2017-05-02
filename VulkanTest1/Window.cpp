#include "Window.h"

#include "Log.h"
#include "Main.h"
#include "Util.h"

#pragma comment(lib, "vulkan-1.lib")

std::map<HWND, std::shared_ptr<Window::WindowData>> Window::m_Windows;

Window::Window()
{
}

void Window::Show()
{
	::ShowWindow(GetWindow(), SW_SHOW);
}

void Window::Hide()
{
	::ShowWindow(GetWindow(), SW_HIDE);
}

void Window::Maximize()
{
	::ShowWindow(GetWindow(), SW_MAXIMIZE);
}

void Window::Minimize()
{
	::ShowWindow(GetWindow(), SW_MINIMIZE);
}

bool Window::Destroyed() const
{
	if (!m_WindowData)
		return false;

	return m_WindowData->m_WindowDestroyed;
}

HWND Window::GetWindow()
{
	WindowData* const data = GetWindowData();

	if (!data->m_Window)
	{
		CreateWindowClass();

		data->m_Window.reset(CreateWindowExA(
			CS_HREDRAW | CS_VREDRAW,
			WINDOW_CLASS_NAME,
			"RKRP Game",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			nullptr,
			nullptr,
			Main().GetAppInstance(),
			nullptr
		));

		data->m_WindowDestroyed = false;
	}

	return data->m_Window.get();
}

void Window::CreateWindowClass()
{
	WNDCLASSEXA windowClass;
	bool existing = GetClassInfoExA(Main().GetAppInstance(), WINDOW_CLASS_NAME, &windowClass);
	if (existing)
		return;
	
	memset(&windowClass, 0, sizeof(windowClass));
	windowClass.cbSize = sizeof(windowClass);
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hbrBackground = GetBackgroundBrush();
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.lpfnWndProc = &WndProc;

	AssertAR(, RegisterClassExA(&windowClass), );
}

HBRUSH Window::GetBackgroundBrush()
{
	WindowData* const data = GetWindowData();

	if (!data->m_BackgroundBrush)
		data->m_BackgroundBrush.reset(CreateSolidBrush(RGB(0, 255, 0)));
	
	return data->m_BackgroundBrush.get();
}

void Window::WindowDeleter(HWND window)
{
	AssertAR(, DestroyWindow(window), );
}

void Window::BrushDeleter(HBRUSH brush)
{
	AssertAR(, DeleteObject(brush), );
}

LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		auto windowData = FindWindowData(hWnd);
		assert(windowData);
		if (windowData)
		{
			windowData->m_Window.reset();
			windowData->m_WindowDestroyed = true;
		}

		PostQuitMessage(0);
		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

Window::WindowData* Window::GetWindowData()
{
	if (!m_WindowData)
		m_WindowData.reset(new WindowData());

	assert(m_WindowData);
	return m_WindowData.get();
}

std::shared_ptr<Window::WindowData> Window::FindWindowData(HWND window)
{
	auto it = m_Windows.find(window);
	if (it == m_Windows.end())
		return nullptr;

	return it->second;
}

Window::WindowData::WindowData() :
	m_Window(nullptr, &WindowDeleter),
	m_BackgroundBrush(nullptr, &BrushDeleter)
{
	m_WindowDestroyed = false;
}

Window::WindowData::~WindowData()
{
	m_Window.reset();
	m_BackgroundBrush.reset();

	// Try unregistering the window class
	if (UnregisterClassA(WINDOW_CLASS_NAME, Main().GetAppInstance()))
		Log::Msg("{0}: Successfully unregistered window class {1}"sv, __FUNCTION__, WINDOW_CLASS_NAME);
	else
		Log::Msg("{0}: Failed to unregister window class {1}"sv, __FUNCTION__, WINDOW_CLASS_NAME);
}
