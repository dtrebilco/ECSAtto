#pragma once
#include "ECS.h"

template <class E, class T>
class Iter
{
public:

  Iter(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

  struct Value
  {
  protected:

    inline Value(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

    Context<E>& m_context;
    T E::*m_member;

    // DT_TODO: Add a "slow" getEntityID

  public:

    T* m_component = nullptr;
    uint16_t m_componentIndex = 0;
  };

  struct Iterator : public Value
  {
    uint16_t m_groupIndex = 0;
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

      return *this;
    }

    void UpdateGroupIndex()
    {
      m_componentIndex = 0;
      m_componentCount = 0;
      while (m_groupIndex < m_context.GetGroups().size())
      {
        E* group = m_context.GetGroups()[m_groupIndex];
        if (group != nullptr)
        {
          m_component = &(group->*m_member);
          m_componentCount = m_component->GetComponentCount();
          if (m_componentCount > 0)
          {
            break;
          }
        }
        m_groupIndex++;
      }
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
auto CreateIter(Context<E> &i_context, T E::*member) { return Iter<E, T>(i_context, member); }


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

    T* m_component = nullptr;
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
          m_component = &(group->*m_member);
          m_componentCount = m_component->GetComponentCount();
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
