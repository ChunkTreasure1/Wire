#pragma once

#include "Entity.h"

#include <vector>
#include <unordered_map>
#include <cassert>

namespace Wire
{
	class ComponentPool
	{
	public:
		ComponentPool() = default;
		ComponentPool(uint32_t aSize);

		void AddComponent(EntityId aId, const std::vector<uint8_t> data);

		template<typename T>
		T& AddComponent(EntityId aId, T& aComponent);

		void RemoveComponent(EntityId aId);

		template<typename T>
		T& GetComponent(EntityId aId);

		bool HasComponent(EntityId aId);

		inline const std::vector<uint8_t>& GetAllComponents() { return m_pool; }

	private:
		void Defragment();

		uint32_t m_componentSize = 0;
		std::vector<uint8_t> m_pool;
		std::unordered_map<EntityId, size_t> m_toEntityMap;
	};

	template<typename T>
	inline T& ComponentPool::AddComponent(EntityId aId, T& aComponent)
	{
		auto it = m_toEntityMap.find(aId);
		assert(it == m_toEntityMap.end());

		size_t index = m_pool.size();
		m_pool.resize(m_pool.size() + sizeof(T));
		memcpy_s(&m_pool[index], sizeof(T), &aComponent, sizeof(T));

		m_toEntityMap[aId] = index;
		
		return *reinterpret_cast<T*>(&m_pool[index]);
	}

	inline void ComponentPool::RemoveComponent(EntityId aId)
	{
		auto it = m_toEntityMap.find(aId);
		assert(it != m_toEntityMap.end());

		const size_t lastComponentIndex = m_pool.size() - m_componentSize;

		memcpy_s(&m_pool[it->second], m_componentSize, &m_pool[lastComponentIndex], m_componentSize);
		m_pool.resize(m_pool.size() - m_componentSize);

		for (auto& newIt : m_toEntityMap)
		{
			if (newIt.second == lastComponentIndex)
			{
				newIt.second = it->second;
				break;
			}
		}

		m_toEntityMap.erase(it);
	}

	template<typename T>
	inline T& ComponentPool::GetComponent(EntityId aId)
	{
		assert(HasComponent<T>(aId));
		return *reinterpret_cast<T*>(&m_pool[m_toEntityMap[aId]]);
	}

	inline bool ComponentPool::HasComponent(EntityId aId)
	{
		auto it = m_toEntityMap.find(aId);
		return it != m_toEntityMap.end();
	}
}