#pragma once
#include "Common.h"

#include <cstdint>
#include <vector>

template<class E>
class Context
{
public:

  enum class GroupID : uint16_t {};  //!< Supports 65k groups
  enum class EntitySubID : uint16_t {};  //!< Supports 65k entities per group

  struct EntityID 
  {
    GroupID m_groupID;   //!< Index of the group
    EntitySubID m_subID; //!< Index of the entity in the group
  };
  static_assert(sizeof(EntityID) == 4, "Unexpected size");

  /// \brief Returns if the group is valid
  /// \return Returns true for a valid group
  inline bool IsGroupValid(GroupID i_group) const;

  /// \brief Returns if the group is valid
  /// \return Returns true for a valid group
  inline bool IsEntityValid(EntityID i_entity) const;

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

protected:

  std::vector<E*> m_groups;   //!< Array of entity groups

};

template<class E>
inline bool Context<E>::IsGroupValid(GroupID i_group) const
{
  return (i_group < m_groups.size()) &&
         (m_groups[i_group] != nullptr);
}

template<class E>
inline bool Context<E>::IsEntityValid(EntityID i_entity) const
{
  return IsGroupValid(i_entity.m_groupID) &&
         m_groups[i_group]->IsValid(i_entity.m_subID);
}

template<class E>
inline typename Context<E>::GroupID Context<E>::AddEntityGroup()
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
  AT_ASSERT(IsValidGroup(i_groupID));

  delete m_groups[i_groupID];
  m_groups[i_groupID] = nullptr;
}

template<class E>
inline typename Context<E>::EntityID Context<E>::AddEntity(GroupID i_groupID)
{
  AT_ASSERT(IsValidGroup(i_groupID));
  return EntityID{ i_groupID , m_groups[i_groupID]->AddEntity() };
}

template<class E>
inline void Context<E>::RemoveEntity(EntityID i_entity)
{
  AT_ASSERT(IsValidGroup(i_entity.m_groupID));
  m_groups[i_groupID]->RemoveEntity(i_entity.m_subID);
}


