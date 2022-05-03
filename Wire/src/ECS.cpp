// ECS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Wire/Registry.h"

struct TestComponent
{
    TestComponent() = default;
    TestComponent(float aX, float aY)
        : x(aX), y(aY)
    { }

	std::string name = "TestComponent";

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

		registry.AddComponent<TestComponent>(ent, TestComponent(0.f, 0.f));
    }

    auto& comps = registry.GetAllComponents<TestComponent>();

	for (const auto& ent : myEntities)
	{
        registry.RemoveEntity(ent);
	}

    auto& newComps = registry.GetAllComponents<TestComponent>();
	
    std::vector<decltype(registry)> myTest;

    system("pause");
}