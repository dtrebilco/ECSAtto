#include <stdio.h>
#include <cstdint>
#include <vector>

struct Entity
{
  uint64_t m_mask;
  uint32_t m_offset;
  uint32_t m_flags; 
};

enum class ComponentType
{


};



class System 
{
public:
  virtual ~System() {}

};


class Manager
{
public:

  typedef uint16_t EntityID;
  typedef uint16_t ComponentID;
  typedef uint64_t ComponentMask;

  EntityID CreateEntity(ComponentMask i_components); // Support recursive adds?
  EntityID CreateEntity(ComponentMask i_components, ComponentID* i_componentIDs); // pass an array of 64 ids?

  void DeleteEntity(EntityID i_entity); // Support recursive deletes?, do linked list of deleted items?

  // If component already exists, it is not added
  // Existing pointers to component types added are invalidated. - perhaps not part of this system?
  bool AddComponents(EntityID i_entity, ComponentMask i_components);
  bool RemoveComponents(EntityID i_entity, ComponentMask i_components);

  bool SetComponentID(EntityID i_entity, ComponentMask i_components, ComponentID componentID);
  ComponentID GetComponentID(EntityID i_entity, ComponentMask i_components);

  void Update(); // Cleanup and maintaince of data

private:

  std::vector<Entity> m_entities;
  std::vector<uint32_t> m_entityOffsets;

};

// Template
class ComponentManager
{

  // Define flag based on enum


  // Serializate/deserialize

  // Array of entity ids

};


class GameManager : private Manager
{
public:

  bool HasAllComponents();
  ComponentMask GetComponents(EntityID i_entity);

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

void CleanUpEntities()
{

}

void CleanUpComponents()
{

}


int main()
{
  printf("Hello"); 

  Manager cm;

  // Register componet systems

  // Add init systems

  // Add Process systems



  while (0)
  {


  }
  
  return 0;
}