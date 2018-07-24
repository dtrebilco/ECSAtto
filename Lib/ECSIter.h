#pragma once
#include "ECS.h"

template <class E, class T>
class Iter
{
public:

  using IterType = decltype(std::declval<std::vector<E*>>().begin());

  Iter(Context<E> &i_context, T E::*i_member) : m_context(i_context), m_member(i_member) {}

  /*
  void Process()
  {
    for (E& group : m_context.m_groups)
    {
      A& value = group.*m_member;
      value.i = 7;
    }
  }*/

  struct Value
  {
  protected:

    Context<E>& m_context;
    T E::*m_member;
    IterType m_iter;

    inline Value(Context<E> &i_context, T E::*i_member, IterType i_iter, T* i_manager, uint16_t i_managerIndex) :
      m_context(i_context), m_member(i_member), m_iter(i_iter),  {}

  public:

    T* m_manager;
    uint16_t m_managerIndex;
  };

  struct Iterator : public Value
  {
    inline Iterator(Context<E> &i_context, T E::*i_member, IterType i_iter) : Value(i_context, i_member, i_iter) {}

    inline Iterator& operator++()
    {
      ++this->m_iter;
      return *this;
    }

    inline bool operator != (const Iterator& a_other) const { return this->m_iter != a_other.m_iter; }
    inline Value& operator *() { return *this; }
  };

  inline Iterator begin() { return Iterator(m_context, m_member, m_context.m_groups.begin()); }
  inline Iterator end() { return Iterator(m_context, m_member, m_context.m_groups.end()); }

  Context<E>& m_context;
  T E::*m_member;
};

template <class E, class T>
auto CreateIter(Context<E> &i_context, T E::*member) { return Iter<E, T>(i_context, member); }


