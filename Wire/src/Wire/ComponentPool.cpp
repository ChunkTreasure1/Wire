#include "ComponentPool.hpp"

namespace Wire
{
	ComponentPool::ComponentPool(const ComponentPool& pool)
	{
		m_componentSize = pool.m_componentSize;
		m_pool = pool.m_pool;
		m_entitiesWithComponent = pool.m_entitiesWithComponent;
		m_toEntityMap = pool.m_toEntityMap;
	}

	ComponentPool::ComponentPool(uint32_t aSize)
		: m_componentSize(aSize)
	{
		m_pool.reserve(aSize * 100);
	}

	void ComponentPool::AddComponent(EntityId aId, const std::vector<uint8_t> data)
	{
		assert(!HasComponent(aId));
		
		size_t index = m_pool.size();
		m_pool.resize(m_pool.size() + data.size());
		memcpy_s(&m_pool[index], data.size(), data.data(), data.size());
		
		m_toEntityMap[aId] = index;
		m_entitiesWithComponent.emplace_back(aId);
	}	

	void ComponentPool::SetComponentData(const std::vector<uint8_t>& data, EntityId aId)
	{
		assert(HasComponent(aId));
		memcpy_s(&m_pool[m_toEntityMap.at(aId)], m_componentSize, data.data(), data.size());
	}
}