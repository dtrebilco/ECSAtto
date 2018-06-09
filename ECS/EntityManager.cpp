#include "EntityManager.h"


EntityManager::EntityID EntityManager::CreateEntity()
{
  Entity newEntity = {};
  EntityManager::EntityID retID;

  // Check if there is a deleted entity to reuse
  if (m_deletedEntityOffset < m_entities.size())
  {
    retID = m_deletedEntityOffset;
    // Move to next in the list
    m_deletedEntityOffset = m_entities[retID].m_offset;
    m_entities[retID] = newEntity;
  }
  else
  {
    retID = (uint32_t)m_entities.size();
    m_entities.push_back(newEntity);
  }
  return retID;
}

void EntityManager::DeleteEntity(EntityID i_entity)
{
  AT_ASSERT(i_entity < m_entities.size());

  // Null out all Component IDs

  // Set lowest null ID counter

  // Set offset to the ID
  m_entities[i_entity].m_componentMask = 0;
  m_entities[i_entity].m_flags = 0;
  m_entities[i_entity].m_offset = m_deletedEntityOffset;
  m_deletedEntityOffset = i_entity;
}


