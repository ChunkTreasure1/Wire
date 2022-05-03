#pragma once

#include "Entity.h"
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
		std::unordered_map<size_t, ComponentPool> myPools;
		EntityId myNextEntityId = 1; // ID zero is null
	};
	
	template<typename T, typename ...Args>
	inline T& Registry::AddComponent(EntityId aEntity, Args && ...args)
	{
		size_t typeId = typeid(T).hash_code();
		auto it = myPools.find(typeId);
		if (it != myPools.end())
		{
			T comp(std::forward<Args>(args)...);
			return it->second.AddComponent<T>(aEntity, comp);
		}
		else
		{
			myPools.emplace(typeId, ComponentPool(sizeof(T)));
			
			T comp(std::forward<Args>(args)...);
			return myPools[typeId].AddComponent(aEntity, comp);
		}
	}

	template<typename T>
	inline T& Registry::GetComponent(EntityId aEntity)
	{
		assert(HasComponent<T>());
		size_t typeId = typeid(T).hash_code();
		return myPools[typeId].GetComponent<T>(aEntity);
	}

	template<typename T>
	inline bool Registry::HasComponent(EntityId aEntity)
	{
		size_t typeId = typeid(T).hash_code();
		return myPools[typeId].HasComponent(aEntity);
	}

	template<typename T>
	inline void Registry::RemoveComponent(EntityId aEntity)
	{
		size_t typeId = typeid(T).hash_code();
		
		auto it = myPools.find(typeId);
		assert(it != myPools.end());

		it->second.RemoveComponent(aEntity);
	}
	
	template<typename T>
	inline const std::vector<T>& Registry::GetAllComponents()
	{
		size_t typeId = typeid(T).hash_code();
		
		auto it = myPools.find(typeId);
		assert(it != myPools.end());

		return reinterpret_cast<const std::vector<T>&>(it->second.GetAllComponents());
	}
}