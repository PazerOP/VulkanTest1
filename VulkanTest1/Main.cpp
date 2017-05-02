#include "Main.h"

#include <assert.h>
#include <chrono>
#include <clocale>
#include "FixedWindows.h"
#include <glm/glm.hpp>
#include "Log.h"
#include "StringTools.h"
#include "Vulkan.h"


class _Main final : public IMain
{
public:
	_Main();

	HINSTANCE GetAppInstance() override { return m_AppInstance; }
	void SetAppInstance(HINSTANCE instance) { m_AppInstance = instance; }

	Window& GetAppWindow() override { return m_AppWindow; }

	const GameLoopFn& GetGameLoopFn() { return m_GameLoopFn; }
	void SetGameLoopFn(const GameLoopFn& fn) override { m_GameLoopFn = fn; }

private:

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static void BrushDeleter(HBRUSH brush) { DeleteObject(brush); }

	//static void Init

	Window m_AppWindow;

	HINSTANCE m_AppInstance;
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
	Log::BlockMsg(u8"{00} EPIC MEME START 🔥🔥🔥", u8"🔥🔥🔥");

	try
	{
		LocalMain().SetAppInstance(hInstance);

		LocalMain().GetAppWindow().Show();

		Vulkan().Init();
	}
	catch (std::runtime_error e)
	{
		Log::Msg("Failed to initialize engine: {0}", e.what());
		std::exit(1);
	}

	// Main loop
	{
		auto last = std::chrono::high_resolution_clock::now();
		float dt = 1.0f / 60;
		while (true)
		{
			MSG message;
			memset(&message, 0, sizeof(message));
			while (PeekMessageA(&message, Main().GetAppWindow().GetWindow(), 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessageW(&message);

				memset(&message, 0, sizeof(message));
			}

			LocalMain().GetGameLoopFn()(dt);

			auto current = std::chrono::high_resolution_clock::now();
			dt = std::chrono::duration<float, std::ratio<1, 1>> (current - last).count();
			last = current;

			if (!LocalMain().GetAppWindow().GetWindow())
				break;
		}
	}
}

_Main::_Main()
{
	m_AppInstance = nullptr;

	StringTools::UnitTests();
}