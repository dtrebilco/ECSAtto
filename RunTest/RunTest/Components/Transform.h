#pragma once

#include "../../../Lib/ECS.h"
#include "../../Framework3/Math/Vector.h"


class Transform;
class TransformManager : public ComponentManager
{
public:

  struct ParentChild
  {
    EntityID m_parent;
    EntityID m_child;
  };

  // Add destructor that unlinks all parent/child links
  // If child / parent no in this manager - mark for delete
  // Or do this externally?

  typedef Transform ComponentType;

  inline void OnComponentAdd(uint16_t i_index)
  {
    m_positions.insert(m_positions.begin() + i_index, vec3());
    m_rotations.insert(m_rotations.begin() + i_index, quat());
    m_scales.insert(m_scales.begin() + i_index, vec3());

    m_globalPositions.insert(m_globalPositions.begin() + i_index, vec3());
    m_globalRotations.insert(m_globalRotations.begin() + i_index, quat());
    m_globalScales.insert(m_globalScales.begin() + i_index, vec3());

    m_parentChilds.insert(m_parentChilds.begin() + i_index, ParentChild{ EntityID_None, EntityID_None });
    m_siblings.insert(m_siblings.begin() + i_index, EntityID_None);
  }

  virtual void OnComponentRemove(uint16_t i_index)
  {
    m_positions.erase(m_positions.begin() + i_index);
    m_rotations.erase(m_rotations.begin() + i_index);
    m_scales.erase(m_scales.begin() + i_index);

    m_globalPositions.erase(m_globalPositions.begin() + i_index);
    m_globalRotations.erase(m_globalRotations.begin() + i_index);
    m_globalScales.erase(m_globalScales.begin() + i_index);

    m_parentChilds.erase(m_parentChilds.begin() + i_index);
    m_siblings.erase(m_siblings.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_positions.reserve(i_count);
    m_rotations.reserve(i_count);
    m_scales.reserve(i_count);

    m_globalPositions.reserve(i_count);
    m_globalRotations.reserve(i_count);
    m_globalScales.reserve(i_count);

    m_parentChilds.reserve(i_count);
    m_siblings.reserve(i_count);
  }

  std::vector<vec3> m_positions; //!< The positions
  std::vector<quat> m_rotations; //!< The rotations
  std::vector<vec3> m_scales;    //!< The scales

  std::vector<vec3> m_globalPositions; //!< The global positions
  std::vector<quat> m_globalRotations; //!< The global rotations
  std::vector<vec3> m_globalScales;    //!< The global scales

  std::vector<ParentChild> m_parentChilds; //!< The parent/child relationship arrays
  std::vector<EntityID> m_siblings;        //!< The array of siblings (only points to next sibling - siblings are sorted by entity IDs)
};


class Transform : public ComponentBase<TransformManager>
{
public:

  inline vec3& GetPosition()
  {
    return m_manager->m_positions[m_index];
  }

  inline quat& GetRotation()
  {
    return m_manager->m_rotations[m_index];
  }

  inline vec3& GetScale()
  {
    return m_manager->m_scales[m_index];
  }

  inline vec3& GetGlobalPosition()
  {
    return m_manager->m_globalPositions[m_index];
  }

  inline quat& GetGlobalRotation()
  {
    return m_manager->m_globalRotations[m_index];
  }

  inline vec3& GetGlobalScale()
  {
    return m_manager->m_globalScales[m_index];
  }

  inline EntityID& GetParent()
  {
    return m_manager->m_parentChilds[m_index].m_parent;
  }

  inline EntityID& GetChild()
  {
    return m_manager->m_parentChilds[m_index].m_child;
  }

  inline EntityID& GetSibling()
  {
    return m_manager->m_siblings[m_index];
  }

  mat4 CalculateModelWorld()
  {
    //mat4 modelWorld = mat4(1.0f);
    //modelWorld = glm::translate(modelWorld, GetPosition());
    //modelWorld *= glm::mat4_cast(GetRotation());
    //modelWorld = glm::scale(modelWorld, GetScale());
    
    mat4 modelWorld;
    const vec3& scale = GetScale();
    const quat& q = GetRotation();

    vec3 scale2 = scale * 2.0f;

    float qxx(q.x * q.x);
    float qyy(q.y * q.y);
    float qzz(q.z * q.z);
    float qxz(q.x * q.z);
    float qxy(q.x * q.y);
    float qyz(q.y * q.z);
    float qwx(q.w * q.x);
    float qwy(q.w * q.y);
    float qwz(q.w * q.z);

    modelWorld[0][0] = scale.x - scale2.x * (qyy + qzz);
    modelWorld[0][1] = scale2.x * (qxy + qwz);
    modelWorld[0][2] = scale2.x * (qxz - qwy);
    modelWorld[0][3] = 0.0f;

    modelWorld[1][0] = scale2.y * (qxy - qwz);
    modelWorld[1][1] = scale.y - scale2.y * (qxx + qzz);
    modelWorld[1][2] = scale2.y * (qyz + qwx);
    modelWorld[1][3] = 0.0f;

    modelWorld[2][0] = scale2.z * (qxz + qwy);
    modelWorld[2][1] = scale2.z * (qyz - qwx);
    modelWorld[2][2] = scale.z - scale2.z * (qxx + qyy);
    modelWorld[2][3] = 0.0f;

    modelWorld[3] = vec4(GetPosition(), 1.0f);
    return modelWorld;
  }

  mat4x3 CalculateModelWorld4x3()
  {
    mat4x3 modelWorld;

    const vec3& scale = GetScale();
    const quat& q = GetRotation();

    vec3 scale2 = scale * 2.0f;

    float qxx(q.x * q.x);
    float qyy(q.y * q.y);
    float qzz(q.z * q.z);
    float qxz(q.x * q.z);
    float qxy(q.x * q.y);
    float qyz(q.y * q.z);
    float qwx(q.w * q.x);
    float qwy(q.w * q.y);
    float qwz(q.w * q.z);

    modelWorld[0][0] = scale.x - scale2.x * (qyy + qzz);
    modelWorld[0][1] = scale2.x * (qxy + qwz);
    modelWorld[0][2] = scale2.x * (qxz - qwy);

    modelWorld[1][0] = scale2.y * (qxy - qwz);
    modelWorld[1][1] = scale.y - scale2.y * (qxx + qzz);
    modelWorld[1][2] = scale2.y * (qyz + qwx);

    modelWorld[2][0] = scale2.z * (qxz + qwy);
    modelWorld[2][1] = scale2.z * (qyz - qwx);
    modelWorld[2][2] = scale.z - scale2.z * (qxx + qyy);

    modelWorld[3] = GetPosition();

    return modelWorld;
  }
};

