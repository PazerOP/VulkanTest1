#include "Main.h"

#include <assert.h>
#include <chrono>
#include "FixedWindows.h"
#include <glm/glm.hpp>
#include "Log.h"
#include "StringTools.h"
#include <vulkan\vulkan.hpp>

#pragma comment(lib, "vulkan-1.lib")

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
	void SetGameLoopFn(const GameLoopFn& fn) override { m_GameLoopFn = fn; }

	vk::Instance& GetVKInstance() override { return m_VKInstance; }

private:
	static constexpr char WINDOW_CLASS_NAME[] = "RKRPWindowClass";

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static void BrushDeleter(HBRUSH brush) { DeleteObject(brush); }
	static void VKInstanceDestroyer(vk::Instance* obj) { obj->destroy(); }

	HINSTANCE m_AppInstance;
	HWND m_AppWindow;
	std::unique_ptr<std::remove_pointer_t<HBRUSH>, decltype(&BrushDeleter)> m_BackgroundBrush;
	std::function<void(float)> m_GameLoopFn;

	vk::Instance m_VKInstance;
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
	try
	{
		LocalMain().SetAppInstance(hInstance);

		LocalMain().CreateWindowClass();
		LocalMain().CreateWindow();

		LocalMain().VulkanInit();
	}
	catch (std::runtime_error e)
	{
		Log::Msg("Failed to initialize engine: %s", e.what());
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
			while (PeekMessageA(&message, Main().GetAppWindow(), 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessageW(&message);

				memset(&message, 0, sizeof(message));
			}

			LocalMain().GetGameLoopFn()(dt);

			auto current = std::chrono::high_resolution_clock::now();
			dt = std::chrono::duration<float, std::ratio<1, 1>> (current - last).count();
			last = current;

			if (!LocalMain().GetAppWindow())
				break;
		}
	}
}

_Main::_Main() :
	m_BackgroundBrush(nullptr, BrushDeleter)
{
	m_AppInstance = nullptr;
	m_AppWindow = nullptr;

	StringTools::UnitTests();
}

void _Main::VulkanInit()
{
	// Enumerate extensions
	{
		uint32_t extensionCount = 0;
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::unique_ptr<vk::ExtensionProperties> props(new vk::ExtensionProperties[extensionCount]);
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, props.get());

		Log::Msg("Vulkan Extensions supported: {0}", extensionCount);
		for (size_t i = 0; i < extensionCount; i++)
		{
			const vk::ExtensionProperties& prop = props.get()[i];

			Log::Msg("\tExtension: {0} (v{1})", prop.extensionName, prop.specVersion);
		}
	}

	// Enumerate layers
	std::vector<const char*> validationLayers;
	{
		auto layerProperties = vk::enumerateInstanceLayerProperties();

		Log::Msg("Vulkan instance layers supported: {0}", layerProperties.size());
		for (const auto& layer : layerProperties)
		{
			Log::Msg("\tLayer: {0} (v{1}) - {2}", layer.layerName, layer.implementationVersion, layer.description);

			static constexpr char PARAMETER_VALIDATION_LAYER[] = "VK_LAYER_LUNARG_parameter_validation";
			static constexpr char STANDARD_VALIDATION_LAYER[] = "VK_LAYER_LUNARG_standard_validation";

			if (!strcmp(layer.layerName, PARAMETER_VALIDATION_LAYER))
				validationLayers.push_back(PARAMETER_VALIDATION_LAYER);
			else if (!strcmp(layer.layerName, STANDARD_VALIDATION_LAYER))
				validationLayers.push_back(STANDARD_VALIDATION_LAYER);
		}
	}

	// Application info
	vk::ApplicationInfo appInfo;
	{
		appInfo.setPEngineName("RKRP Engine 3 - Test");
		appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

		appInfo.setPApplicationName("Engine Test App");
		appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));

		appInfo.setApiVersion(VK_API_VERSION_1_0);
	}

	// Init instance
	{
		vk::InstanceCreateInfo info;

		info.setPApplicationInfo(&appInfo);

		info.setEnabledLayerCount(validationLayers.size());
		info.setPpEnabledLayerNames(validationLayers.data());

		std::array<const char*, 2> extensions =
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
		};

		info.setEnabledExtensionCount(extensions.size());
		info.setPpEnabledExtensionNames(extensions.data());

		m_VKInstance = vk::createInstance(info);

		Log::Msg(L"██████████████████████████████████████████████");
		Log::Msg(L"████ Successfully created Vulkan instance ████");
		Log::Msg(L"██████████████████████████████████████████████");
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
	case WM_DESTROY:
		LocalMain().m_AppWindow = nullptr;
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

