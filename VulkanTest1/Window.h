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

	HWND GetWindow();

private:
	static constexpr char WINDOW_CLASS_NAME[] = "RKRPWindowClass";

	void CreateWindowClass();
	HBRUSH GetBackgroundBrush();

	static void WindowDeleter(HWND window);
	static void BrushDeleter(HBRUSH brush);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	struct WindowData
	{
		WindowData();
		~WindowData();

		std::unique_ptr<std::remove_pointer_t<HWND>, decltype(&WindowDeleter)> m_Window;
		std::unique_ptr<std::remove_pointer_t<HBRUSH>, decltype(&BrushDeleter)> m_BackgroundBrush;
		bool m_WindowDestroyed;
	};
	std::shared_ptr<WindowData> m_WindowData;
	WindowData* GetWindowData();

	static std::shared_ptr<WindowData> FindWindowData(HWND window);
	static std::map<HWND, std::shared_ptr<WindowData>> m_Windows;
};