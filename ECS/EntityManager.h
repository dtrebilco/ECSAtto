#pragma once

#include <cstdint>
#include <vector>

#include "Common.h"

class EntityManager
{
public:

  typedef uint16_t EntityID;      //!< Support 65k entities
  typedef uint16_t ComponentID;   //!< Support 65k entries per component type
  typedef uint64_t ComponentMask; //!< Support 64 components per entity

  EntityID CreateEntity();
  void DeleteEntity(EntityID i_entity);

  inline bool HasComponent(EntityID i_entity, ComponentType i_component) const
  {
    return m_entities[i_entity].m_componentMask & GetMask(i_component);
  }

  void Update(); // Cleanup and maintenance of data

protected:
  friend class ComponentManager;

  // Component Manager Interface
  bool AddComponent(EntityID i_entity, ComponentType i_component, ComponentID i_id);
  bool RemoveComponent(EntityID i_entity, ComponentType i_component);

  bool SetComponentID(EntityID i_entity, ComponentType i_component, ComponentID i_id);
  ComponentID GetComponentID(EntityID i_entity, ComponentType i_component);

private:

  static inline ComponentMask GetMask(ComponentType i_component)
  {
    return (ComponentMask)1 << (uint32_t)i_component;
  }

  struct Entity
  {
    ComponentMask m_componentMask; //!< Mask indicating what components are used
    uint32_t m_offset; //!< Offset for the components
    uint32_t m_flags;  //!< Custom flags per entity
  };

  std::vector<Entity> m_entities; //!< Array of entities - may have unused entities
  std::vector<ComponentID> m_entityComponents; //!< Array of components per entity

  uint32_t m_deletedEntityOffset = UINT32_MAX; //!< The first deleted entity (if any)
};
