#pragma once

#include "Entity.h"
#include "WireGUID.h"
#include "ComponentPool.hpp"

namespace Wire
{
	// TODO: Serialization

	class Registry
	{
	public:
		Registry() = default;
		Registry(const Registry& registry);
		~Registry();

		EntityId CreateEntity();
		EntityId AddEntity(EntityId aId);

		void AddChild(EntityId parent, EntityId child);
		void RemoveChild(EntityId parent, EntityId child);

		inline const std::vector<EntityId>& GetAllEntities() const { return m_usedIds; }
		const std::vector<EntityId>& GetChildren(EntityId parent) const;

		void RemoveEntity(EntityId aId);
		void Clear();

		void AddComponent(const std::vector<uint8_t> data, const WireGUID& guid, EntityId id);
		bool HasComponent(const WireGUID& guid, EntityId id);

		std::vector<uint8_t> GetEntityComponentData(EntityId id) const;

		/*
		* First 2 bytes: The length of the name
		* Next X bytes: The name of the component
		* Next X bytes: The data of the component
		*/
		std::vector<uint8_t> GetEntityComponentDataEncoded(EntityId id) const;
		const uint32_t GetComponentCount(EntityId aId) const;

		template<typename T>
		T& AddComponent(EntityId aEntity);

		template<typename T>
		T& GetComponent(EntityId aEntity);

		template<typename T>
		bool HasComponent(EntityId aEntity) const;

		template<typename ... T>
		bool HasComponents(EntityId aEntity) const;

		template<typename T>
		void RemoveComponent(EntityId aEntity);

		template<typename T>
		const std::vector<T>& GetAllComponents() const;

		std::unordered_map<WireGUID, std::vector<uint8_t>> GetComponents(EntityId aEntity) const;
		void SetComponents(const std::unordered_map<WireGUID, std::vector<uint8_t>>& components, EntityId aEntity);

		template<typename T>
		const std::vector<EntityId> GetComponentView() const;

		template<typename ... T, typename F>
		void ForEach(F&& func);

		template<typename T>
		inline void SetOnCreateComponent(std::function<void(void*)> func);

		template<typename T>
		inline void SetOnRemoveComponent(std::function<void(void*)> func);

	private:
		friend class Serializer;

		std::unordered_map<WireGUID, ComponentPool> m_pools;
		std::unordered_map<EntityId, std::vector<EntityId>> m_childEntities;

		EntityId m_nextEntityId = 1; // ID zero is null
		std::vector<EntityId> m_availiableIds;
		std::vector<EntityId> m_usedIds;
	};

	template<typename T>
	inline T& Registry::AddComponent(EntityId aEntity)
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			return it->second.AddComponent<T>(aEntity);
		}
		else
		{
			m_pools.emplace(guid, ComponentPool(sizeof(T)));
			m_pools[guid].SetDestructor(std::make_shared<Destructor<T>>());
			return m_pools[guid].AddComponent<T>(aEntity);
		}
	}

	template<typename T>
	inline T& Registry::GetComponent(EntityId aEntity)
	{
		assert(HasComponent<T>(aEntity));
		const WireGUID guid = T::comp_guid;

		return m_pools.at(guid).GetComponent<T>(aEntity);
	}

	template<typename T>
	inline bool Registry::HasComponent(EntityId aEntity) const
	{
		const WireGUID guid = T::comp_guid;
		if (m_pools.find(guid) != m_pools.end())
		{
			return m_pools.at(guid).HasComponent(aEntity);
		}

		return false;
	}

	template<typename ...T>
	inline bool Registry::HasComponents(EntityId aEntity) const
	{
		return (HasComponent<T>(aEntity) && ...);
	}

	template<typename T>
	inline void Registry::RemoveComponent(EntityId aEntity)
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		assert(it != m_pools.end());

		it->second.RemoveComponent(aEntity);
	}

	template<typename T>
	inline const std::vector<T>& Registry::GetAllComponents() const
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		assert(it != m_pools.end());

		return reinterpret_cast<const std::vector<T>&>(it->second.GetAllComponents());
	}

	inline std::unordered_map<WireGUID, std::vector<uint8_t>> Registry::GetComponents(EntityId aEntity) const
	{
		std::unordered_map<WireGUID, std::vector<uint8_t>> data;

		for (const auto& [guid, pool] : m_pools)
		{
			if (pool.HasComponent(aEntity))
			{
				data.emplace(guid, pool.GetComponentData(aEntity));
			}
		}

		return data;
	}

	inline void Registry::SetComponents(const std::unordered_map<WireGUID, std::vector<uint8_t>>& components, EntityId aEntity)
	{
		for (auto& [guid, pool] : m_pools)
		{
			if (pool.HasComponent(aEntity) && components.find(guid) != components.end())
			{
				pool.SetComponentData(components.at(guid), aEntity);
			}
		}
	}

	template<typename T>
	inline const std::vector<EntityId> Registry::GetComponentView() const
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			return it->second.GetComponentView();
		}

		return std::vector<EntityId>();
	}

	template<typename ...T, typename F>
	inline void Registry::ForEach(F&& func)
	{
		for (const auto& id : m_usedIds)
		{
			if (HasComponents<T...>(id))
			{
				func(id, GetComponent<T>(id)...);
			}
		}
	}

	template<typename T>
	inline void Registry::SetOnCreateComponent(std::function<void(void*)> func)
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			it->second.SetOnCreateComponent(func);
		}
	}

	template<typename T>
	inline void Registry::SetOnRemoveComponent(std::function<void(void*)> func)
	{
		const WireGUID guid = T::comp_guid;

		auto it = m_pools.find(guid);
		if (it != m_pools.end())
		{
			it->second.SetOnRemoveComponent(func);
		}
	}
}