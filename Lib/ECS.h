#pragma once
#include "Common.h"

#include <cstdint>
#include <vector>


enum class GroupID : uint16_t {};  //!< Supports 65k groups
enum class EntitySubID : uint16_t {};  //!< Supports 65k entities per group

struct EntityID
{
  GroupID m_groupID;   //!< Index of the group
  EntitySubID m_subID; //!< Index of the entity in the group
};
static_assert(sizeof(EntityID) == 4, "Unexpected size");


template<class E>
class Context
{
public:

  ~Context();

  /// \brief Returns if the group is valid
  /// \return Returns true for a valid group
  inline bool IsValid(GroupID i_group) const;

  /// \brief Returns if the group is valid
  /// \return Returns true for a valid group
  inline bool IsValid(EntityID i_entity) const;

  /// \brief Add a new entity group
  /// \return The new group is returned
  inline GroupID AddEntityGroup();

  /// \brief Remove an entity group. 
  ///        NOTE: Ensure the group is not being accessed (ie. iterated upon) when doing this.
  /// \param i_groupID The group ID to remove
  inline void RemoveEntityGroup(GroupID i_groupID);

  /// \brief Add an entity to the indicated group
  /// \return The added entity is returned
  inline EntityID AddEntity(GroupID i_groupID);

  /// \brief Remove the entity from the context
  ///        NOTE: Ensure the entity is not being accessed (ie. iterated upon) when doing this.
  /// \param i_entity The entity to remove.
  inline void RemoveEntity(EntityID i_entity);

  template <class T>
  inline bool HasComponent(EntityID i_entity, T E::*i_member)
  {
    AT_ASSERT(IsValid(i_entity));
    return (m_groups[(uint16_t)i_entity.m_groupID]->*i_member).HasComponent(i_entity.m_subID);
  }

  template <class T, typename... Args>
  inline void AddComponent(EntityID i_entity, T E::*i_member, Args&... args)
  {
    AT_ASSERT(IsValid(i_entity));
    E* group = m_groups[(uint16_t)i_entity.m_groupID];

    uint16_t index = EntityGroup::SetComponentBit(i_entity.m_subID, group->*i_member);
    (group->*i_member).OnComponentAdd(index, args...);
  }

  template <class T>
  inline void RemoveComponent(EntityID i_entity, T E::*i_member)
  {
    AT_ASSERT(IsValid(i_entity));
    E* group = m_groups[(uint16_t)i_entity.m_groupID];

    uint16_t index = EntityGroup::ClearComponentBit(i_entity.m_subID, group->*i_member);
    (group->*i_member).OnComponentRemove(index);
  }

protected:

  std::vector<E*> m_groups;   //!< Array of entity groups

};

template<class E>
Context<E>::~Context()
{
  for (E* e : m_groups)
  {
    delete e;
  }
  m_groups.clear();
}

template<class E>
inline bool Context<E>::IsValid(GroupID i_group) const
{
  return ((uint16_t)i_group < m_groups.size()) &&
         (m_groups[(uint16_t)i_group] != nullptr);
}

template<class E>
inline bool Context<E>::IsValid(EntityID i_entity) const
{
  return IsValid(i_entity.m_groupID) &&
         m_groups[(uint16_t)i_entity.m_groupID]->IsValid(i_entity.m_subID);
}

template<class E>
inline GroupID Context<E>::AddEntityGroup()
{
  // Loop and find a vacant index
  for (uint32_t i = 0; i < m_groups.size(); i++)
  {
    // Allocate to that index
    if (m_groups[i] == nullptr)
    {
      m_groups[i] = new E();
      return GroupID(i);
    }
  }

  // Add a new item 
  m_groups.push_back(new E());
  return GroupID(m_groups.size() - 1);
}

template<class E>
inline void Context<E>::RemoveEntityGroup(GroupID i_groupID)
{
  AT_ASSERT(IsValid(i_groupID));

  delete m_groups[(uint16_t)i_groupID];
  m_groups[(uint16_t)i_groupID] = nullptr;
}

template<class E>
inline EntityID Context<E>::AddEntity(GroupID i_groupID)
{
  AT_ASSERT(IsValid(i_groupID));
  return EntityID{ i_groupID , m_groups[(uint16_t)i_groupID]->AddEntity() };
}

template<class E>
inline void Context<E>::RemoveEntity(EntityID i_entity)
{
  AT_ASSERT(IsValid(i_entity.m_groupID));
  m_groups[(uint16_t)i_entity.m_groupID]->RemoveEntity(i_entity.m_subID);
}

class ComponentManager;
class FlagManager;

/// \brief A entity group base class. This is intended to be inherited from and contain ComponentManagers
class EntityGroup
{
public:

  bool IsValid(EntitySubID i_entity) const
  {
    return (uint16_t)i_entity < m_entityMax;
  }

  EntitySubID AddEntity();
  void RemoveEntity(EntitySubID i_entity);

  static uint16_t SetComponentBit(EntitySubID i_entity, ComponentManager& i_manager);
  static uint16_t ClearComponentBit(EntitySubID i_entity, ComponentManager& i_manager);

  inline void AddManager(ComponentManager* i_manager)
  {
    AT_ASSERT(m_entityMax == 0);
    m_managers.push_back(i_manager);
  }

  inline void AddManager(FlagManager* i_manager)
  {
    AT_ASSERT(m_entityMax == 0);
    m_flagManagers.push_back(i_manager);
  }

private:

  uint16_t m_entityMax = 0;                   //!< Max entity allocated 

  std::vector<ComponentManager*> m_managers;  //!< Registry array of component managers
  std::vector<FlagManager*> m_flagManagers;   //!< Registry array of single flag managers

  std::vector<EntitySubID> m_deletedEntities; //!< Array of re-usable entity ids that have been deleted
};

class ComponentFlags
{
public:

  inline bool HasComponent(EntitySubID i_entity) const
  {
    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
    uint16_t index = (uint16_t)i_entity >> 6;

    return (m_bitData[index] & mask) != 0;
  }

  inline const std::vector<uint64_t>& GetBits() const
  {
    return m_bitData;
  }

private:

  friend class EntityGroup;
  std::vector<uint64_t> m_bitData; //!< The array of bit data

};


class ComponentManager : public ComponentFlags
{
public:

  virtual ~ComponentManager() {}

  inline uint16_t GetComponentIndex(EntitySubID i_entity)
  {
    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
    uint16_t index = (uint16_t)i_entity >> 6;

    return m_prevSum[index] + PopCount64(GetBits()[index] & (mask - 1));
  }

  virtual void OnComponentRemove(uint16_t i_index) = 0;

private:
  friend class EntityGroup;

  std::vector<uint16_t> m_prevSum; //!< The sum of all previous bits

};

class FlagManager : public ComponentFlags {};

template<typename T>
class ComponentTypeManager : public ComponentManager
{
public:

  void OnComponentAdd(uint16_t i_index)
  {
    m_data.insert(m_data.begin() + i_index, T());
  }

  void OnComponentAdd(uint16_t i_index, const T& i_addData)
  {
    m_data.insert(m_data.begin() + i_index, i_addData);
  }

  virtual void OnComponentRemove(uint16_t i_index)
  {
    m_data.erase(m_data.begin() + i_index);
  }

private:

  std::vector<T> m_data; //!< The data stored

};



