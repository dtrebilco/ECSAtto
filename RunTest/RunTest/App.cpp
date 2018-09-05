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
#include "Utils.h"

#include "..\..\Lib\ECSIter.h"

#include "Components/Bounds.h"
#include "Components/Transform.h"

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

  /*
  m_context.ReserveEntities(m_staticGroup, 10000);
  for (uint32_t y = 0; y < 100; y++)
  {
    for (uint32_t x = 0; x < 100; x++)
    {
      EntityID newEntity = m_context.AddEntity(m_staticGroup);
      Transform newTransform = m_context.AddComponent<TransformManager>(newEntity);
      newTransform.GetPosition() = vec3((float)x + 0.5f, 0.5f, (float)y + 0.5f);
      newTransform.GetScale() = vec3(0.25f);

      //if (x % 6 == 0)
      {
        m_context.SetFlag<FlagManager>(newEntity, true);
      }

      auto newBounds = m_context.AddComponent<BoundingManager>(newEntity);
      newBounds.SetCenter(newTransform.GetPosition());
      newBounds.SetExtents(newTransform.GetScale());
    }
  }
  */
  {
    EntityID entity1 = m_context.AddEntity(m_staticGroup);
    {
      Transform newTransform = m_context.AddComponent<TransformManager>(entity1);
      newTransform.GetPosition() = vec3(2.0f, 1.0f, 2.0f);
      newTransform.GetScale() = vec3(1.0f, 0.5f, 1.0f);

      m_context.UpdateGlobalTransform(entity1);

      auto newBounds = m_context.AddComponent<BoundingManager>(entity1);
      newBounds.SetCenter(newTransform.GetPosition());
      newBounds.SetExtents(newTransform.GetScale());
    }

    EntityID entity2 = m_context.AddEntity(m_staticGroup);
    {
      Transform newTransform = m_context.AddComponent<TransformManager>(entity2);
      newTransform.GetPosition() = vec3(1.5f, 0.0f, 0.0f);
      newTransform.GetScale() = vec3(0.5f, 0.25f, 0.25f);

      m_context.SetParent(entity2, entity1);
      m_context.UpdateGlobalTransform(entity2);

      auto newBounds = m_context.AddComponent<BoundingManager>(entity2);
      newBounds.SetCenter(newTransform.GetGlobalPosition());
      newBounds.SetExtents(newTransform.GetGlobalScale());
    }

    EntityID entity3 = m_context.AddEntity(m_staticGroup);
    {
      Transform newTransform = m_context.AddComponent<TransformManager>(entity3);
      newTransform.GetPosition() = vec3(1.5f, 0.0f, 0.0f);
      newTransform.GetScale() = vec3(0.5f, 2.0f, 1.0f);

      m_context.SetParent(entity3, entity2);
      m_context.UpdateGlobalTransform(entity3);

      auto newBounds = m_context.AddComponent<BoundingManager>(entity3);
      newBounds.SetCenter(newTransform.GetGlobalPosition());
      newBounds.SetExtents(newTransform.GetGlobalScale());
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
    switch (key)
    {
    case '1': 
      m_freeCameraMode = !m_freeCameraMode;
      if (m_freeCameraMode)
      {
        m_fcSavedModelView = m_modelView;

        // Get frustum points
        vec4 winCoords[8] =
        { {-1.0, -1.0, -1.0, 1.0},
          {-1.0,  1.0, -1.0, 1.0},
          { 1.0,  1.0, -1.0, 1.0},
          { 1.0, -1.0, -1.0, 1.0},

          {-1.0, -1.0,  1.0, 1.0},
          {-1.0,  1.0,  1.0, 1.0},
          { 1.0,  1.0,  1.0, 1.0},
          { 1.0, -1.0,  1.0, 1.0},
        };

        //Get the frustum coordinates 
        // (need 4 coordinates as some perspective matrices result in negative w)
        mat4 inverseProj = glm::inverse(m_projection * m_modelView);
        for (uint i = 0; i < 8; i++)
        {
          m_fcFrustumPoints[i] = inverseProj * winCoords[i];
        }
      }
      break;
    }
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

void DrawBox(const mat4& i_transform)
{
  vec4 pos[8] =
  {
    i_transform * vec4(-1.0f, -1.0f, -1.0f, 1.0f), // 0
    i_transform * vec4(-1.0f, -1.0f, +1.0f, 1.0f), // 1
    i_transform * vec4(+1.0f, -1.0f, +1.0f, 1.0f), // 2
    i_transform * vec4(+1.0f, -1.0f, -1.0f, 1.0f), // 3
    i_transform * vec4(-1.0f, +1.0f, -1.0f, 1.0f), // 4
    i_transform * vec4(-1.0f, +1.0f, +1.0f, 1.0f), // 5
    i_transform * vec4(+1.0f, +1.0f, +1.0f, 1.0f), // 6
    i_transform * vec4(+1.0f, +1.0f, -1.0f, 1.0f)  // 7
  };

  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex4fv(value_ptr(pos[0]));
  glVertex4fv(value_ptr(pos[1]));
  glVertex4fv(value_ptr(pos[2]));
  glVertex4fv(value_ptr(pos[3]));

  glVertex4fv(value_ptr(pos[4]));
  glVertex4fv(value_ptr(pos[5]));
  glVertex4fv(value_ptr(pos[6]));
  glVertex4fv(value_ptr(pos[7]));

  glColor3f(1.0f, 1.0f, 0.0f);
  glVertex4fv(value_ptr(pos[0]));
  glVertex4fv(value_ptr(pos[1]));
  glVertex4fv(value_ptr(pos[5]));
  glVertex4fv(value_ptr(pos[4]));

  glVertex4fv(value_ptr(pos[2]));
  glVertex4fv(value_ptr(pos[3]));
  glVertex4fv(value_ptr(pos[7]));
  glVertex4fv(value_ptr(pos[6]));

  glColor3f(0.0f, 1.0f, 1.0f);
  glVertex4fv(value_ptr(pos[0]));
  glVertex4fv(value_ptr(pos[4]));
  glVertex4fv(value_ptr(pos[7]));
  glVertex4fv(value_ptr(pos[3]));

  glVertex4fv(value_ptr(pos[1]));
  glVertex4fv(value_ptr(pos[5]));
  glVertex4fv(value_ptr(pos[6]));
  glVertex4fv(value_ptr(pos[2]));
}

void DrawBox(const mat4x3& i_transform)
{
  vec3 pos[8] =
  {
    i_transform * vec4(-1.0f, -1.0f, -1.0f, 1.0f), // 0
    i_transform * vec4(-1.0f, -1.0f, +1.0f, 1.0f), // 1
    i_transform * vec4(+1.0f, -1.0f, +1.0f, 1.0f), // 2
    i_transform * vec4(+1.0f, -1.0f, -1.0f, 1.0f), // 3
    i_transform * vec4(-1.0f, +1.0f, -1.0f, 1.0f), // 4
    i_transform * vec4(-1.0f, +1.0f, +1.0f, 1.0f), // 5
    i_transform * vec4(+1.0f, +1.0f, +1.0f, 1.0f), // 6
    i_transform * vec4(+1.0f, +1.0f, -1.0f, 1.0f)  // 7
  };

  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3fv(value_ptr(pos[0]));
  glVertex3fv(value_ptr(pos[1]));
  glVertex3fv(value_ptr(pos[2]));
  glVertex3fv(value_ptr(pos[3]));

  glVertex3fv(value_ptr(pos[4]));
  glVertex3fv(value_ptr(pos[5]));
  glVertex3fv(value_ptr(pos[6]));
  glVertex3fv(value_ptr(pos[7]));

  glColor3f(1.0f, 1.0f, 0.0f);
  glVertex3fv(value_ptr(pos[0]));
  glVertex3fv(value_ptr(pos[1]));
  glVertex3fv(value_ptr(pos[5]));
  glVertex3fv(value_ptr(pos[4]));

  glVertex3fv(value_ptr(pos[2]));
  glVertex3fv(value_ptr(pos[3]));
  glVertex3fv(value_ptr(pos[7]));
  glVertex3fv(value_ptr(pos[6]));

  glColor3f(0.0f, 1.0f, 1.0f);
  glVertex3fv(value_ptr(pos[0]));
  glVertex3fv(value_ptr(pos[4]));
  glVertex3fv(value_ptr(pos[7]));
  glVertex3fv(value_ptr(pos[3]));

  glVertex3fv(value_ptr(pos[1]));
  glVertex3fv(value_ptr(pos[5]));
  glVertex3fv(value_ptr(pos[6]));
  glVertex3fv(value_ptr(pos[2]));
}

void App::drawFrame()
{
  /*
  // Update transform systems
  for (auto& v : Iter<TransformManager>(m_context))
  {
    vec3& pos = v.GetPosition();
    pos.y = cosf(pos.x + time) + sinf(pos.z + time);

    vec3& scale = v.GetScale();
    scale = vec3(fabsf(pos.y) * 0.12f, 0.25f, 0.25f);
    
    quat& rot = v.GetRotation();
    rot = glm::angleAxis(time * 0.9f, vec3(0.0f, 1.0f, 0.0f));
  }
  */

  for (auto& v : IterID<TransformManager>(m_context))
  {
    //vec3& pos = v.GetPosition();
    //pos.y = cosf(pos.x + time) + sinf(pos.z + time);

    //vec3& scale = v.GetScale();
    //scale = vec3(fabsf(pos.y) * 0.12f, 0.25f, 0.25f);

    quat& rot = v.GetRotation();
    rot = glm::angleAxis(time * 0.9f, vec3(0.0f, 1.0f, 0.0f));

    m_context.UpdateGlobalTransform(v.GetEntityID());
  }


  m_projection = perspectiveMatrixX(1.5f, width, height, 0.1f, 4000);
  //mat4 modelview = scale(1.0f, 1.0f, -1.0f) * rotateXY(-wx, -wy) * translate(-camPos) * rotateX(PI * 0.5f);
  m_modelView = rotateXY(-wx, -wy) * translate(-camPos);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(value_ptr(m_projection));

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(value_ptr(m_modelView));

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

  vec4 cullPlanes[6];
  getProjectionPlanes(m_projection, cullPlanes);
  planeInvTransform(m_freeCameraMode ? m_fcSavedModelView : m_modelView, &cullPlanes[0], 6);
  planeNormalize(&cullPlanes[0], 6);
  
  // Boxes
  glBegin(GL_QUADS);
  for (auto& v : IterID<TransformManager>(m_context))
  {
    //DrawBox(v.GetPosition(), 0.25f);
    EntityID id = v.GetEntityID();
    
    //auto bound = m_context.GetComponent<BoundingManager>(id);
    //if (testAABBFrustumPlanes(cullPlanes, bound.GetCenter(), bound.GetExtents()))
    {
      //DrawBox(v.CalculateModelWorld());
      DrawBox(ApplyScale(v.GetGlobalTransform(), v.GetGlobalScale()));
    }

    mat4 addMatrix = CalculateTransform4x4(v.GetPosition(), v.GetRotation(), v.GetScale());
    EntityID parentID = v.GetParent();
    while (parentID != EntityID_None)
    {
      Transform parent = m_context.GetComponent<TransformManager>(parentID);
      parentID = parent.GetParent();

      addMatrix = CalculateTransform4x4(parent.GetPosition(), parent.GetRotation(), parent.GetScale()) * addMatrix;
    }
    DrawBox(addMatrix);
  }
  glEnd();


  if (m_freeCameraMode)
  {
    renderer->reset();
    renderer->apply();

    // Render frustum
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    //Front
    glVertex4fv(value_ptr(m_fcFrustumPoints[0]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[1]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[1]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[2]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[2]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[3]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[3]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[0]));

    //Back
    glVertex4fv(value_ptr(m_fcFrustumPoints[4]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[5]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[5]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[6]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[6]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[7]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[7]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[4]));

    //Sides
    glVertex4fv(value_ptr(m_fcFrustumPoints[0]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[4]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[3]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[7]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[1]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[5]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[2]));
    glVertex4fv(value_ptr(m_fcFrustumPoints[6]));
    glEnd();
  }

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
