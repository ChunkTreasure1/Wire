#pragma once

#include "Wire/ECS/Registry.h"

#include <string>

class Entity;
class Scene
{
public:
	Scene() = default;
	Scene(const std::string& name);
	Scene(const Scene& scene);

	void OnRuntimeStart();
	void OnRuntimeEnd();

	Entity CreateEntity();
	void RemoveEntity(const Entity& entity);

	inline Wire::Registry& GetRegistry() { return m_registry; }

private:
	friend class Entity;

	std::string m_name;
	Wire::Registry m_registry;
};