#include "EntityManager.h"


EntityManager::EntityID EntityManager::CreateEntity()
{
  Entity newEntity = {};
  EntityManager::EntityID retID;

  // Check if there is a deleted entity to reuse
  if (m_deletedEntityOffset < m_entities.size())
  {
    retID = (EntityID)m_deletedEntityOffset;
    
    // Move to next in the list
    Entity& entity = m_entities[m_deletedEntityOffset];
    m_deletedEntityOffset = entity.m_offset;
    entity = newEntity;
  }
  else
  {
    retID = (EntityID)m_entities.size();
    m_entities.push_back(newEntity);
  }
  return retID;
}

void EntityManager::DeleteEntity(EntityID i_entity)
{
  // Null out all Component IDs
  Entity& entity = GetEntity(i_entity);
  uint32_t count = PopCount64(entity.m_componentMask);
  for (uint32_t i = 0; i < count; i++)
  {
    m_entityComponents[entity.m_offset + i] = INVALID_CID;
  }

  // Set lowest null ID counter
  // DT_TODO:

  // Set offset to the ID
  entity.m_componentMask = 0;
  entity.m_flags = 0;
  entity.m_offset = m_deletedEntityOffset;
  m_deletedEntityOffset = (uint32_t)i_entity;
}

void EntityManager::Update()
{
  // If lowest NULL id is less than the limit, update
}

void EntityManager::AddComponent(EntityID i_entity, ComponentType i_component, ComponentID i_id)
{
  AT_ASSERT(!HasComponent(i_entity, i_component));

  // If the entity is at the end of the array, insert in place


  // Else, copy to the end of the array first

  // Set the new ID value

}

void EntityManager::RemoveComponent(EntityID i_entity, ComponentType i_component)
{
  AT_ASSERT(HasComponent(i_entity, i_component));

  // Unset flag

  // Move components down

  // Set slot to invalid
    // Update invalid slot index
}

void EntityManager::SetComponentID(EntityID i_entity, ComponentType i_component, ComponentID i_id)
{
  AT_ASSERT(HasComponent(i_entity, i_component));

  uint32_t offset = GetComponentOffset(i_entity, i_component);
  m_entityComponents[offset] = i_id;
}

EntityManager::ComponentID EntityManager::GetComponentID(EntityID i_entity, ComponentType i_component) const
{
  uint32_t offset = GetComponentOffset(i_entity, i_component);
  return m_entityComponents[offset];
}

