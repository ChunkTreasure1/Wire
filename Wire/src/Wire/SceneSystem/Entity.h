#pragma once

#include "Wire/ECS/Entity.h"

#include "Wire/SceneSystem/Scene.h"

#include <vector>

class Entity
{
public:
	Entity();
	Entity(Wire::EntityId id, Scene* scene);
	Entity(const Entity& entity);

	~Entity();

	template<typename T>
	T& GetComponent();

	template<typename T>
	T& AddComponent();

	template<typename T>
	bool HasComponent();

	template<typename T>
	void RemoveComponent();

	inline const Wire::EntityId GetId() const { return m_id; }
	inline bool IsNull() const { return m_id == Wire::NullID; }

	Entity& operator=(const Entity& entity);

	inline bool operator==(const Entity& entity) const { return m_id == entity.m_id; }
	inline bool operator!() const { return IsNull(); }
	inline explicit operator bool() const { return !IsNull(); }
	
private:
	Scene* m_scene = nullptr;
	Wire::EntityId m_id;
};

template<typename T>
inline T& Entity::GetComponent()
{
	m_scene->m_registry.GetComponent<T>(m_id);
}

template<typename T>
inline T& Entity::AddComponent()
{
	return m_scene->m_registry.AddComponent<T>(m_id);
}

template<typename T>
inline bool Entity::HasComponent()
{
	return m_scene->m_registry.HasComponent<T>(m_id);
}

template<typename T>
inline void Entity::RemoveComponent()
{
	m_scene->m_registry.RemoveComponent<T>(m_id);
}
