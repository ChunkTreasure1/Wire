#include "Registry.h"

namespace Wire
{
	Registry::~Registry()
	{
		Clear();
	}

	EntityId Registry::CreateEntity()
	{
		return m_nextEntityId++;
	}
	
	void Registry::RemoveEntity(EntityId aId)
	{
		for (auto& compPool : m_pools)
		{
			if (compPool.second.HasComponent(aId))
			{
				compPool.second.RemoveComponent(aId);
			}
		}
	}

	void Registry::Clear()
	{
		m_pools.clear();
		m_nextEntityId = 1;
	}

	void Registry::AddComponent(const std::vector<uint8_t> data, const GUID& guid, EntityId aId)
	{
		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			it->second.AddComponent(aId, data);
		}
		else
		{
			m_pools.emplace(guid, ComponentPool(data.size()));
			m_pools[guid].AddComponent(aId, data);
		}
	}
}
