#include <stdio.h>
#include <cstdint>
#include <vector>

#include "Common.h"
#include "EntityManager.h"


// ECS General
// - Add entity
// - Remove entity
// - Add component
// - Remove component
// - Get component
// - Test if component exists


// ECS Bit-mask
// - Add entity  - OK
// - Remove entity - Bit mask of pending deletes, Need to test all component managers on delete - not great
// - Add component - Pending add
// - Remove component - Pending remove
// - Get component - OK
// - Test if component exists - OK
//    Need to delete a bit from an array and shuffle down
//    Need cache of counts every x bits - need to recalculate those also
// - Have arrays of bit flags only components







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