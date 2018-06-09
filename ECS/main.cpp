#include <stdio.h>
#include <cstdint>
#include <vector>

#define AT_ASSERT(x)

enum class ComponentType
{


};


class System 
{
public:
  virtual ~System() {}

};


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
    retID = m_entities.size();
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






// Template
class ComponentManager
{

  // Define flag based on enum


  // Serialize/deserialize

  // Array of entity ids

};

/*
class GameManager : private Manager
{
public:

  //bool HasAllComponents(EntityID i_entity, ComponentMask i_components);
  //ComponentMask GetComponents(EntityID i_entity);

  // Template?
  //Data* GetComponent<>(EntityID i_entity);

  //CreateEntity() { Call base, init the components, set the IDs}

  // Update() { call base, defrag a component manager (cycle)

  //bool AddComponents(EntityID i_entity, ComponentMask i_components);
  //bool RemoveComponents(EntityID i_entity, ComponentMask i_components); { Call destructor


  // DefragAll();
protected:
  // List of component managers

  // 

};
*/
void CleanUpEntities()
{

}

void CleanUpComponents()
{

}


int main()
{
  printf("Hello"); 

  //Manager cm;

  // Register component systems

  // Add init systems

  // Add Process systems



  while (0)
  {


  }
  
  return 0;
}