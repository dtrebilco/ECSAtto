//======================================================================
// ECS Runtime test
//      By Damian Trebilco
//======================================================================

#include "App.h"
#include <crtdbg.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream> 
#include <algorithm>
#include <time.h>

extern long g_allocRequestCount;
extern long g_allocFreeCount;

BaseApp *CreateApp() { return new App(); }

bool s_mouseLeftDown = false;
bool s_mouseRightDown = false;
int32_t s_mouseDownX = 0;
int32_t s_mouseDownY = 0;

App::App()
{
}

bool App::init()
{
  speed = 100.0f;

  return OpenGLApp::init();
}

void App::resetCamera()
{
  BaseApp::resetCamera();
  camPos = vec3(-8.05934429f, 17.5981026f, -12.2385817f);
  wx = 0.360000521f;
  wy = -0.71699959f;

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
  if ((radialFilter = renderer->addSamplerState(LINEAR, WRAP, CLAMP, CLAMP)) == SS_NONE) return false;

  //if ((m_perlin = renderer->addTexture("Perlin.png", true, trilinearAniso)) == TEXTURE_NONE) return false;
  //if ((m_gridDraw = renderer->addShader("gridDraw.shd")) == SHADER_NONE) return false;

  return true;
}

bool App::onMouseButton(const int x, const int y, const MouseButton button, const bool pressed)
{
  if (button == MOUSE_LEFT)
  {
    s_mouseLeftDown = pressed;
    s_mouseDownX = x;
    s_mouseDownY = y;
  }
  if (button == MOUSE_RIGHT)
  {
    s_mouseRightDown = pressed;
    s_mouseDownX = x;
    s_mouseDownY = y;
  }
  return OpenGLApp::onMouseButton(x, y, button, pressed);
}

bool App::onMouseMove(const int x, const int y, const int deltaX, const int deltaY)
{
  if (s_mouseLeftDown ||
    s_mouseRightDown)
  {
    s_mouseDownX = x;
    s_mouseDownY = y;
  }
  return OpenGLApp::onMouseMove(x, y, deltaX, deltaY);
}

void DrawBox(const vec3& i_pos, float i_size)
{
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(i_pos.x - i_size, i_pos.y - i_size, i_pos.z - i_size);
  glVertex3f(i_pos.x - i_size, i_pos.y - i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y - i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y - i_size, i_pos.z - i_size);

  glVertex3f(i_pos.x - i_size, i_pos.y + i_size, i_pos.z - i_size);
  glVertex3f(i_pos.x - i_size, i_pos.y + i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y + i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y + i_size, i_pos.z - i_size);

  glColor3f(1.0f, 1.0f, 0.0f);
  glVertex3f(i_pos.x - i_size, i_pos.y - i_size, i_pos.z - i_size);
  glVertex3f(i_pos.x - i_size, i_pos.y - i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x - i_size, i_pos.y + i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x - i_size, i_pos.y + i_size, i_pos.z - i_size);

  glVertex3f(i_pos.x + i_size, i_pos.y - i_size, i_pos.z - i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y - i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y + i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y + i_size, i_pos.z - i_size);

  glColor3f(0.0f, 1.0f, 1.0f);
  glVertex3f(i_pos.x - i_size, i_pos.y - i_size, i_pos.z - i_size);
  glVertex3f(i_pos.x - i_size, i_pos.y + i_size, i_pos.z - i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y + i_size, i_pos.z - i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y - i_size, i_pos.z - i_size);

  glVertex3f(i_pos.x - i_size, i_pos.y - i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x - i_size, i_pos.y + i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y + i_size, i_pos.z + i_size);
  glVertex3f(i_pos.x + i_size, i_pos.y - i_size, i_pos.z + i_size);
}

void App::drawFrame()
{
  m_projection = perspectiveMatrixX(1.5f, width, height, 0.1f, 4000);
  //mat4 modelview = scale(1.0f, 1.0f, -1.0f) * rotateXY(-wx, -wy) * translate(-camPos) * rotateX(PI * 0.5f);
  mat4 modelview = rotateXY(-wx, -wy) * translate(-camPos);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(value_ptr(m_projection));

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(value_ptr(modelview));

  float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  renderer->clear(true, true, false, clearColor);

  renderer->reset();
  //renderer->setShader(m_gridDraw);
  //renderer->setTexture("perlinTex", m_perlin);
  renderer->apply();

  // Floor
  glColor3f(0.0f, 1.0f, 0.0f);
  glBegin(GL_LINES);
  for (uint32_t i = 0; i <= 100; i++)
  {
    glVertex3i(i, 0, 0);
    glVertex3i(i, 0, 100);
  }
  for (uint32_t i = 0; i <= 100; i++)
  {
    glVertex3i(0, 0, i);
    glVertex3i(100, 0, i);
  }
  glEnd();

  // Boxes
  glBegin(GL_QUADS);
  for (uint32_t x = 0; x < 100; x++)
  {
    for (uint32_t y = 0; y < 100; y++)
    {
      DrawBox(vec3(float(x) + 0.5f, 0.5f, float(y) + 0.5f), 0.25f);
    }
  }
  glEnd();

  renderer->setup2DMode(0, (float)width, 0, (float)height);

  // Draw text data to the screen 
  renderer->reset();
  //renderer->setShader(m_gridDraw);
  //renderer->setTexture("perlinTex", m_perlin);
  renderer->apply();

  renderer->reset();
  renderer->setDepthState(noDepthWrite);
  renderer->apply();

  {
    char buffer[100];
    float xPos = (float)width - 250.0f;

#ifdef _DEBUG
    sprintf(buffer, "Alloc Count %d", g_allocRequestCount);
    renderer->drawText(buffer, xPos, 138.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);

    sprintf(buffer, "Free Count %d", g_allocFreeCount);
    renderer->drawText(buffer, xPos, 168.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);

    sprintf(buffer, "Working Count %d", g_allocRequestCount - g_allocFreeCount);
    renderer->drawText(buffer, xPos, 198.0f, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);
#endif // _DEBUG
  }
}
