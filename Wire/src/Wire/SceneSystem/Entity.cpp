#include "Entity.h"

#include "Scene.h"

Entity::Entity()
    : m_id(Wire::NullID)
{}

Entity::Entity(Wire::EntityId id, Scene* scene)
    : m_id(id), m_scene(scene)
{
}

Entity::Entity(const Entity& entity)
{
	*this = entity;
}

Entity::~Entity()
{
}

Entity& Entity::operator=(const Entity& entity)
{
	m_id = entity.m_id;
	m_scene = entity.m_scene;
	return *this;
}
