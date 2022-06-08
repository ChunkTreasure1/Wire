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

	Entity CreateEntity();
	void RemoveEntity(const Entity& entity);

private:
	friend class Entity;

	std::string m_name;
	Wire::Registry m_registry;
	std::vector<Entity> m_entities;
};