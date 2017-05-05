#pragma once

class IGame
{
public:
	virtual ~IGame() = default;

	virtual void InitGame() = 0;
};

extern IGame& Game();