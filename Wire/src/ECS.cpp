// ECS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>

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

struct OtherComponent
{
	OtherComponent() = default;
	OtherComponent(float aX, float aY)
		: x(aX), y(aY)
	{
	}

	float x, y;
	SERIALIZE_COMPONENT(OtherComponent, "{4709522E-FB7B-4B85-8FDD-C31853A89FF3}"_guid);
};

REGISTER_COMPONENT(OtherComponent);
REGISTER_COMPONENT(TestComponent);

int main()
{
    std::cout << "Hello World!\n";

    Wire::Registry registry;
	auto ent = registry.CreateEntity();

    system("pause");
} 