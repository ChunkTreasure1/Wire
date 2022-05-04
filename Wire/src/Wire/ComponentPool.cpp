#include "ComponentPool.hpp"

namespace Wire
{
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
	}
	
	void ComponentPool::Defragment()
	{
		// TODO: implement
		EntityId start = 0;
		EntityId end = 0;

		for (auto it = m_toEntityMap.begin(); it != m_toEntityMap.end(); it++)
		{
			if (std::distance(it, m_toEntityMap.end()) < 2)
			{
				break;
			}

			auto next = std::next(it);
			if (it->second - next->second > m_componentSize)
			{
				start = it->first;
				end = next->first;
				break;
			}
		}

		if (start != end)
		{
			
		}
	}
}