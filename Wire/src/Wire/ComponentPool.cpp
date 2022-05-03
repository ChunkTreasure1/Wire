#include "ComponentPool.hpp"

namespace Wire
{
	ComponentPool::ComponentPool(uint32_t aSize)
		: myComponentSize(aSize)
	{
		myPool.reserve(aSize * 100);
	}
	
	void ComponentPool::Defragment()
	{
		// TODO: implement
		EntityId start = 0;
		EntityId end = 0;

		for (auto it = myToEntityMap.begin(); it != myToEntityMap.end(); it++)
		{
			if (std::distance(it, myToEntityMap.end()) < 2)
			{
				break;
			}

			auto next = std::next(it);
			if (it->second - next->second > myComponentSize)
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