#pragma once

#include "FixedWindows.h"

#include <functional>

class IMain
{
public:
	typedef std::function<void(float)> GameLoopFn;

	virtual ~IMain() = default;

	virtual HINSTANCE GetAppInstance() = 0;
	virtual HWND GetAppWindow() = 0;

	virtual void SetGameLoopFn(const GameLoopFn& gameLoop) = 0;
};

extern IMain& Main();