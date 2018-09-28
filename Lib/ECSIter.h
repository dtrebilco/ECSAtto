#pragma once
#include "ECS.h"

template <class T, class E>
class IterProcess
{
public:

  IterProcess(Context<E> &i_context) : m_context(i_context) {}

  struct Value : public T::ComponentType
  {
  protected:

    uint16_t m_groupIndex = 0;

  public:

    inline EntityID GetGroupID() const { return (GroupID)m_groupIndex; }

  };

  struct Iterator : public Value
  {
    uint16_t m_componentCount = 0;
    Context<E>& m_context;

    inline Iterator(Context<E> &i_context)
    : m_context(i_context)
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
          m_manager = &::GetManager<T>(*group);
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
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
};

template <class T, class E>
auto Iter(Context<E> &i_context) { return IterProcess<T, E>(i_context); }


template <class T, class E>
class IterIDProcess
{
public:

  IterIDProcess(Context<E> &i_context) : m_context(i_context) {}

  struct Value : public T::ComponentType
  {
  protected:

    uint16_t m_groupIndex = 0;

  public:

    inline EntityID GetEntityID() const { return EntityID{ (GroupID)m_groupIndex, GetSubID() }; }
    inline EntityID GetGroupID() const { return (GroupID)m_groupIndex; }

  };

  struct Iterator : public Value
  {
    uint16_t m_componentCount = 0;
    Context<E>& m_context;

    inline Iterator(Context<E> &i_context)
      : m_context(i_context)
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
          m_manager = &::GetManager<T>(*group);
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
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
};

template <class T, class E>
auto IterID(Context<E> &i_context) { return IterIDProcess<T, E>(i_context); }


template <class T, class E>
class IterEntityProcess
{
public:

  IterEntityProcess(Context<E> &i_context) : m_context(i_context) {}

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

    inline Iterator(Context<E> &i_context)
    : m_context(i_context)
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
          m_manager = &::GetManager<T>(*group);
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

  inline Iterator begin() { return Iterator(m_context); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
};


template <class T, class E>
auto IterEntity(Context<E> &i_context) { return IterEntityProcess<T, E>(i_context); }


template <class T, class CF, class E>
class IterEntityProcessF1
{
public:

  IterEntityProcessF1(Context<E> &i_context) : m_context(i_context) {}

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
    uint64_t m_flagBits; // DT_TODO - Recalculate bit each iteration in case they change?
    uint16_t m_entityCount;
    CF* m_flagManager;

    Context<E>& m_context;

    inline Iterator(Context<E> &i_context)
      : m_context(i_context)
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
        if (m_entitySubID >= m_entityCount)
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
          m_manager = &::GetManager<T>(*group);
          m_flagManager = &::GetManager<CF>(*group);

          m_entityCount    = group->GetEntityCount();
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
            if (m_entitySubID < m_entityCount)
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
        if (m_entitySubID >= m_entityCount)
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

  inline Iterator begin() { return Iterator(m_context); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  Context<E>& m_context;
};

template <class T, class CF, class E>
auto IterEntity(Context<E> &i_context) { return IterEntityProcessF1<T, CF, E>(i_context); }


