#include "stdafx.h"
#include "Game.h"

#include "Main.h"
#include "ShaderModule.h"
#include "Vulkan.h"

#include <functional>

class GamePlaceholder : public IGame
{
public:
	GamePlaceholder();

	void InitGame() override;

private:
	void GameLoopFn(float dt);

	std::shared_ptr<ShaderModule> m_Vertex;
	std::shared_ptr<ShaderModule> m_Pixel;
};

static GamePlaceholder s_Game;
IGame& Game()
{
	return s_Game;
}

GamePlaceholder::GamePlaceholder()
{
	Main().SetGameLoopFn(std::bind(&GamePlaceholder::GameLoopFn, this, std::placeholders::_1));
}

void GamePlaceholder::InitGame()
{
	m_Vertex = std::make_shared<ShaderModule>("shaders/vert.spv");
	m_Pixel = std::make_shared<ShaderModule>("shaders/frag.spv");
}

void GamePlaceholder::GameLoopFn(float /*dt*/)
{
}
