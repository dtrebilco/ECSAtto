#pragma once

#include <ECS.h>
#include "../Utils.h"

class Transform;
class TransformManager : public ComponentManager
{
public:

  struct ParentChild
  {
    EntityID m_parent;
    EntityID m_child;
  };

  typedef Transform ComponentType;

  inline void OnComponentAdd(EntityID i_entity, uint16_t i_index)
  {
    m_positions.insert(m_positions.begin() + i_index, vec3());
    m_rotations.insert(m_rotations.begin() + i_index, quat());
    m_scales.insert(m_scales.begin() + i_index, vec3());

    m_parentChilds.insert(m_parentChilds.begin() + i_index, ParentChild{ EntityID_None, EntityID_None });
    m_siblings.insert(m_siblings.begin() + i_index, EntityID_None);
  }

  void OnComponentRemove(EntityID i_entity, uint16_t i_index) override
  {
    AT_ASSERT(m_parentChilds[i_index].m_parent == EntityID_None);
    AT_ASSERT(m_parentChilds[i_index].m_child == EntityID_None);
    AT_ASSERT(m_siblings[i_index] == EntityID_None);

    m_positions.erase(m_positions.begin() + i_index);
    m_rotations.erase(m_rotations.begin() + i_index);
    m_scales.erase(m_scales.begin() + i_index);

    m_parentChilds.erase(m_parentChilds.begin() + i_index);
    m_siblings.erase(m_siblings.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_positions.reserve(i_count);
    m_rotations.reserve(i_count);
    m_scales.reserve(i_count);

    m_parentChilds.reserve(i_count);
    m_siblings.reserve(i_count);
  }

  std::vector<vec3> m_positions; //!< The positions
  std::vector<quat> m_rotations; //!< The rotations
  std::vector<vec3> m_scales;    //!< The scales

  std::vector<ParentChild> m_parentChilds; //!< The parent/child relationship arrays
  std::vector<EntityID>    m_siblings;     //!< The array of siblings (only points to next sibling - siblings are sorted by entity IDs)
};


class Transform : public ComponentBase<TransformManager>
{
public:

  inline vec3& GetPosition() { return m_manager->m_positions[m_index]; }
  inline quat& GetRotation() { return m_manager->m_rotations[m_index]; }
  inline vec3& GetScale()    { return m_manager->m_scales[m_index]; }

  inline EntityID& GetParent()  { return m_manager->m_parentChilds[m_index].m_parent; }
  inline EntityID& GetChild()   { return m_manager->m_parentChilds[m_index].m_child; }
  inline EntityID& GetSibling() { return m_manager->m_siblings[m_index]; }
};


class GlobalTransform;
class GlobalTransformManager : public ComponentManager
{
public:

  typedef GlobalTransform ComponentType;

  inline void OnComponentAdd(EntityID i_entity, uint16_t i_index)
  {
    m_globalTransform.insert(m_globalTransform.begin() + i_index, mat4x3());
    m_globalScales.insert(m_globalScales.begin() + i_index, vec3());
  }

  void OnComponentRemove(EntityID i_entity, uint16_t i_index) override
  {
    m_globalTransform.erase(m_globalTransform.begin() + i_index);
    m_globalScales.erase(m_globalScales.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_globalTransform.reserve(i_count);
    m_globalScales.reserve(i_count);
  }

  std::vector<mat4x3> m_globalTransform; //!< The global transform without scale
  std::vector<vec3>   m_globalScales;    //!< The global scales
};


class GlobalTransform : public ComponentBase<GlobalTransformManager>
{
public:

  inline vec3&   GetGlobalPosition()  { return m_manager->m_globalTransform[m_index][3]; }
  inline mat4x3& GetGlobalTransform() { return m_manager->m_globalTransform[m_index]; }
  inline vec3&   GetGlobalScale()     { return m_manager->m_globalScales[m_index]; }
};


