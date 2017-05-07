#pragma once
#include "FixedWindows.h"
#include <map>
#include <memory>

class Window
{
public:
	Window();
	Window(const Window& other) = delete;

	void Show();
	void Hide();
	void Maximize();
	void Minimize();

	bool Destroyed() const;

	void SetTitle(const std::string& title);
	std::string GetWindowTitle() const;

	void SetWindowResizedCallback(const std::function<void(Window&)>& fn);

	HWND GetWindow();

private:
	static constexpr char WINDOW_CLASS_NAME[] = "RKRPWindowClass";

	void CreateWindowClass();
	void CreateWindow();
	HBRUSH GetBackgroundBrush();

	static void BrushDeleter(HBRUSH brush);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	using shared_hwnd = std::shared_ptr<std::remove_pointer_t<HWND>>;

	struct WindowData
	{
		WindowData();
		~WindowData();

		shared_hwnd m_Window;
		std::unique_ptr<std::remove_pointer_t<HBRUSH>, decltype(&BrushDeleter)> m_BackgroundBrush;
		bool m_WindowDestroyed;

		bool m_IsResizing;
		RECT m_StartRect;

		std::function<void(Window&)> m_OnResizedFn;
	};
	std::shared_ptr<WindowData> m_WindowData;
	WindowData* GetWindowData();

	Window(const std::shared_ptr<WindowData>& data);

	static std::shared_ptr<WindowData> FindWindowData(HWND window);
	static std::map<HWND, std::shared_ptr<WindowData>> m_Windows;
};