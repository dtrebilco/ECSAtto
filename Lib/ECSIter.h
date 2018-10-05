#pragma once
#include "ECS.h"

// DT_TODO: Docs on each iterator type and how to use it

template <class T>
struct IterProcessValue : public T::ComponentType
{
protected:
  uint16_t m_groupIndex = 0;

public:
  inline EntityID GetGroupID() const { return (GroupID)m_groupIndex; }
};

template <class T>
struct IterProcessValueID : public IterProcessValue<T>
{
public:
  // Note: This relies on the component type implementing GetSubID() - use Iter/IterEntities instead on compile failure
  inline EntityID GetEntityID() const { return EntityID{ (GroupID)m_groupIndex, GetSubID() }; }
};

template <class T, class E, typename V>
class IterProcess
{
public:
  IterProcess(const Context<E> &i_context) : m_context(i_context) {}

  struct Iterator : public V
  {
    uint16_t m_componentCount = 0;
    const Context<E>& m_context;

    inline Iterator(const Context<E> &i_context)
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
      for (; m_groupIndex < m_context.GetGroups().size(); m_groupIndex++)
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
      }
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline V& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  const Context<E>& m_context;
};

template <class T, class E>
auto Iter(const Context<E> &i_context) { return IterProcess<T, E, IterProcessValue<T>>(i_context); }

template <class T, class E>
auto IterID(const Context<E> &i_context) { return IterProcess<T, E, IterProcessValueID<T>>(i_context); }


template <class T, class E>
class IterEntityProcess
{
public:

  IterEntityProcess(const Context<E> &i_context) : m_context(i_context) {}

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
    const Context<E>& m_context;

    inline Iterator(const Context<E> &i_context)
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
      for (; m_groupIndex < m_context.GetGroups().size(); m_groupIndex++)
      {
        E* group = m_context.GetGroups()[m_groupIndex];
        if (group != nullptr)
        {
          m_manager = &::GetManager<T>(*group);
          m_componentCount = m_manager->GetComponentCount();
          if (m_componentCount > 0)
          {
            // Skip starting zero areas
            for (uint64_t bits : m_manager->GetBits())
            {
              if (bits != 0)
              {
                m_bits = bits;
                break;
              }
              m_entitySubID += 64;
            }

            // If not the first bit, go to the next
            if ((m_bits & 0x1) == 0)
            {
              UpdateEntityID();
            }
            break;
          }
        }
      }
    }

    void UpdateEntityID()
    {
      do
      {
        // Go to next bit
        m_bits >>= 1;
        m_entitySubID++;
        if ((m_entitySubID & 0x3F) == 0)
        {
          // Skip long runs of 0 bits // DT_TODO: Test
          for (uint32_t i = (m_entitySubID >> 6); ; i++)
          {
            m_bits = m_manager->GetBits()[i];
            if (m_bits != 0)
            {
              break;
            }
            m_entitySubID += 64;
          }
        }
      } while ((m_bits & 0x1) == 0);
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  const Context<E>& m_context;
};

template <class T, template<class> class C, class E>
auto IterEntity(const C<E> &i_context) { return IterEntityProcess<T, E>(i_context); }


template <class T, class E, typename... Args>
class IterEntityProcessF
{
public:

  IterEntityProcessF(const Context<E> &i_context) : m_context(i_context) {}

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
    uint64_t m_testBit;
    uint16_t m_entityCount;
    E*       m_group = nullptr;

    const Context<E>& m_context;

    inline Iterator(const Context<E> &i_context)
      : m_context(i_context)
    {
      UpdateGroupIndex();
    }

    template<typename H>
    inline uint64_t GetFlagBits(uint16_t i_index) const
    {
      return GetManager<H>(*m_group).GetBits()[i_index];
    }

    template<typename H, typename... Tail, typename = typename std::enable_if<(sizeof...(Tail)) != 0>::type>
    inline uint64_t GetFlagBits(uint16_t i_index) const
    {
      return GetFlagBits<H>(i_index) &
             GetFlagBits<Tail...>(i_index);
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
        uint64_t flagBits = m_bits & GetFlagBits<Args...>(m_entitySubID >> 6);
        UpdateEntityID(flagBits);

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
      for (; m_groupIndex < m_context.GetGroups().size(); m_groupIndex++)
      {
        m_group = m_context.GetGroups()[m_groupIndex];
        if (m_group != nullptr)
        {
          m_manager = &::GetManager<T>(*m_group);

          m_entityCount    = m_group->GetEntityCount();
          m_componentCount = m_manager->GetComponentCount();

          if (m_componentCount > 0)
          {
            m_entitySubID = 0;
            m_testBit = 0x1;

            // Skip starting zero areas
            for (uint32_t i = 0; i < m_manager->GetBits().size(); i++)
            {
              m_bits = m_manager->GetBits()[i];
              if (m_bits != 0)
              {
                uint64_t flagBits = m_bits & GetFlagBits<Args...>(i);
                if (flagBits != 0)
                {
                  m_index = m_manager->GetPrevSum()[i];

                  // If not the first bit, go to the next
                  if ((flagBits & m_testBit) == 0)
                  {
                    UpdateEntityID(flagBits);
                  }
                  break;
                }
              }
              m_entitySubID += 64;
            }

            // Only break if an id is set
            if (m_entitySubID < m_entityCount)
            {
              break;
            }
          }
        }
      }
    }

    void UpdateEntityID(uint64_t flagBits)
    {
      do
      {
        // Go to next bit
        m_index += (m_bits & m_testBit) != 0 ? 1 : 0;
        m_testBit <<= 1;
        m_entitySubID++;
        if (m_entitySubID >= m_entityCount)
        {
          return;
        }

        // If going to the next group of 64
        if (m_testBit == 0)
        {
          m_testBit = 0x1;

          // Skip long runs of 0 bits // DT_TODO: Test
          for (uint32_t i = (m_entitySubID >> 6); i < m_manager->GetBits().size(); i++)
          {
            m_bits = m_manager->GetBits()[i];
            if (m_bits != 0)
            {
              flagBits = m_bits & GetFlagBits<Args...>(i);
              if (flagBits != 0)
              {
                m_index = m_manager->GetPrevSum()[i];
                break;
              }
            }
            m_entitySubID += 64;
          }
          if (m_entitySubID >= m_entityCount)
          {
            return;
          }
        }

      } while ((flagBits & m_testBit) == 0);
    }

    inline bool operator != (uint16_t a_other) const { return this->m_groupIndex != a_other; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context); }
  inline uint16_t end() { return (uint16_t)m_context.GetGroups().size(); }

  const Context<E>& m_context;
};

template <class T, typename... Args, template<class> class C, class E, typename = typename std::enable_if<(sizeof...(Args)) != 0>::type>
auto IterEntity(const C<E> &i_context) { return IterEntityProcessF<T, E, Args...>(i_context); }


