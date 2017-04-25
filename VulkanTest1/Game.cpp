#include "Game.h"

#include "Main.h"

#include <functional>

class GamePlaceholder
{
public:
	GamePlaceholder();

private:
	void GameLoopFn(float dt);
};

static GamePlaceholder s_Game;

GamePlaceholder::GamePlaceholder()
{
	Main().SetGameLoopFn(std::bind(&GamePlaceholder::GameLoopFn, this, std::placeholders::_1));
}

void GamePlaceholder::GameLoopFn(float dt)
{
}
