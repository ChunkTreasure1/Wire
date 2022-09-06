#pragma once

#include "Entity.h"
#include "Destructor.h"

#include <vector>
#include <functional>
#include <unordered_map>
#include <cassert>

namespace Wire
{
	class ComponentPool
	{
	public:
		ComponentPool() = default;
		ComponentPool(const ComponentPool& pool);
		ComponentPool(uint32_t aSize);
		~ComponentPool();

		void AddComponent(EntityId aId, const std::vector<uint8_t> data);
		inline void SetDestructor(std::shared_ptr<DestructorBase> destructor) { m_destructor = destructor; }

		template<typename T>
		T& AddComponent(EntityId aId);

		void RemoveComponent(EntityId aId);

		template<typename T>
		T& GetComponent(EntityId aId);

		// Copies the data
		std::vector<uint8_t> GetComponentData(EntityId aId) const;
		void SetComponentData(const std::vector<uint8_t>& data, EntityId aId);

		bool HasComponent(EntityId aId) const;

		inline const std::vector<uint8_t>& GetAllComponents() const { return m_pool; }
		inline const uint32_t GetComponentSize() const { return m_componentSize; }
		inline const std::vector<EntityId>& GetComponentView() const { return m_entitiesWithComponent; }
		
		inline void SetOnCreateComponent(std::function<void(void*)>&& func) { m_onCreate = func; }
		inline void SetOnRemoveComponent(std::function<void(void*)>&& func) { m_onRemove = func; }

	private:
		uint32_t m_componentSize = 0;
		std::vector<uint8_t> m_pool;
		std::vector<EntityId> m_entitiesWithComponent;
		std::unordered_map<EntityId, size_t> m_toEntityMap;

		std::shared_ptr<DestructorBase> m_destructor;

		std::function<void(void*)> m_onCreate;
		std::function<void(void*)> m_onRemove;
	};

	template<typename T>
	inline T& ComponentPool::AddComponent(EntityId aId)
	{
		auto it = m_toEntityMap.find(aId);
		assert(it == m_toEntityMap.end());

		size_t index = m_pool.size();
		m_pool.resize(m_pool.size() + sizeof(T));

		T* newComp = new(&m_pool[index]) T();

		m_toEntityMap[aId] = index;
		m_entitiesWithComponent.emplace_back(aId);

		if (m_onCreate)
		{
			m_onCreate(&m_pool[index]);
		}

		return *reinterpret_cast<T*>(&m_pool[index]);
	}

	inline void ComponentPool::RemoveComponent(EntityId aId)
	{
		auto it = m_toEntityMap.find(aId);
		assert(it != m_toEntityMap.end());

		if (m_onRemove)
		{
			m_onRemove(&m_pool[it->second]);
		}

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

		m_entitiesWithComponent.erase(std::find(m_entitiesWithComponent.begin(), m_entitiesWithComponent.end(), aId));
		m_toEntityMap.erase(it);
	}

	template<typename T>
	inline T& ComponentPool::GetComponent(EntityId aId)
	{
		assert(HasComponent(aId));
		return *reinterpret_cast<T*>(&m_pool.at(m_toEntityMap.at(aId)));
	}

	inline std::vector<uint8_t> ComponentPool::GetComponentData(EntityId aId) const
	{
		assert(HasComponent(aId));
		std::vector<uint8_t> data;
		data.resize(m_componentSize);

		memcpy_s(data.data(), m_componentSize, &m_pool[m_toEntityMap.at(aId)], m_componentSize);
		return data;
	}

	inline bool ComponentPool::HasComponent(EntityId aId) const
	{
		auto it = m_toEntityMap.find(aId);
		return it != m_toEntityMap.end();
	}
}