// ECS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Wire/Registry.h"

struct TestComponent
{
    float x, y;
};

int main()
{
    std::cout << "Hello World!\n";

    Wire::Registry registry;
    
    std::vector<Wire::EntityId> myEntities;

    for (uint32_t i = 0; i < 100; i++)
    {
		auto ent = registry.CreateEntity();
        myEntities.emplace_back(ent);

		registry.AddComponent<TestComponent>(ent, TestComponent{ 1.0f + i, 2.0f });
    }

    auto& comps = registry.GetAllComponents<TestComponent>();

	for (auto& comp : comps)
	{
		std::cout << comp.x << " " << comp.y << std::endl;
	}

	for (const auto& ent : myEntities)
	{
        registry.RemoveEntity(ent);
	}

    auto& newComps = registry.GetAllComponents<TestComponent>();
	
    std::vector<decltype(registry)> myTest;

    system("pause");
}