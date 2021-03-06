#include <stdio.h>
#include <cstdint>
#include <vector>

#include "Common.h"
#include "EntityManager.h"
#include <tuple>

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

//For each component type serialize via
//  template<class T> bool SerializeBinary(const T& i_data, Serializer i_s);
//  template<class T> bool SerializeJson(const T& i_data, SerializerJson i_json);

class EntityComponentManager
{
public:

  // HasComponent
  // GetComponentIndex

private:

  // OnAddEntity
  // OnRemoveEntity
  // OnAddComponent
  // OnRemoveComponent

  std::vector<uint64_t> m_data; //!< The array of bit data
  std::vector<uint16_t> m_prevSum; //!< The sum of all previous bits

};

class EntityFlagManager
{
public:

  // IsFlagSet
  // SetFlag

private:

  std::vector<uint64_t> m_data; //!< The array of bit data
};

// Template
class ComponentManager
{
  //ComponentManager(EntityGroup & i_register)
  //virtual ~ComponentManager();

  // HasComponent
  // GetComponentIndex

  // virtual void OnComponentRemove(index, context) = 0;
  // virtual void OnGroupRemove(context);
  // virtual void ProcessAdditions(context);

  // EntityComponentManager m_entityManager;
};

class TransformComponentManager : ComponentManager
{
public:
  int i = 0; // dummy
};

class EntityGroup
{
public:
  enum class EntityGroupID : uint16_t {};

  //AddEntity()
  //RemoveEntity()

  TransformComponentManager m_transformComponents;

private:

  //!< Index re-direct (only if delete has been called)
  //!< Count of entities
  //!< Registry array of entity components (ComponentManager array)
  //!< Registry of bit flags

};

class EngineEntityGroup : public EntityGroup
{
public:
  // Engine components used
};

class GameEntityGroup : public EngineEntityGroup
{
public:
  // Game components used
};

template<class ENTITYGROUP>
class Context
{
public:
  enum class EntityID : uint32_t {};
  enum class GroupID : uint16_t {};

  // AddEntityGroup()
  // RemoveEntityGroup() // Delayed

  // AddEntity(groupId)
  // RemoveEntity() // Delayed

  //template<T>
  // uint32_t GetComponentIndex(EntityID);

  // Update() <- Process delete requests


  // Array of entity groups
  std::vector<ENTITYGROUP> m_groups;

  // Systems
};


template <class A, class E>
class Iter
{
public:
  Iter(Context<E> &i_context, A E::*i_member) : m_context(i_context), m_member(i_member){}

  void Process()
  {
    for (E& group : m_context.m_groups)
    {
      A& value = group.*m_member;
      value.i = 7;
    }
  }

  Context<E>& m_context;
  A E::*m_member;
};

template <class T, class E>
auto CreateIter(Context<E> &i_context, T E::*member) { return Iter<T, E>(i_context, member); }


class ComponentIterator
{
  // Next(Context);

  //Context::EntityID GetEntity() const; // Construct from array ID

  //ComponentManagerType* m_component;
  //uint16_t m_componentIndex;

private:
  //Context::GroupID m_groupID;
  uint32_t m_componentIndex;
};

class ComponentIterator2
{
  // Next(Context);

  //Context::EntityID GetEntity() const; // Construct from array ID

  //ComponentManagerType* m_component;
  //uint16_t m_componentIndex;
  //ComponentManagerType* m_component;
  //uint16_t m_componentIndex;

private:
  //Context::GroupID m_groupID;
  //uint32_t m_componentIndex;
};


class ComponentIterator3
{
  // Next(Context);

  //Context::EntityID GetEntity() const; // Construct from array ID

  //ComponentManagerType* m_component;
  //uint16_t m_componentIndex;
  //ComponentManagerType* m_component;
  //uint16_t m_componentIndex;
  //ComponentManagerType* m_component;
  //uint16_t m_componentIndex;

private:
  //Context::GroupID m_groupID;
  //uint32_t m_componentIndex;
};



// LoadContext

// 


// Systems <- register themselves with appropriate callbacks/update etc (on delete etc)
// SystemData?

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
  std::tuple<int, float> test(10, 6.0f);
  std::get<0>(test);

  //test.

  printf("Hello"); 

  Context<EntityGroup> context;
  auto iter = Iter<TransformComponentManager, EntityGroup>(context, &EntityGroup::m_transformComponents);

  auto iter2 = CreateIter(context, &EntityGroup::m_transformComponents);

  iter.Process();

  // Do interface like:
  //context.IsValidEntity(entityID);

  //context.AddComponent(entityID, &EntityGroup::m_transformComponents);
  //context.RemoveComponent(entityID, &EntityGroup::m_transformComponents); 
  //context.HasComponent(entityID, &EntityGroup::m_transformComponents); 

  // context.GetComponentManager(entityID, &EntityGroup::m_transformComponents); 
  // context.GetComponentOffset(entityID, &EntityGroup::m_transformComponents); 
  // T& context.GetComponent(entityID, &EntityGroup::m_transformComponents); 
  // const T& context.GetComponent(entityID, &EntityGroup::m_transformComponents); 

  //Manager cm;

  // Register component systems

  // Add init systems

  // Add Process systems



  while (0)
  {


  }
  
  return 0;
}