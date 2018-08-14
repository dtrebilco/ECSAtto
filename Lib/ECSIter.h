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
    uint64_t m_bits;
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
            m_bits = m_manager->GetBits()[0];
            if ((m_bits & 0x1) == 0)
            {
              UpdateEntityID();
            }
            break;
          }
        }
        m_groupIndex++;
      }
    }

    void UpdateEntityID()
    {
      // DT_TODO: test + optimize for long runs of zeros (skip 64 at a time)?
      do
      {
        // Go to next bit
        m_bits >>= 1;
        m_entitySubID++;
        if ((m_entitySubID & 0x3F) == 0)
        {
          m_bits = m_manager->GetBits()[m_entitySubID >> 6];
        }
      } while ((m_bits & 0x1) == 0);
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context, m_member); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
  T E::*m_member;
};

//template <class E, class T>
//auto IterID(Context<E> &i_context, T E::*member) { return IterIDProcess<E, T>(i_context, member); }

template <class T, class E>
auto IterID(Context<E> &i_context) { return IterIDProcess<E, T>(i_context, GetManager<T, E>()); }



template <class E, class T, class CF>
class IterIDProcessF1
{
public:

  IterIDProcessF1(Context<E> &i_context, T E::*i_member, CF E::*i_flagMember) : m_context(i_context), m_member(i_member), m_flagMember(i_flagMember) {}

  struct Value : public T::ComponentType
  {
  protected:

    uint16_t m_groupIndex = 0;
    uint16_t m_entitySubID = 0;

  public:

    inline EntityID GetEntityID() const
    {
      return EntityID{ (GroupID)m_groupIndex, (EntitySubID)m_entitySubID };
    }
  };

  struct Iterator : public Value
  {
    uint16_t m_componentCount = 0;
    uint64_t m_bits;
    uint64_t m_flagBits;
    uint16_t m_entityMax;
    CF* m_flagManager;

    Context<E>& m_context;
    T E::*m_member;
    CF E::*m_flagMember;

    inline Iterator(Context<E> &i_context, T E::*i_member, CF E::*i_flagMember)
      : m_context(i_context), m_member(i_member), m_flagMember(i_flagMember)
    {
      UpdateGroupIndex();
    }

    inline Iterator& operator++()
    {
      // Go to next group if no more components
      if (m_index == (m_componentCount - 1))
      {
        m_groupIndex++;
        UpdateGroupIndex();
      }
      else
      {
        UpdateEntityID();

        // Go to next group if no more entities
        if (m_entitySubID >= m_entityMax)
        {
          m_groupIndex++;
          UpdateGroupIndex();
        }
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
          m_manager     = &(group->*m_member);
          m_flagManager = &(group->*m_flagMember);

          m_entityMax      = group->GetEntityMax();
          m_componentCount = m_manager->GetComponentCount();

          if (m_componentCount > 0)
          {
            m_bits = m_manager->GetBits()[0];
            m_flagBits = m_bits & m_flagManager->GetBits()[0];
            if ((m_flagBits & 0x1) == 0)
            {
              UpdateEntityID();
            }

            // Only break if an id is set
            if (m_entitySubID < m_entityMax)
            {
              break;
            }
          }
        }
        m_groupIndex++;
      }
    }

    void UpdateEntityID()
    {
      // DT_TODO: test + optimize for long runs of zeros (skip 64 at a time)?
      do
      {
        // Go to next bit
        m_bits >>= 1;
        m_flagBits >>= 1;
        m_entitySubID++;
        if (m_entitySubID >= m_entityMax)
        {
          break;
        }

        if ((m_entitySubID & 0x3F) == 0)
        {
          uint16_t i = m_entitySubID >> 6;
          m_bits = m_manager->GetBits()[i];
          m_flagBits = m_bits & m_flagManager->GetBits()[i];
        }
        m_index += (m_bits & 0x1);
      } while ((m_flagBits & 0x1) == 0);
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context, m_member, m_flagMember); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
  T E::*m_member;
  CF E::*m_flagMember;
};

//template <class E, class T>
//auto IterID(Context<E> &i_context, T E::*member, ComponentFlags E::*flagMember) { return IterIDProcessF1<E, T>(i_context, member, flagMember); }

template <class T, class E, class CF>
auto IterID(Context<E> &i_context, CF E::*flagMember) { return IterIDProcessF1<E, T, CF>(i_context, GetManager<T, E>(), flagMember); }


