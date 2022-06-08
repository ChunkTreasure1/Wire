#include "Registry.h"

#include "Serialization.h"

namespace Wire
{
	Registry::Registry(const Registry& registry)
	{
		m_nextEntityId = registry.m_nextEntityId;
		m_availiableIds = registry.m_availiableIds;
		m_pools = registry.m_pools;
	}

	Registry::~Registry()
	{
		Clear();
	}

	EntityId Registry::CreateEntity()
	{
		EntityId id;
		if (!m_availiableIds.empty())
		{
			id = m_availiableIds.back();
			m_availiableIds.pop_back();
		}
		else
		{
			id = m_nextEntityId++;
		}
		
		m_usedIds.emplace_back(id);

		return id;
	}

	EntityId Registry::AddEntity(EntityId aId)
	{
		assert(aId != 0);
		assert(std::find(m_usedIds.begin(), m_usedIds.end(), aId) == m_usedIds.end());

		if (m_nextEntityId <= aId)
		{
			m_nextEntityId = aId + 1;
		}

		m_usedIds.emplace_back(aId);

		return aId;
	}

	void Registry::AddChild(EntityId parent, EntityId child)
	{
		assert(std::find(m_usedIds.begin(), m_usedIds.end(), parent) != m_usedIds.end());
		assert(std::find(m_usedIds.begin(), m_usedIds.end(), child) != m_usedIds.end());

		auto& children = m_childEntities[parent];
		if (std::find(children.begin(), children.end(), child) == children.end())
		{
			children.emplace_back(child);
		}
	}


	void Registry::RemoveChild(EntityId parent, EntityId child)
	{
		assert(std::find(m_usedIds.begin(), m_usedIds.end(), parent) != m_usedIds.end());
		assert(std::find(m_usedIds.begin(), m_usedIds.end(), child) != m_usedIds.end());

		auto& children = m_childEntities[parent];
		if (auto it = std::find(children.begin(), children.end(), child); it != children.end())
		{
			children.erase(it);
		}
	}

	const std::vector<EntityId>& Registry::GetChildren(EntityId parent) const
	{
		assert(m_childEntities.find(parent) != m_childEntities.end());
		return m_childEntities.at(parent);
	}

	void Registry::RemoveEntity(EntityId aId)
	{
		assert(aId != 0);
		assert(std::find(m_availiableIds.begin(), m_availiableIds.end(), aId) == m_availiableIds.end());
		assert(std::find(m_usedIds.begin(), m_usedIds.end(), aId) != m_usedIds.end());

		auto it = std::find(m_usedIds.begin(), m_usedIds.end(), aId);
		m_usedIds.erase(it);

		for (auto& compPool : m_pools)
		{
			if (compPool.second.HasComponent(aId))
			{
				compPool.second.RemoveComponent(aId);
			}
		}

		m_availiableIds.emplace_back(aId);
	}

	void Registry::Clear()
	{
		m_pools.clear();
		m_availiableIds.clear();
		m_usedIds.clear();
		m_nextEntityId = 1;
	}

	void Registry::AddComponent(const std::vector<uint8_t> data, const WireGUID& guid, EntityId aId)
	{
		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			it->second.AddComponent(aId, data);
		}
		else
		{
			m_pools.emplace(guid, ComponentPool((uint32_t)data.size()));
			m_pools[guid].AddComponent(aId, data);
		}
	}

	std::vector<uint8_t> Registry::GetEntityComponentData(EntityId id) const
	{
		std::vector<uint8_t> data;

		for (const auto& pool : m_pools)
		{
			if (pool.second.HasComponent(id))
			{
				const size_t size = data.size();
				const uint32_t componentSize = pool.second.GetComponentSize();

				data.resize(data.size() + componentSize);

				std::vector<uint8_t> componentData = pool.second.GetComponentData(id);
				memcpy_s(&data[size], componentSize, componentData.data(), componentSize);
			}
		}

		return data;
	}
	std::vector<uint8_t> Registry::GetEntityComponentDataEncoded(EntityId id) const
	{
		std::vector<uint8_t> data;

		for (const auto& pool : m_pools)
		{
			if (pool.second.HasComponent(id))
			{
				const std::string componentName = ComponentRegistry::GetNameFromGUID(pool.first);
				const uint16_t nameSize = (uint16_t)componentName.size();

				size_t size = data.size();
				const uint32_t componentSize = pool.second.GetComponentSize();

				data.resize(data.size() + sizeof(uint16_t) + nameSize + componentSize);

				std::vector<uint8_t> componentData = pool.second.GetComponentData(id);

				memcpy_s(&data[size], sizeof(uint16_t), &nameSize, sizeof(uint16_t));
				size += sizeof(uint16_t);

				memcpy_s(&data[size], nameSize, componentName.data(), nameSize);
				size += nameSize;

				memcpy_s(&data[size], componentSize, componentData.data(), componentSize);
			}
		}
		return data;
	}

	const uint32_t Registry::GetComponentCount(EntityId aId) const
	{
		uint32_t count = 0;

		for (const auto& pool : m_pools)
		{
			if (pool.second.HasComponent(aId))
			{
				count++;
			}
		}
		return count;
	}
}
