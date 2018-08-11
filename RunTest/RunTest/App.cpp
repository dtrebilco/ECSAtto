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

#include "..\..\Lib\ECSIter.h"

extern long g_allocRequestCount;
extern long g_allocFreeCount;

BaseApp *CreateApp() { return new App(); }

bool s_mouseLeftDown = false;
bool s_mouseRightDown = false;
int32_t s_mouseDownX = 0;
int32_t s_mouseDownY = 0;

bool dummy = false;

App::App()
{
}

bool App::init()
{
/*
  m_context.ReserveGroups(10);
  GroupID groupID1 = m_context.AddEntityGroup();
  GroupID groupID2 = m_context.AddEntityGroup();

  m_context.ReserveEntities(groupID1, 20);
  EntityID entity1 = m_context.AddEntity(groupID1);
  EntityID entity2 = m_context.AddEntity(groupID2);
  EntityID entity3 = m_context.AddEntity(groupID2);

  bool isValidG1 = m_context.IsValid(groupID1);
  bool isValidG2 = m_context.IsValid(groupID2);

  //m_context.RemoveEntityGroup(groupID2);
  bool isValidG2b = m_context.IsValid(groupID2);

  bool isvalidE1 = m_context.IsValid(entity1);
  bool isvalidE2 = m_context.IsValid(entity2);
  bool isvalidE3 = m_context.IsValid(entity3);

  m_context.ReserveComponent(groupID1, &GameGroup::m_transforms, 20);
  m_context.AddComponent(entity1, &GameGroup::m_transforms, vec3(1,1,1));
  m_context.AddComponent(entity2, &GameGroup::m_transforms);
  m_context.AddComponent(entity3, &GameGroup::m_transforms);
  bool hasComponent = m_context.HasComponent(entity1, &GameGroup::m_transforms);
  m_context.RemoveComponent(entity1, &GameGroup::m_transforms);

  vec3 sum = vec3(0);
  for (auto v : CreateIter(m_context, &GameGroup::m_transforms))
  {
    sum += v.m_component->GetData(v.m_componentIndex);
  }

  vec3 sum2 = vec3(0);
  for (auto v : CreateIDIter(m_context, &GameGroup::m_transforms))
  {
    EntityID id = v.GetEntityID();
    sum2 += v.m_component->GetData(v.m_componentIndex);
  }


  m_context.RemoveEntity(entity1);

  bool hasFlag1 = m_context.HasFlag(entity1, &GameGroup::m_flagTest);

  m_context.SetFlag(entity1, &GameGroup::m_flagTest, true);
  bool hasFlag2 = m_context.HasFlag(entity1, &GameGroup::m_flagTest);

  m_context.SetFlag(entity1, &GameGroup::m_flagTest, false);
  bool hasFlag3 = m_context.HasFlag(entity1, &GameGroup::m_flagTest);

  dummy = isValidG1 | isValidG2 | isValidG2b | isvalidE1 | isvalidE2 | isvalidE3 | hasComponent | hasFlag1 | hasFlag2 | hasFlag3;
*/

  m_context.ReserveGroups(2);
  m_staticGroup = m_context.AddEntityGroup();

  m_context.ReserveEntities(m_staticGroup, 10000);
  for (uint32_t y = 0; y < 100; y++)
  {
    for (uint32_t x = 0; x < 100; x++)
    {
      EntityID newEntity = m_context.AddEntity(m_staticGroup);
      Transform newTransform = m_context.AddComponent(newEntity, &GameGroup::m_transforms);
      newTransform.GetPosition() = vec3((float)x + 0.5f, 0.5f, (float)y + 0.5f);
    }
  }
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

  // Update transform systems
  for (auto v : CreateIterT<TransformManager>(m_context))
  {
    vec3& pos = v.GetPosition();
    pos.y = cosf(pos.x + time) + sinf(pos.z + time);
  }
  


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
  //for (auto v : CreateIter(m_context, &GameGroup::m_transforms))
  //{
  //  DrawBox(v.m_manager->GetData(v.m_componentIndex), 0.25f);
  //}

  for (auto v : CreateIterT<TransformManager>(m_context))
  {
    DrawBox(v.GetPosition(), 0.25f);
  }


  

  /*
  for (uint32_t x = 0; x < 100; x++)
  {
    for (uint32_t y = 0; y < 100; y++)
    {
      DrawBox(vec3(float(x) + 0.5f, 0.5f, float(y) + 0.5f), 0.25f);
    }
  }*/
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
