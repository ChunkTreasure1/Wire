#pragma once

#include "Entity.h"
#include "GUID.h"
#include "ComponentPool.hpp"

namespace Wire
{
	// TODO: Serialization

	class Registry
	{
	public:
		Registry() = default;
		~Registry();

		EntityId CreateEntity();

		void RemoveEntity(EntityId aId);
		void Clear();

		void AddComponent(const std::vector<uint8_t> data, const GUID& guid, EntityId aId);

		template<typename T, typename ... Args>
		T& AddComponent(EntityId aEntity, Args&&... args);

		template<typename T>
		T& GetComponent(EntityId aEntity);

		template<typename T>
		bool HasComponent(EntityId aEntity);

		template<typename T>
		void RemoveComponent(EntityId aEntity);

		template<typename T>
		const std::vector<T>& GetAllComponents();

	private:
		std::unordered_map<GUID, ComponentPool> m_pools;
		EntityId m_nextEntityId = 1; // ID zero is null
	};
	
	template<typename T, typename ...Args>
	inline T& Registry::AddComponent(EntityId aEntity, Args && ...args)
	{
		const GUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			T comp(std::forward<Args>(args)...);
			return it->second.AddComponent<T>(aEntity, comp);
		}
		else
		{
			m_pools.emplace(guid, ComponentPool(sizeof(T)));
			
			T comp(std::forward<Args>(args)...);
			return m_pools[guid].AddComponent(aEntity, comp);
		}
	}

	template<typename T>
	inline T& Registry::GetComponent(EntityId aEntity)
	{
		assert(HasComponent<T>());
		const GUID guid = T::comp_guid;

		return m_pools[guid].GetComponent<T>(aEntity);
	}

	template<typename T>
	inline bool Registry::HasComponent(EntityId aEntity)
	{
		const GUID guid = T::comp_guid;
		return m_pools[guid].HasComponent(aEntity);
	}

	template<typename T>
	inline void Registry::RemoveComponent(EntityId aEntity)
	{
		const GUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		assert(it != m_pools.end());

		it->second.RemoveComponent(aEntity);
	}
	
	template<typename T>
	inline const std::vector<T>& Registry::GetAllComponents()
	{
		const GUID guid = T::comp_guid;
		
		auto it = m_pools.find(guid);
		assert(it != m_pools.end());

		return reinterpret_cast<const std::vector<T>&>(it->second.GetAllComponents());
	}
}