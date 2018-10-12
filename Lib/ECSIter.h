#pragma once
#include "ECS.h"

/// \brief Component iterators
///  There a three main iterator types for iterating over components:
///  - Iter<A> - To iterate over each component of the type. Fastest, but cannot access other component siblings.
///
///  - IterID<A> - If a component stores the entity sub-ID and implements GetSubID(), (eg inherits ComponentTypeIDManager) 
///    this iterator can be used. Just as fast as Iter<A> and can access siblings. Useful for sparse components.
///
///  - IterEntity<A> - Iterates each entity in the context, stopping at entities that have the component. 
///    Can filter on as many components/flags as necessary. (eg IterEntity<A, B, C, D...> will only stop on entities that have all listed components/flags)
///    First filter type must be a component and not a flag.
///   
///  Example usage: 
///         for (auto& i : Iter<A>(context))
///         { *i = foo; // Access component data (what methods are available depends on the component)
///
///         for (auto& i : IterID<A>(context))
///         { *i = foo;       // Access component data like a pointer
///           i.GetEntityID() // Access other components with the entity ID
///
///         for (auto& i : IterEntity<A, B>(context))
///         { *i = foo;       // Access component A data like a pointer
///           i.GetEntityID() // Entity has component A and component/flag B
///
///  To restrict iteration to an entity group, pass the group ID as a second argument to any of the iterator types.
///  Example:
///         for (auto& i : IterEntity<A, B>(context, groupID))
///         { i.GetEntityID() // Entity will be in the passed group
///

template <class T>
struct IterProcessValue : public T::Component
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


template <class T, typename V>
class IterProcessGroup
{
public:
  IterProcessGroup(GroupID i_group, T &i_manager) : m_group(i_group), m_manager(i_manager) {}

  struct Iterator : public V
  {
    inline Iterator(GroupID i_group, T &i_manager) { m_groupIndex = (uint16_t)i_group; m_manager = &i_manager; }
    inline Iterator& operator++() { m_index++; return *this; }
    inline bool operator != (uint16_t a_other) const { return this->m_index != a_other; }
    inline V& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_group, m_manager); }
  inline uint16_t end() { return m_manager.GetComponentCount(); }

  GroupID m_group;
  T& m_manager;
};

template <class T, class E>
auto Iter(const Context<E> &i_context, GroupID i_groupID) { return IterProcessGroup<T, IterProcessValue<T>>(i_groupID, GetManager<T>(*i_context.GetGroup(i_groupID))); }

template <class T, class E>
auto IterID(const Context<E> &i_context, GroupID i_groupID) { return IterProcessGroup<T, IterProcessValueID<T>>(i_groupID, GetManager<T>(*i_context.GetGroup(i_groupID))); }


template <class T, class E>
class IterEntityProcess
{
public:

  IterEntityProcess(const Context<E> &i_context) : m_context(i_context) {}

  struct Value : public T::Component
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
          // Skip long runs of 0 bits
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


template <class T>
class IterEntityProcessGroup
{
public:

  IterEntityProcessGroup(GroupID i_group, T &i_manager) : m_group(i_group), m_manager(i_manager) {}

  struct Value : public T::Component
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
    uint64_t m_bits = 0;

    inline Iterator(GroupID i_group, T &i_manager) 
    {
      m_groupIndex = (uint16_t)i_group;
      m_manager = &i_manager;

      if (m_manager->GetComponentCount() > 0)
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
      }
    }

    inline Iterator& operator++()
    {
      m_index++;
      if (m_index < m_manager->GetComponentCount())
      {
        UpdateEntityID();
      }
      return *this;
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
          // Skip long runs of 0 bits
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

    inline bool operator != (uint16_t a_other) const { return this->m_index != a_other; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_group, m_manager); }
  inline uint16_t end() { return m_manager.GetComponentCount(); }

  GroupID m_group;
  T& m_manager;
};

template <class T, template<class> class C, class E>
auto IterEntity(const C<E> &i_context, GroupID i_groupID) { return IterEntityProcessGroup<T>(i_groupID, GetManager<T>(*i_context.GetGroup(i_groupID))); }



template <class T, class E, typename... Args>
class IterEntityProcessF
{
public:

  IterEntityProcessF(const Context<E> &i_context) : m_context(i_context) {}

  struct Value : public T::Component
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

          // Skip long runs of 0 bits
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



template <class T, class E, typename... Args>
class IterEntityProcessGroupF
{
public:

  IterEntityProcessGroupF(GroupID i_groupID, E &i_group) : m_groupID(i_groupID), m_group(i_group) {}

  struct Value : public T::Component
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
    uint64_t m_bits = 0;
    uint64_t m_testBit = 0x1;
    E&       m_group;

    inline Iterator(GroupID i_groupID, E &i_group)
    : m_group(i_group)
    {
      m_groupIndex = (uint16_t)i_groupID;
      m_manager = &::GetManager<T>(m_group);
      m_componentCount = m_manager->GetComponentCount();

      if (m_componentCount > 0)
      {
        m_entitySubID = 0;
        m_index = m_componentCount; // Set initial index in case no values found

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
      }
    }

    template<typename H>
    inline uint64_t GetFlagBits(uint16_t i_index) const
    {
      return GetManager<H>(m_group).GetBits()[i_index];
    }

    template<typename H, typename... Tail, typename = typename std::enable_if<(sizeof...(Tail)) != 0>::type>
    inline uint64_t GetFlagBits(uint16_t i_index) const
    {
      return GetFlagBits<H>(i_index) &
             GetFlagBits<Tail...>(i_index);
    }

    inline Iterator& operator++()
    {
      if (m_index < m_componentCount)
      {
        uint64_t flagBits = m_bits & GetFlagBits<Args...>(m_entitySubID >> 6);
        UpdateEntityID(flagBits);
      }

      return *this;
    }

    void UpdateEntityID(uint64_t flagBits)
    {
      do
      {
        // Go to next bit
        m_index += (m_bits & m_testBit) != 0 ? 1 : 0;
        m_testBit <<= 1;
        m_entitySubID++;
        if (m_index >= m_componentCount)
        {
          return;
        }

        // If going to the next group of 64
        if (m_testBit == 0)
        {
          m_testBit = 0x1;

          // Skip long runs of 0 bits
          m_index = m_componentCount;
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
          if (m_index >= m_componentCount)
          {
            return;
          }
        }

      } while ((flagBits & m_testBit) == 0);
    }

    inline bool operator != (uint16_t) const { return this->m_index < this->m_componentCount; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_groupID, m_group); }
  inline uint16_t end() { return 0; }

  GroupID m_groupID;
  E& m_group;
};

template <class T, typename... Args, template<class> class C, class E, typename = typename std::enable_if<(sizeof...(Args)) != 0>::type>
auto IterEntity(const C<E> &i_context, GroupID i_groupID) { return IterEntityProcessGroupF<T, E, Args...>(i_groupID, *i_context.GetGroup(i_groupID)); }


