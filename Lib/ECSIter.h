#pragma once
#include "ECS.h"

template <class E, class T>
class IterProcess
{
public:

  IterProcess(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

  struct Iterator : public T::ComponentType
  {
    uint16_t m_groupIndex = 0;
    uint16_t m_componentCount = 0;
    Context<E>& m_context;
    T E::*m_member;

    inline Iterator(Context<E> &i_context, T E::*i_member)
    : m_context(i_context), m_member(i_member)
    {
      UpdateGroupIndex();
    }

    inline Iterator& operator++()
    {
      m_index++;
      if (m_index == m_componentCount)
      {
        m_groupIndex++;
        UpdateGroupIndex();
      }

      return *this;
    }

    void UpdateGroupIndex()
    {
      m_index = 0;
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
            break;
          }
        }
        m_groupIndex++;
      }
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline typename T::ComponentType& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context, m_member); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
  T E::*m_member;
};

template <class E, class T>
auto Iter(Context<E> &i_context, T E::*member) { return IterProcess<E, T>(i_context, member); }

template <class T, class E>
auto Iter(Context<E> &i_context) { return IterProcess<E, T>(i_context, GetManager<T,E>()); }



template <class E, class T>
class IterIDProcess
{
public:

  IterIDProcess(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

  struct Value : public T::ComponentType
  {
  protected:

    uint16_t m_groupIndex = 0;
    uint16_t m_entitySubID = 0;

  public:

    inline EntityID GetEntityID() const
    {
      return EntityID { (GroupID)m_groupIndex, (EntitySubID)m_entitySubID };
    }
  };

  struct Iterator : public Value
  {
    uint16_t m_componentCount = 0;
    Context<E>& m_context;
    T E::*m_member;

    inline Iterator(Context<E> &i_context, T E::*i_member)
    : m_context(i_context), m_member(i_member)
    {
      UpdateGroupIndex();
    }

    inline Iterator& operator++()
    {
      m_index++;
      if (m_index == m_componentCount)
      {
        m_groupIndex++;
        UpdateGroupIndex();
      }
      else
      {
        m_entitySubID++;
        UpdateEntityID();
      }

      return *this;
    }

    void UpdateGroupIndex()
    {
      m_entitySubID = 0;
      m_index = 0;
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
auto IterID(Context<E> &i_context, T E::*member) { return IterIDProcess<E, T>(i_context, member); }

template <class T, class E>
auto IterID(Context<E> &i_context) { return IterIDProcess<E, T>(i_context, GetManager<T, E>()); }

