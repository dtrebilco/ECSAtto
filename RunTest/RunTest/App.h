//======================================================================
// ECS Runtime test
//      By Damian Trebilco
//======================================================================

#include "../Framework3/OpenGL/OpenGLApp.h"
#include "../../Lib/ECS.h"

class TransformManager : public ComponentTypeManager<vec3>
{
public:
  TransformManager(EntityGroup i_group) : ComponentTypeManager<vec3>(i_group) {}
};


class GameGroup : public EntityGroup
{
public:

  GameGroup()
  : m_transforms(*this)
  {
  }

  TransformManager m_transforms;
};


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

  //TextureID m_perlin; 
  //ShaderID m_gridDraw;
};
