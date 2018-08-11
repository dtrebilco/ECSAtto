#pragma once
#include "ECS.h"

template <class E, class T>
class Iter
{
public:

  Iter(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

  struct Iterator
  {
    uint16_t m_groupIndex = 0;
    uint16_t m_componentCount = 0;
    Context<E>& m_context;
    T E::*m_member;
    typename T::ComponentType m_value;

    inline Iterator(Context<E> &i_context, T E::*i_member)
    : m_context(i_context), m_member(i_member)
    {
      UpdateGroupIndex();
    }

    inline Iterator& operator++()
    {
      m_value.m_index++;
      if (m_value.m_index == m_componentCount)
      {
        m_groupIndex++;
        UpdateGroupIndex();
      }

      return *this;
    }

    void UpdateGroupIndex()
    {
      m_value.m_index = 0;
      m_componentCount = 0;
      while (m_groupIndex < m_context.GetGroups().size())
      {
        E* group = m_context.GetGroups()[m_groupIndex];
        if (group != nullptr)
        {
          m_value.m_manager = &(group->*m_member);
          m_componentCount = m_value.m_manager->GetComponentCount();
          if (m_componentCount > 0)
          {
            break;
          }
        }
        m_groupIndex++;
      }
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline typename T::ComponentType& operator *() { return m_value; }
  };

  inline Iterator begin() { return Iterator(m_context, m_member); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
  T E::*m_member;
};

template <class E, class T>
auto CreateIter(Context<E> &i_context, T E::*member) { return Iter<E, T>(i_context, member); }

template <class T, class E>
auto CreateIterT(Context<E> &i_context) { return Iter<E, T>(i_context, GetManager<T,E>()); }



template <class E, class T>
class IterID
{
public:

  IterID(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

  struct Value
  {
  protected:

    inline Value(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

    Context<E>& m_context;
    T E::*m_member;
    uint16_t m_groupIndex = 0;
    EntitySubID m_entitySubID = (EntitySubID)0;

  public:

    T* m_manager = nullptr;
    uint16_t m_componentIndex = 0;

    inline EntityID GetEntityID() const
    {
      return EntityID { (GroupID)m_groupIndex, m_entitySubID };
    }
  };

  struct Iterator : public Value
  {
    uint16_t m_componentCount = 0;

    inline Iterator(Context<E> &i_context, T E::*i_member)
      : Value(i_context, i_member)
    {
      UpdateGroupIndex();
    }

    inline Iterator& operator++()
    {
      m_componentIndex++;
      if (m_componentIndex == m_componentCount)
      {
        m_groupIndex++;
        UpdateGroupIndex();
      }
      else
      {
        m_entitySubID = (EntitySubID)((uint16_t)m_entitySubID + 1);
        UpdateEntityID();
      }

      return *this;
    }

    void UpdateGroupIndex()
    {
      m_entitySubID = (EntitySubID)0;
      m_componentIndex = 0;
      m_componentCount = 0;
      while (m_groupIndex < m_context.GetGroups().size())
      {
        E* group = m_context.GetGroups()[m_groupIndex];
        if (group != nullptr)
        {
          m_manager = &(group->*m_member);
          m_componentCount = m_manager->GetComponentCount();
          if (m_componentCount > 0)
          {
            UpdateEntityID();
            break;
          }
        }
        m_groupIndex++;
      }
    }

    void UpdateEntityID()
    {
      //DT_TODO:
      // Test if the current entity id bit is set
        // Go to next bit
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context, m_member); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
  T E::*m_member;
};

template <class E, class T>
auto CreateIDIter(Context<E> &i_context, T E::*member) { return IterID<E, T>(i_context, member); }
