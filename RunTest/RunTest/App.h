//======================================================================
// ECS Runtime test
//      By Damian Trebilco
//======================================================================

#include "../Framework3/OpenGL/OpenGLApp.h"
#include "../../Lib/ECS.h"

class TransformManager : public ComponentTypeManager<vec3> {};


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
