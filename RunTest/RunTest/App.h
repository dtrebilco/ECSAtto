//======================================================================
// BlendDemo
//      By Damian Trebilco
//======================================================================

#include "../Framework3/OpenGL/OpenGLApp.h"
#include <vector>

class App : public OpenGLApp
{
public:

  char *getTitle() const override { return "Pre-Multiply Blend Demo"; }
  bool init() override;

  bool onKey(const uint key, const bool pressed) override;
  bool onMouseButton(const int x, const int y, const MouseButton button, const bool pressed) override;
  bool onMouseMove(const int x, const int y, const int deltaX, const int deltaY) override;

  bool load() override;

  void drawFrame() override;
  void updatePFX(float i_delta);

protected:
  
  enum class ParticleType : uint8_t
  {
    Additive = 0,
    Multiply,
    Blend,
    BlendAddMul, //!< Blend, add and multiply in one blend mode

    MAX
  };

  struct Particle
  {
    inline Particle() { Reset(); }
    void Reset();

    vec2 m_position = vec2(0.0f, 0.0f);
    float m_alpha = 0.0f;
    float m_size = 0.0f;
    float m_rotation = 0.0f;
    ParticleType m_type = ParticleType::Additive;

    vec2 m_direction = vec2(0.0f, 1.0f);
    float m_alphaDelta = 0.1f;
    float m_sizeDelta = 0.0f;
    float m_rotationDelta = 0.0f;
  };
  
  SamplerStateID trilinearClamp, trilinearAniso, radialFilter;

  bool m_mouseLeftDown = false;     //!< If the mouse button is down
  int32_t m_divPos = 0;             //!< The divider position

  std::vector<Particle> m_particles;//!< The array of render particles

  TextureID m_texBackground;        //<! The bcakground texture

  TextureID m_texAdditve;           //!< The additive texture
  TextureID m_texMultiply;          //!< The multiply texture
  TextureID m_texBlend;             //!< The blend texture

  BlendStateID m_blendModeAdditve;  //!< The additive blend mode (ONE, ONE)
  BlendStateID m_blendModeMultiply; //!< The multiply blend mode (ZERO, ONE_MINUS_SRC_COLOR)
  BlendStateID m_blendModeBlend;    //!< The stadard post multiply alpha blend mode (SRC_ALPHA, ONE_MINUS_SRC_ALPHA)

  TextureID m_texPreMul;           //!< The pre multiply texture
  BlendStateID m_blendModePreMul;  //!< The pre multiply blend mode (ONE, ONE_MINUS_SRC_ALPHA)
}; 
