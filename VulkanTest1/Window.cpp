#include "stdafx.h"
#include "Window.h"

#include "Log.h"
#include "Main.h"
#include "Util.h"

std::map<HWND, std::shared_ptr<Window::WindowData>> Window::m_Windows;

Window::Window()
{
}

void Window::Show()
{
	WindowData* const data = GetWindowData();
	if (!data->m_Window)
		CreateWindow();

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

void Window::SetTitle(const std::string& title)
{
	::SetWindowTextA(GetWindow(), title.c_str());
}

std::string Window::GetWindowTitle() const
{
	const auto length = GetWindowTextLengthA(m_WindowData->m_Window.get());
	std::string retVal;
	retVal.resize(length);
	GetWindowTextA(m_WindowData->m_Window.get(), retVal.data(), length);
	return retVal;
}

void Window::SetWindowResizedCallback(const std::function<void(Window&)>& fn)
{
	GetWindowData()->m_OnResizedFn = fn;
}

HWND Window::GetWindow()
{
	WindowData* const data = GetWindowData();
	if (data)
		return data->m_Window.get();

	return nullptr;
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

void Window::CreateWindow()
{
	WindowData* const data = GetWindowData();
	if (data->m_Window)
		return;

	CreateWindowClass();

	auto newWindow = CreateWindowExA(
		CS_HREDRAW | CS_VREDRAW,
		WINDOW_CLASS_NAME,
		"RKRP Game",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		Main().GetAppInstance(),
		nullptr
	);

	auto deleter = [](HWND hWnd)
	{
		auto it = m_Windows.find(hWnd);
		if (it != m_Windows.end() && !it->second->m_WindowDestroyed)
			AssertAR(, DestroyWindow(hWnd), );
	};
	data->m_Window = shared_hwnd(newWindow, deleter);
	data->m_WindowDestroyed = false;

	m_Windows.insert(std::make_pair(data->m_Window.get(), m_WindowData));
}

HBRUSH Window::GetBackgroundBrush()
{
	WindowData* const data = GetWindowData();

	if (!data->m_BackgroundBrush)
		data->m_BackgroundBrush.reset(CreateSolidBrush(RGB(0, 255, 0)));

	return data->m_BackgroundBrush.get();
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
		auto it = m_Windows.find(hWnd);
		assert(it != m_Windows.end());
		if (it != m_Windows.end())
		{
			auto windowData = it->second;
			assert(windowData);
			assert(!windowData->m_WindowDestroyed);
			if (windowData)
			{
				windowData->m_WindowDestroyed = true;
				windowData->m_Window.reset();
			}

			m_Windows.erase(it);
		}

		PostQuitMessage(0);
		return 0;
	}

	case WM_ENTERSIZEMOVE:
	{
		const auto data = FindWindowData(hWnd);
		assert(!data->m_IsResizing);
		data->m_IsResizing = true;

		GetWindowRect(hWnd, &data->m_StartRect);

		break;
	}

	case WM_SIZE:
	case WM_EXITSIZEMOVE:
	{
		const auto data = FindWindowData(hWnd);
		data->m_IsResizing = false;

		if (data->m_OnResizedFn)
		{
			RECT endRect;
			GetWindowRect(hWnd, &endRect);

			const auto startH = data->m_StartRect.bottom - data->m_StartRect.top;
			const auto startW = data->m_StartRect.right - data->m_StartRect.left;
			const auto endH = endRect.bottom - endRect.top;
			const auto endW = endRect.right - endRect.left;

			if (startH != endH || startW != endW)
			{
				// Window was resized
				Window temp(data);
				data->m_OnResizedFn(temp);
			}
		}

		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

Window::WindowData* Window::GetWindowData()
{
	if (!m_WindowData)
		m_WindowData.reset(new WindowData());

	assert(m_WindowData);
	return m_WindowData.get();
}

Window::Window(const std::shared_ptr<WindowData>& data)
{
	m_WindowData = data;
}

std::shared_ptr<Window::WindowData> Window::FindWindowData(HWND window)
{
	auto it = m_Windows.find(window);
	if (it == m_Windows.end())
		return nullptr;

	return it->second;
}

Window::WindowData::WindowData() :
	m_BackgroundBrush(nullptr, &BrushDeleter)
{
	m_WindowDestroyed = true;
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
