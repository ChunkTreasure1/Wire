#include "Scene.h"

#include "Entity.h"

Scene::Scene(const std::string& name)
	: m_name(name)
{}

Scene::Scene(const Scene& scene)
{
	m_name = scene.m_name;
	m_entities = scene.m_entities;
	
}

Entity Scene::CreateEntity()
{
	Wire::EntityId id = m_registry.CreateEntity();
	return Entity(id, this);
}

void Scene::RemoveEntity(const Entity& entity)
{
	if (auto it = std::find(m_entities.begin(), m_entities.end(), entity); it != m_entities.end())
	{
		m_registry.RemoveEntity(it->GetId());
		m_entities.erase(it);
	}
}
