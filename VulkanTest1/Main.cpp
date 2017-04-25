#include "Main.h"

#include "Log.h"

#include <assert.h>
#include "FixedWindows.h"

#pragma comment(lib, "vulkan-1.lib")
#include <vulkan\vulkan.hpp>

class _Main final : public IMain
{
public:
	_Main();

	HINSTANCE GetAppInstance() override { return m_AppInstance; }
	void SetAppInstance(HINSTANCE instance) { m_AppInstance = instance; }

	HWND GetAppWindow() override { return m_AppWindow; }

	void VulkanInit();

	void CreateWindowClass();
	void CreateWindow();

	const GameLoopFn& GetGameLoopFn() { return m_GameLoopFn; }
	void SetGameLoopFn(const GameLoopFn& fn) { m_GameLoopFn = fn; }

private:
	static constexpr char WINDOW_CLASS_NAME[] = "RKRPWindowClass";

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static void BrushDeleter(HBRUSH brush) { DeleteObject(brush); }

	HINSTANCE m_AppInstance;
	HWND m_AppWindow;
	std::unique_ptr<std::remove_pointer_t<HBRUSH>, decltype(&BrushDeleter)> m_BackgroundBrush;
	std::function<void(float)> m_GameLoopFn;
};

_Main& LocalMain()
{
	static _Main s_Main;
	return s_Main;
}
IMain& Main() { return LocalMain(); }

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
	LocalMain().SetAppInstance(hInstance);

	LocalMain().CreateWindowClass();
	LocalMain().CreateWindow();

	LocalMain().VulkanInit();

	// Main loop
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		LARGE_INTEGER last;
		QueryPerformanceCounter(&last);
		float dt = 1.0f / 60;
		while (true)
		{
			MSG message;
			memset(&message, 0, sizeof(message));
			while (PeekMessageA(&message, Main().GetAppWindow(), 0, 0, PM_REMOVE) != 0)
			{
				TranslateMessage(&message);
				DispatchMessageW(&message);

				memset(&message, 0, sizeof(message));
			}

			LocalMain().GetGameLoopFn()(dt);

			LARGE_INTEGER current;
			QueryPerformanceCounter(&current);

			dt = (current.QuadPart - last.QuadPart) / float(frequency.QuadPart);
			last = current;
		}
	}
}

_Main::_Main() : m_BackgroundBrush(nullptr, BrushDeleter)
{
	m_AppInstance = nullptr;
	m_AppWindow = nullptr;
}

void _Main::VulkanInit()
{
	uint32_t extensionCount = 0;
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::unique_ptr<vk::ExtensionProperties> props(new vk::ExtensionProperties[extensionCount]);
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, props.get());

	Log::Msg("Extensions supported: %i", extensionCount);
	for (size_t i = 0; i < extensionCount; i++)
	{
		const vk::ExtensionProperties& prop = props.get()[i];

		Log::Msg("    Extension: %s (v%i)", prop.extensionName, prop.specVersion);
	}
}

void _Main::CreateWindowClass()
{
	WNDCLASSEXA windowClass;
	bool existing = GetClassInfoExA(Main().GetAppInstance(), WINDOW_CLASS_NAME, &windowClass);
	if (existing)
		return;

	m_BackgroundBrush.reset(CreateSolidBrush(RGB(0, 255, 0)));

	memset(&windowClass, 0, sizeof(windowClass));
	windowClass.cbSize = sizeof(windowClass);
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hbrBackground = m_BackgroundBrush.get();
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.lpfnWndProc = &WndProc;

	ATOM windowClassAtom = RegisterClassExA(&windowClass);
}

void _Main::CreateWindow()
{
	m_AppWindow = CreateWindowExA(
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
	);
}

LRESULT _Main::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

