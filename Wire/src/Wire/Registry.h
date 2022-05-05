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
		EntityId AddEntity(EntityId aId);

		void RemoveEntity(EntityId aId);
		void Clear();

		void AddComponent(const std::vector<uint8_t> data, const GUID& guid, EntityId id);
		std::vector<uint8_t> GetEntityComponentData(EntityId id) const;
		
		/*
		* First 2 bytes: The length of the name
		* Next X bytes: The name of the component
		* Next X bytes: The data of the component
		*/
		std::vector<uint8_t> GetEntityComponentDataEncoded(EntityId id) const;
		const uint32_t GetComponentCount(EntityId aId) const;

		template<typename T, typename ... Args>
		T& AddComponent(EntityId aEntity, Args&&... args);

		template<typename T>
		T& GetComponent(EntityId aEntity);

		template<typename T>
		bool HasComponent(EntityId aEntity) const;

		template<typename T>
		void RemoveComponent(EntityId aEntity);

		template<typename T>
		const std::vector<T>& GetAllComponents() const;

		template<typename T>
		const std::vector<EntityId>& GetComponentView() const;

	private:
		std::unordered_map<GUID, ComponentPool> m_pools;
		EntityId m_nextEntityId = 1; // ID zero is null

		std::vector<EntityId> m_availiableIds;
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
		assert(HasComponent<T>(aEntity));
		const GUID guid = T::comp_guid;

		return m_pools.at(guid).GetComponent<T>(aEntity);
	}

	template<typename T>
	inline bool Registry::HasComponent(EntityId aEntity) const
	{
		const GUID guid = T::comp_guid;
		return m_pools.at(guid).HasComponent(aEntity);
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
	inline const std::vector<T>& Registry::GetAllComponents() const
	{
		const GUID guid = T::comp_guid;
		
		auto it = m_pools.find(guid);
		assert(it != m_pools.end());

		return reinterpret_cast<const std::vector<T>&>(it->second.GetAllComponents());
	}
	
	template<typename T>
	inline const std::vector<EntityId>& Registry::GetComponentView() const
	{
		const GUID guid = T::comp_guid;

		auto& it = m_pools.find(guid);
		assert(it != m_pools.end());

		return it->second.GetComponentView();
	}
}