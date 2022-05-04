// ECS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Wire/Registry.h"
#include "Wire/Serialization.h"

struct TestComponent
{
	TestComponent() = default;
	TestComponent(float aX, float aY)
        : x(aX), y(aY)
    { }

    float x, y;
    SERIALIZE_COMPONENT(TestComponent, "{6A2347FD-8CB4-431D-8599-AF7340755113}"_guid);
};

int main()
{
    std::cout << "Hello World!\n";

    Wire::Registry registry;
    
    std::vector<Wire::EntityId> myEntities;

    std::vector<uint8_t> compData;
    compData.resize(8);

    TestComponent testComp(10.f, 4.f);
    memcpy(compData.data(), &testComp, sizeof(TestComponent));

    for (uint32_t i = 0; i < 100; i++)
    {
		auto ent = registry.CreateEntity();
        myEntities.emplace_back(ent);

		registry.AddComponent(compData, TestComponent::comp_guid, ent);
    }

    auto& comps = registry.GetAllComponents<TestComponent>();
    for (const auto& comp : comps)
    {
		std::cout << comp.x << " " << comp.y << std::endl;
    }
    
    system("pause");
}