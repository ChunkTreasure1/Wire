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

		template<typename T>
		T& AddComponent(EntityId aId, T& aComponent);

		void RemoveComponent(EntityId aId);

		template<typename T>
		T& GetComponent(EntityId aId);

		bool HasComponent(EntityId aId);

		inline const std::vector<uint8_t>& GetAllComponents() { return myPool; }

	private:
		void Defragment();

		uint32_t myComponentSize = 0;
		std::vector<uint8_t> myPool;
		std::unordered_map<EntityId, size_t> myToEntityMap;
	};

	template<typename T>
	inline T& ComponentPool::AddComponent(EntityId aId, T& aComponent)
	{
		auto it = myToEntityMap.find(aId);
		assert(it == myToEntityMap.end());

		size_t index = myPool.size();
		myPool.resize(myPool.size() + sizeof(T));
		memcpy_s(&myPool[index], sizeof(T), &aComponent, sizeof(T));

		myToEntityMap[aId] = index;

		return *reinterpret_cast<T*>(&myPool[index]);
	}

	inline void ComponentPool::RemoveComponent(EntityId aId)
	{
		auto it = myToEntityMap.find(aId);
		assert(it != myToEntityMap.end());

		const size_t lastComponentIndex = myPool.size() - myComponentSize;

		memcpy_s(&myPool[it->second], myComponentSize, &myPool[lastComponentIndex], myComponentSize);
		myPool.resize(myPool.size() - myComponentSize);

		for (auto& newIt : myToEntityMap)
		{
			if (newIt.second == lastComponentIndex)
			{
				newIt.second = it->second;
				break;
			}
		}

		myToEntityMap.erase(it);
	}

	template<typename T>
	inline T& ComponentPool::GetComponent(EntityId aId)
	{
		assert(HasComponent<T>(aId));
		return *reinterpret_cast<T*>(&myPool[myToEntityMap[aId]]);
	}

	inline bool ComponentPool::HasComponent(EntityId aId)
	{
		auto it = myToEntityMap.find(aId);
		return it != myToEntityMap.end();
	}
}