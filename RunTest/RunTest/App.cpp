//======================================================================
// ECS Runtime test
//      By Damian Trebilco
//======================================================================

#include "App.h"
#include <vector>
#include <algorithm>

#include <ECSIter.h>

#include "../Examples/TransformUtils.h"
#include "../Examples/Components/Bounds.h"
#include "../Examples/Components/Transforms.h"

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
  m_context.ReserveGroups(2);
  m_staticGroup = m_context.AddEntityGroup();
  m_dynamicGroup = m_context.AddEntityGroup();

  m_context.ReserveEntities(m_staticGroup, 10000);
  for (uint32_t y = 0; y < 100; y++)
  {
    for (uint32_t x = 0; x < 100; x++)
    {
      EntityID newEntity = m_context.AddEntity(m_staticGroup);
      m_context.AddComponent<WorldTransforms>(newEntity);
      m_context.AddComponent<WorldBounds>(newEntity);

      auto newTransform = m_context.AddComponent<Transforms>(newEntity);
      newTransform.GetPosition() = vec3((float)x + 0.5f, 0.5f, (float)y + 0.5f);
      newTransform.GetScale() = vec3(0.25f);

      auto newBounds = m_context.AddComponent<Bounds>(newEntity);
      newBounds.SetCenter(vec3(0.0f));
      newBounds.SetExtents(vec3(1.0f));

      // Set initial position and scale adjustments
      {
        vec3& pos = newTransform.GetPosition();
        pos.y = cosf(pos.x) + sinf(pos.z);

        vec3& scale = newTransform.GetScale();
        scale = vec3(fabsf(pos.y) * 0.12f, 0.25f, 0.25f);
      }

      UpdateWorldData(m_context, newEntity);
    }
  }

  vec3 center(10.0f, 0.0f, 5.5f);
  vec3 extents(2.0f, 2.0f, 3.5f);

  {
    EntityID entity1 = m_context.AddEntity(m_dynamicGroup);
    {
      auto newTransform = m_context.AddComponent<Transforms>(entity1);
      auto newWorldTransform = m_context.AddComponent<WorldTransforms>(entity1);

      newTransform.GetPosition() = vec3(2.0f, 3.0f, 2.0f);
      newTransform.GetScale() = vec3(1.0f, 0.5f, 1.0f);

      auto newBounds = m_context.AddComponent<Bounds>(entity1);
      auto newWorldBounds = m_context.AddComponent<WorldBounds>(entity1);
      newBounds.SetCenter(center);
      newBounds.SetExtents(extents);
    }

    EntityID entity2 = m_context.AddEntity(m_dynamicGroup);
    {
      auto newTransform = m_context.AddComponent<Transforms>(entity2);
      auto newWorldTransform = m_context.AddComponent<WorldTransforms>(entity2);

      newTransform.GetPosition() = vec3(1.5f, 0.0f, 0.0f);
      newTransform.GetScale() = vec3(0.5f, 0.25f, 0.25f);

      auto newBounds = m_context.AddComponent<Bounds>(entity2);
      auto newWorldBounds = m_context.AddComponent<WorldBounds>(entity2);
      newBounds.SetCenter(center);
      newBounds.SetExtents(extents);

      SetParent(m_context, entity2, entity1);
    }

    EntityID entity3 = m_context.AddEntity(m_dynamicGroup);
    {
      auto newTransform = m_context.AddComponent<Transforms>(entity3);
      auto newWorldTransform = m_context.AddComponent<WorldTransforms>(entity3);

      newTransform.GetPosition() = vec3(1.5f, 0.0f, 0.0f);
      newTransform.GetScale() = vec3(0.5f, 2.0f, 1.0f);

      auto newBounds = m_context.AddComponent<Bounds>(entity3);
      auto newWorldBounds = m_context.AddComponent<WorldBounds>(entity3);
      newBounds.SetCenter(center); 
      newBounds.SetExtents(extents);

      SetParent(m_context, entity3, entity2);
    }

    UpdateWorldData(m_context, entity1);

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

void DrawWireBox(const vec3& i_center, const vec3& i_extents)
{
  vec3 pos[8] =
  {
    i_center + vec3(i_extents.x,   i_extents.y,  i_extents.z), // 0
    i_center + vec3(i_extents.x,  -i_extents.y,  i_extents.z), // 1
    i_center + vec3(i_extents.x,   i_extents.y, -i_extents.z), // 2
    i_center + vec3(i_extents.x,  -i_extents.y, -i_extents.z), // 3
    i_center + vec3(-i_extents.x,  i_extents.y,  i_extents.z), // 4
    i_center + vec3(-i_extents.x, -i_extents.y,  i_extents.z), // 5
    i_center + vec3(-i_extents.x,  i_extents.y, -i_extents.z), // 6
    i_center + vec3(-i_extents.x, -i_extents.y, -i_extents.z)  // 7
  };

  glVertex3fv(value_ptr(pos[0]));
  glVertex3fv(value_ptr(pos[1]));
  
  glVertex3fv(value_ptr(pos[0]));
  glVertex3fv(value_ptr(pos[2]));

  glVertex3fv(value_ptr(pos[2]));
  glVertex3fv(value_ptr(pos[3]));

  glVertex3fv(value_ptr(pos[1]));
  glVertex3fv(value_ptr(pos[3]));


  glVertex3fv(value_ptr(pos[4]));
  glVertex3fv(value_ptr(pos[5]));

  glVertex3fv(value_ptr(pos[4]));
  glVertex3fv(value_ptr(pos[6]));

  glVertex3fv(value_ptr(pos[6]));
  glVertex3fv(value_ptr(pos[7]));

  glVertex3fv(value_ptr(pos[5]));
  glVertex3fv(value_ptr(pos[7]));


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
  for (auto& v : IterEntity<Transforms>(m_context, m_staticGroup))
  {
    vec3& pos = v.GetPosition();
    pos.y = cosf(pos.x + time) + sinf(pos.z + time);

    vec3& scale = v.GetScale();
    scale = vec3(fabsf(pos.y) * 0.12f, 0.25f, 0.25f);
    
    quat& rot = v.GetRotation();
    rot = glm::angleAxis(time * 0.9f, vec3(0.0f, 1.0f, 0.0f));

    UpdateWorldData(m_context, v.GetEntityID());
  }
  */
  for (auto& v : IterEntity<Transforms>(m_context, m_dynamicGroup))
  {
    quat& rot = v.GetRotation();
    rot = glm::angleAxis(time * 0.9f, vec3(0.0f, 1.0f, 0.0f));
  }
  UpdateWorldData(m_context, EntityID{ m_dynamicGroup, (EntitySubID)0 });


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

  for (auto& v : IterEntity<WorldTransforms, WorldBounds>(m_context, m_staticGroup))
  {
    EntityID id = v.GetEntityID();

    auto bound = m_context.GetComponent<WorldBounds>(id);
    if (testAABBFrustumPlanes(cullPlanes, bound.GetCenter(), bound.GetExtents()))
    {
      DrawBox(ApplyScale(v.GetWorldTransform(), v.GetWorldScale()));
    }
  }

  for (auto& v : IterEntity<WorldTransforms>(m_context, m_dynamicGroup))
  {
    //DrawBox(v.GetPosition(), 0.25f);
    EntityID id = v.GetEntityID();
    
    auto bound = m_context.GetComponent<Bounds>(id);
    auto worldBound = m_context.GetComponent<WorldBounds>(id);
    if (testAABBFrustumPlanes(cullPlanes, worldBound.GetCenter(), worldBound.GetExtents()))
    {
      // Apply offset to account for offset in bounding box (box is not centered)
      mat4 newScale(ApplyScale(v.GetWorldTransform(), v.GetWorldScale()));
      newScale = glm::translate(newScale, bound.GetCenter());
      newScale = glm::scale(newScale, bound.GetExtents());
      DrawBox(newScale);
    }
  }
  glEnd();


  glBegin(GL_LINES);
  for (auto& v : IterEntity<WorldTransforms>(m_context, m_dynamicGroup))
  {
    glColor3f(1.0f, 1.0f, 0.0f);
    EntityID id = v.GetEntityID();
    auto bound = m_context.GetComponent<WorldBounds>(id);
    DrawWireBox(bound.GetCenter(), bound.GetExtents());

    auto transform = m_context.GetComponent<Transforms>(id);
    auto worldTransform = m_context.GetComponent<WorldTransforms>(id);
    if (transform.GetParent() == EntityID_None)
    {
      glVertex3fv(value_ptr(vec3(0.0f)));
      glVertex3fv(value_ptr(worldTransform.GetWorldPosition()));

      glColor3f(1.0f, 1.0f, 1.0f);
      glVertex3fv(value_ptr(worldTransform.GetWorldPosition()));
      glVertex3fv(value_ptr(bound.GetCenter()));
    }
    else
    {
      auto parentWorldTransform = m_context.GetComponent<WorldTransforms>(transform.GetParent());
      glVertex3fv(value_ptr(parentWorldTransform.GetWorldPosition()));
      glVertex3fv(value_ptr(worldTransform.GetWorldPosition()));

      glColor3f(1.0f, 1.0f, 1.0f);
      glVertex3fv(value_ptr(worldTransform.GetWorldPosition()));
      glVertex3fv(value_ptr(bound.GetCenter()));
    }

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
