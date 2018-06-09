#pragma once
#include "Common.h"

#include <cstdint>
#include <vector>

/// \brief This class manages entities in the system. 
/// It is expected that it will be overridden with a game specific version that does custom behavior on adding and removing entities.
/// This class is optimized with the expected usage of adding components to newly created entities. 
/// Not creating lots of entities then adding components to them later.
///   ie.  Do: CreateEntity(), AddComponent(), AddComponent() Then, CreateEntity(), AddComponent(), AddComponent()
///        Rather than: CreateEntity(), CreateEntity(),  Then  AddComponent(), AddComponent(), AddComponent(), AddComponent()
class EntityManager
{
public:

  enum class EntityID : uint16_t {};    //!< Support 65k entities
  enum class ComponentID : uint16_t {}; //!< Support 65k entries per component type
  typedef uint64_t ComponentMask;       //!< Support 64 components per entity (could use std::bitset?)

  const EntityID INVALID_EID = (EntityID)UINT16_MAX;
  const ComponentID INVALID_CID = (ComponentID)UINT16_MAX;

  /// \brief Create an entity.
  EntityID CreateEntity();

  /// \brief Delete an entity.
  /// \param i_entity The entity to delete.
  void DeleteEntity(EntityID i_entity);

  /// \brief Test if a entity has a component on it
  /// \param i_entity The entity to test
  /// \param i_component The component to test for
  /// \return Returns true if the component exists on the entity
  inline bool HasComponent(EntityID i_entity, ComponentType i_component) const
  {
    return m_entities[(uint16_t)i_entity].m_componentMask & GetMask(i_component);
  }

  /// \brief Update the manager - cleans up and organizes data
  void Update();

protected:
  friend class ComponentManager;

  // Component Manager Interface
  void AddComponent(EntityID i_entity, ComponentType i_component, ComponentID i_id);
  void RemoveComponent(EntityID i_entity, ComponentType i_component);

  void SetComponentID(EntityID i_entity, ComponentType i_component, ComponentID i_id);
  ComponentID GetComponentID(EntityID i_entity, ComponentType i_component) const;

private:

  static inline ComponentMask GetMask(ComponentType i_component)
  {
    return (ComponentMask)1 << (uint32_t)i_component;
  }

  /// \brief Get the offset into the components data array
  inline uint32_t GetComponentOffset(EntityID i_entity, ComponentType i_component) const
  {
    const Entity& entity = m_entities[(uint16_t)i_entity];
    return entity.m_offset + PopCount64(entity.m_componentMask & (GetMask(i_component) - 1));
  }

  struct Entity
  {
    ComponentMask m_componentMask; //!< Mask indicating what components are used
    uint32_t m_offset;             //!< Offset for the components
    uint32_t m_flags;              //!< Custom flags per entity
  };
  static_assert(sizeof(Entity) == 16, "Unexpected entity size");

  std::vector<Entity> m_entities;              //!< Array of entities - may have unused entities
  std::vector<ComponentID> m_entityComponents; //!< Array of components per entity

  uint32_t m_deletedEntityOffset = UINT32_MAX; //!< The first deleted entity (if any)
};

