#include "Registry.h"

namespace Wire
{
	Registry::~Registry()
	{
		Clear();
	}

	EntityId Registry::CreateEntity()
	{
		return myNextEntityId++;
	}
	
	void Registry::RemoveEntity(EntityId aId)
	{
		for (auto& compPool : myPools)
		{
			if (compPool.second.HasComponent(aId))
			{
				compPool.second.RemoveComponent(aId);
			}
		}
	}

	void Registry::Clear()
	{
		myPools.clear();
		myNextEntityId = 1;
	}
}
