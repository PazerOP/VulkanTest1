#pragma once

#include "FixedWindows.h"

#include <functional>

#include "Window.h"

struct GlobalValues;

class IMain
{
public:
	typedef std::function<void(float)> GameLoopFn;

	virtual ~IMain() = default;

	virtual HINSTANCE GetAppInstance() = 0;
	virtual Window& GetAppWindow() = 0;

	virtual void SetGameLoopFn(const GameLoopFn& gameLoop) = 0;

	virtual const GlobalValues& GetGlobals() const = 0;
};

extern IMain& Main();
__forceinline const GlobalValues& Globals() { return Main().GetGlobals(); }