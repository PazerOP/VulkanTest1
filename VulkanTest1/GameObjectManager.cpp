#include "stdafx.h"
#include "GameObjectManager.h"

GameObjectManagerImpl& GameObjectManager()
{
	static GameObjectManagerImpl s_GameObjectManager;
	return s_GameObjectManager;
}

GameObjectManagerImpl::GameObjectManagerImpl()
{
}
