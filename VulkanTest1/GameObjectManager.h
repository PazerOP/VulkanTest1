#pragma once

class IGameObject;
class GameObjectManagerImpl
{
public:
	GameObjectManagerImpl();

private:
	std::vector<std::shared_ptr<IGameObject>> m_GameObjects;
};

extern GameObjectManagerImpl& GameObjectManager();