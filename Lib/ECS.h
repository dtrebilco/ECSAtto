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


class ComponentFlags
{
public:

  inline bool HasComponent(EntitySubID i_entity) const
  {
    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F); // DT_TODO: Move this common code? (EntitySubID struct?)
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

  inline uint16_t GetComponentCount()
  {
    if (m_prevSum.size() == 0)
    {
      return 0;
    }

    return m_prevSum.back() + PopCount64(GetBits().back()); // DT_TODO: Consider an extra count entry at the end of m_prevSum?
  }

  //inline EntitySubID GetSubID(uint16_t i_index)
  //{
  //  AT_ASSERT(i_index < GetComponentCount());
  //
  //  // Binary search - get index
  //
  //  // Count the bits up to the index (optimize?)
  //  
  //}

  virtual void OnComponentRemove(uint16_t i_index) = 0;

private:
  friend class EntityGroup;

  std::vector<uint16_t> m_prevSum; //!< The sum of all previous bits

};

template <typename T, typename E>
T E::* GetManager();

class FlagManager : public ComponentFlags {};

template<typename T>
class ComponentTypeManager : public ComponentManager
{
public:

  inline void OnComponentAdd(uint16_t i_index)
  {
    m_data.insert(m_data.begin() + i_index, T());
  }

  inline void OnComponentAdd(uint16_t i_index, const T& i_addData)
  {
    m_data.insert(m_data.begin() + i_index, i_addData);
  }

  virtual void OnComponentRemove(uint16_t i_index)
  {
    m_data.erase(m_data.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_data.reserve(i_count);
  }

  T& GetData(uint16_t i_index)
  {
    return m_data[i_index];
  }

private:

  std::vector<T> m_data; //!< The data stored

};

/// \brief A entity group base class. This is intended to be inherited from and contain ComponentManagers
class EntityGroup
{
public:

  inline bool IsValid(EntitySubID i_entity) const
  {
    return (uint16_t)i_entity < m_entityMax;
  }

  inline uint16_t GetEntityMax() const
  {
    return m_entityMax;
  }

  EntitySubID AddEntity();
  void RemoveEntity(EntitySubID i_entity);
  void ReserveEntities(uint16_t i_count);

  static uint16_t SetComponentBit(EntitySubID i_entity, ComponentManager& i_manager);
  static uint16_t ClearComponentBit(EntitySubID i_entity, ComponentManager& i_manager);

  inline static void SetFlagBit(EntitySubID i_entity, FlagManager& i_manager)
  {
    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
    uint16_t index = (uint16_t)i_entity >> 6;

    i_manager.m_bitData[index] |= mask;
  }

  inline static void ClearFlagBit(EntitySubID i_entity, FlagManager& i_manager)
  {
    uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
    uint16_t index = (uint16_t)i_entity >> 6;

    i_manager.m_bitData[index] &= ~mask;
  }

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
  inline void RemoveEntityGroup(GroupID i_group);

  /// \brief Add an entity to the indicated group
  /// \return The added entity is returned
  inline EntityID AddEntity(GroupID i_group);

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
  inline void AddComponent(EntityID i_entity, T E::*i_member, const Args&... args)
  {
    AT_ASSERT(IsValid(i_entity));
    E* group = m_groups[(uint16_t)i_entity.m_groupID];

    uint16_t index = EntityGroup::SetComponentBit(i_entity.m_subID, group->*i_member);
    (group->*i_member).OnComponentAdd(index, args...);
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

  inline bool HasFlag(EntityID i_entity, FlagManager E::*i_member)
  {
    AT_ASSERT(IsValid(i_entity));
    return (m_groups[(uint16_t)i_entity.m_groupID]->*i_member).HasComponent(i_entity.m_subID);
  }

  inline void SetFlag(EntityID i_entity, FlagManager E::*i_member, bool i_value)
  {
    AT_ASSERT(IsValid(i_entity));
    E* group = m_groups[(uint16_t)i_entity.m_groupID];

    if (i_value)
    {
      EntityGroup::SetFlagBit(i_entity.m_subID, group->*i_member);
    }
    else
    {
      EntityGroup::ClearFlagBit(i_entity.m_subID, group->*i_member);
    }
  }

  inline void ReserveGroups(uint16_t i_count)
  {
    m_groups.reserve(i_count);
  }

  inline void ReserveEntities(GroupID i_group, uint16_t i_count)
  {
    AT_ASSERT(IsValid(i_group));
    m_groups[(uint16_t)i_group]->ReserveEntities(i_count);
  }

  template <class T>
  inline void ReserveComponent(GroupID i_group, T E::*i_member, uint16_t i_count)
  {
    AT_ASSERT(IsValid(i_group));
    (m_groups[(uint16_t)i_group]->*i_member).ReserveComponent(i_count);
  }

  inline const std::vector<E*>& GetGroups() const
  {
    return m_groups;
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
inline void Context<E>::RemoveEntityGroup(GroupID i_group)
{
  AT_ASSERT(IsValid(i_group));

  delete m_groups[(uint16_t)i_group];
  m_groups[(uint16_t)i_group] = nullptr;
}

template<class E>
inline EntityID Context<E>::AddEntity(GroupID i_group)
{
  AT_ASSERT(IsValid(i_group));
  return EntityID{ i_group , m_groups[(uint16_t)i_group]->AddEntity() };
}

template<class E>
inline void Context<E>::RemoveEntity(EntityID i_entity)
{
  AT_ASSERT(IsValid(i_entity.m_groupID));
  m_groups[(uint16_t)i_entity.m_groupID]->RemoveEntity(i_entity.m_subID);
}

