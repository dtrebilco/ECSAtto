//======================================================================
// ECS Runtime test
//      By Damian Trebilco
//======================================================================

#include "../Framework3/OpenGL/OpenGLApp.h"
#include "../../Lib/ECS.h"

class TransformManager2 : public ComponentTypeManager<vec3> {};

class Transform;
class TransformManager : public ComponentManager
{
public:

  typedef Transform ComponentType;

  inline void OnComponentAdd(uint16_t i_index)
  {
    m_positions.insert(m_positions.begin() + i_index, vec3());
    m_rotations.insert(m_rotations.begin() + i_index, quat());
    m_scales.insert(m_scales.begin() + i_index, vec3());
  }

  virtual void OnComponentRemove(uint16_t i_index)
  {
    m_positions.erase(m_positions.begin() + i_index);
    m_rotations.erase(m_rotations.begin() + i_index);
    m_scales.erase(m_scales.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_positions.reserve(i_count);
    m_rotations.reserve(i_count);
    m_scales.reserve(i_count);
  }

  std::vector<vec3> m_positions;      //!< The positions
  std::vector<quat> m_rotations; //!< The rotations
  std::vector<vec3> m_scales;         //!< The scales

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


class GameGroup : public EntityGroup
{
public:

  GameGroup()
  {
    AddManager(&m_transforms);
    AddManager(&m_flagTest);
  }

  TransformManager m_transforms;
  FlagManager m_flagTest;
};


template<>
TransformManager GameGroup::* GetManager<TransformManager, GameGroup>() { return &GameGroup::m_transforms; }


class App : public OpenGLApp {
public:

  App();

  char *getTitle() const override { return "ESC Test"; }
  bool init() override;

  bool onKey(const uint key, const bool pressed) override;
  bool onMouseButton(const int x, const int y, const MouseButton button, const bool pressed) override;
  bool onMouseMove(const int x, const int y, const int deltaX, const int deltaY) override;

  void resetCamera() override;
  bool load() override;

  void drawFrame() override;

protected:

  mat4 m_projection; //!< The projection matrix used
  SamplerStateID trilinearClamp, trilinearAniso, radialFilter;

  Context<GameGroup> m_context;

  GroupID m_staticGroup = GroupID(0);

  //TextureID m_perlin; 
  //ShaderID m_gridDraw;
};
