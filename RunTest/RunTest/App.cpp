//======================================================================
// BlendDemo
//      By Damian Trebilco
//======================================================================

#include "App.h"

extern long g_allocRequestCount;
extern long g_allocFreeCount;

BaseApp *CreateApp() { return new App(); }

int32_t RandomInt(int32_t i_min, int32_t i_max)
{
  int32_t range = i_max - i_min;
  int32_t random = (int32_t)rand() % (range + 1);

  return random + i_min;
}

float RandomFloat(float i_min, float i_max)
{
  float random = ((float)rand()) / (float)RAND_MAX;

  float range = i_max - i_min;
  return (random * range) + i_min;
}

void App::Particle::Reset()
{
  m_position = vec2(0.0f, 0.0f);
  m_alpha = 1.5f;
  m_size = 1.0f;
  m_rotation = 0.0f;
  m_type = (ParticleType)RandomInt(0, (int32_t)ParticleType::MAX - 1);

  m_direction = vec2(RandomFloat(-4.0f, 4.0f), RandomFloat(4.0f, 10.0f));
  m_alphaDelta = RandomFloat(-0.15f, -0.09f);
  m_sizeDelta = RandomFloat(0.5f, 1.5f);
  m_rotationDelta = RandomFloat(-0.5f, 0.5f);
}

bool App::init()
{
  m_particles.resize(200);

  // Age the pfx system for the first draw
  for (uint32_t i = 0; i < 1000; i++)
  {
    updatePFX(1.0f / 30.0f);
  }

  return OpenGLApp::init();
}

bool App::onKey(const uint key, const bool pressed)
{
  if (pressed)
  {
    //switch (key)
    //{
    //case '1': break;
    //case '2': break;
    //case '3': break;
    //}
  }
  return BaseApp::onKey(key, pressed);
}

bool App::load()
{
  // Set the shader version used
  ((OpenGLRenderer*)renderer)->SetShaderVersionStr("#version 130");

  // Filtering modes
  if ((trilinearClamp = renderer->addSamplerState(TRILINEAR, CLAMP, CLAMP, CLAMP)) == SS_NONE) return false;
  if ((trilinearAniso = renderer->addSamplerState(TRILINEAR_ANISO, WRAP, WRAP, WRAP)) == SS_NONE) return false;
  if ((radialFilter   = renderer->addSamplerState(LINEAR, WRAP, CLAMP, CLAMP)) == SS_NONE) return false;

  if ((m_texBackground = renderer->addTexture("Background.png", true, trilinearAniso)) == TEXTURE_NONE) return false;
  
  if ((m_texAdditve = renderer->addTexture("Additive.png", true, trilinearAniso)) == TEXTURE_NONE) return false;
  if ((m_texMultiply = renderer->addTexture("Multiply.png", true, trilinearAniso)) == TEXTURE_NONE) return false;
  if ((m_texBlend = renderer->addTexture("Blend.png", true, trilinearAniso)) == TEXTURE_NONE) return false;
  if ((m_texPreMul = renderer->addTexture("PreMul.png", true, trilinearAniso)) == TEXTURE_NONE) return false;

  m_blendModeAdditve = renderer->addBlendState(ONE, ONE);
  m_blendModeMultiply = renderer->addBlendState(ZERO, ONE_MINUS_SRC_COLOR);
  m_blendModeBlend = renderer->addBlendState(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
  m_blendModePreMul = renderer->addBlendState(ONE, ONE_MINUS_SRC_ALPHA);

  // Set initial divider position
  m_divPos = width / 2;

  return true;
}

bool App::onMouseButton(const int x, const int y, const MouseButton button, const bool pressed)
{
  if (button == MOUSE_LEFT)
  {
    m_mouseLeftDown = pressed;
    m_divPos = x;
  }
  return OpenGLApp::onMouseButton(x, y, button, pressed);
}

bool App::onMouseMove(const int x, const int y, const int deltaX, const int deltaY)
{
  if (m_mouseLeftDown)
  {
    m_divPos = x;
  }
  return OpenGLApp::onMouseMove(x, y, deltaX, deltaY);
}

void App::updatePFX(float i_delta)
{
  // Update all particles and reset ones that expire
  for (Particle& p : m_particles)
  {
    p.m_position += p.m_direction * i_delta;
    p.m_alpha += p.m_alphaDelta * i_delta;
    p.m_size += p.m_sizeDelta * i_delta;
    p.m_rotation += p.m_rotationDelta * i_delta;

    if (p.m_alpha <= 0.0f)
    {
      p.Reset();
    }
  }
}

void App::drawFrame()
{
  // Update the PFX
  updatePFX(frameTime);

  float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  renderer->clear(true, true, false, clearColor);

  // Draw the background
  float maxY = (float)height / (float)width * 100.0f;
  renderer->setup2DMode(-50.0f, 50.0f, maxY, 0.0f);
  renderer->reset();
  ((OpenGLRenderer*)renderer)->setTexture(m_texBackground);
  renderer->apply();
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-50.0f, maxY);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(50.0f, maxY);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(50.0f, 0.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-50.0f, 0.0f);
  glEnd();

  // Draw the traditional blending
  // Setup scissor
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, m_divPos, height);

  // Render each particle one by one, as the blend mode can change per particle
  uint32_t drawCallCount = 0;
  {
    auto drawQuad = [&](const Particle& p) 
    {
      drawCallCount++;
      glBegin(GL_QUADS);
      vec2 offset1 = vec2(cosf(p.m_rotation), sinf(p.m_rotation)) * p.m_size;
      vec2 offset2 = vec2(-offset1.y, offset1.x);

      glTexCoord2f(0.0f, 0.0f);
      glVertex2fv(value_ptr(p.m_position - offset1 - offset2));

      glTexCoord2f(1.0f, 0.0f);
      glVertex2fv(value_ptr(p.m_position + offset1 - offset2));

      glTexCoord2f(1.0f, 1.0f);
      glVertex2fv(value_ptr(p.m_position + offset1 + offset2));

      glTexCoord2f(0.0f, 1.0f);
      glVertex2fv(value_ptr(p.m_position - offset1 + offset2));
      glEnd();
    };

    auto drawAdditiveMode = [&](const Particle& p)
    {
      renderer->reset();
      renderer->setBlendState(m_blendModeAdditve);
      ((OpenGLRenderer*)renderer)->setTexture(m_texAdditve);
      glColor4f(p.m_alpha, p.m_alpha, p.m_alpha, 1.0f);
      renderer->apply();
      drawQuad(p);
    };

    auto drawMultiplyMode = [&](const Particle& p)
    {
      renderer->reset();
      renderer->setBlendState(m_blendModeMultiply);
      ((OpenGLRenderer*)renderer)->setTexture(m_texMultiply);
      glColor4f(p.m_alpha, p.m_alpha, p.m_alpha, 1.0f);
      renderer->apply();
      drawQuad(p);
    };

    auto drawBlendMode = [&](const Particle& p)
    {
      renderer->reset();
      renderer->setBlendState(m_blendModeBlend);
      ((OpenGLRenderer*)renderer)->setTexture(m_texBlend);
      glColor4f(1.0f, 1.0f, 1.0f, p.m_alpha);
      renderer->apply();
      drawQuad(p);
    };

    for (Particle& p : m_particles)
    {
      // Bind the appropiate blend mode and texture
      switch (p.m_type)
      {
      case(ParticleType::Additive):
        drawAdditiveMode(p);
        break;

      case(ParticleType::Multiply):
        drawMultiplyMode(p);
        break;

      case(ParticleType::Blend):
        drawBlendMode(p);
        break;

      case(ParticleType::BlendAddMul):
        drawBlendMode(p);
        drawAdditiveMode(p);
        drawMultiplyMode(p);
        break;
      }
    }
  }

  // Draw the pre-mul alpha
  // Setup scissor
  glScissor(m_divPos, 0, width, height);
  renderer->reset();
  renderer->setBlendState(m_blendModePreMul);
  ((OpenGLRenderer*)renderer)->setTexture(m_texPreMul);
  renderer->apply();

  glBegin(GL_QUADS);
  for (Particle& p : m_particles)
  {
    vec2 offset1 = vec2(cosf(p.m_rotation), sinf(p.m_rotation)) * p.m_size;
    vec2 offset2 = vec2(-offset1.y, offset1.x);

    float texSize = 0.5f;
    float texOffsetX = 0.0f;
    float texOffsetY = 0.0f;
    switch (p.m_type)
    {
    case(ParticleType::Additive):
      texOffsetX = 0.0f;
      texOffsetY = 0.0f;
      break;

    case(ParticleType::Multiply):
      texOffsetX = texSize;
      texOffsetY = 0.0f;
      break;

    case(ParticleType::Blend):
      texOffsetX = 0.0f;
      texOffsetY = texSize;
      break;

    case(ParticleType::BlendAddMul):
      texOffsetX = texSize;
      texOffsetY = texSize;
      break;
    }

    glColor4f(p.m_alpha, p.m_alpha, p.m_alpha, p.m_alpha);

    glTexCoord2f(texOffsetX, texOffsetY);
    glVertex2fv(value_ptr(p.m_position - offset1 - offset2));

    glTexCoord2f(texOffsetX + texSize, texOffsetY);
    glVertex2fv(value_ptr(p.m_position + offset1 - offset2));

    glTexCoord2f(texOffsetX + texSize, texOffsetY + texSize);
    glVertex2fv(value_ptr(p.m_position + offset1 + offset2));

    glTexCoord2f(texOffsetX, texOffsetY + texSize);
    glVertex2fv(value_ptr(p.m_position - offset1 + offset2));
  }
  glEnd();

  // Reset the scissor
  glDisable(GL_SCISSOR_TEST);

  // Draw the dividing line
  renderer->reset();
  renderer->setup2DMode(0, (float)width, 0, (float)height);
  renderer->apply();
  glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex2i(m_divPos - 1, 0);
    glVertex2i(m_divPos + 1, 0);
    glVertex2i(m_divPos + 1, height);
    glVertex2i(m_divPos - 1, height);
  glEnd();

  // Draw the draw call counts
  {
    char buffer[100];

    sprintf(buffer, "Draw calls = %d", drawCallCount);
    renderer->drawText("Mixed blend mode", 30.0f, 38.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);
    renderer->drawText(buffer, 30.0f, 68.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);

    renderer->drawText("Pre-multiply", (float)width - 200.0f, 38.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);
    renderer->drawText("Draw calls = 1", (float)width - 200.0f, 68.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);

//#ifdef _DEBUG
//    float xPos = (float)width - 250.0f;
//    sprintf(buffer, "Alloc Count %d", g_allocRequestCount);
//    renderer->drawText(buffer, xPos, 138.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);
//    
//    sprintf(buffer, "Free Count %d", g_allocFreeCount);
//    renderer->drawText(buffer, xPos, 168.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);
//    
//    sprintf(buffer, "Working Count %d", g_allocRequestCount - g_allocFreeCount);
//    renderer->drawText(buffer, xPos, 198.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);
//#endif // _DEBUG
  }
}
