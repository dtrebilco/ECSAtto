#include "GameGroup.h"
#include "Components/Transform.h"
#include "Components/Bounds.h"

GameGroup::GameGroup()
{
  m_transforms = new TransformManager();
  m_bounds = new BoundingManager();
  //m_bounds = new BoundingManagerSIMD();
  m_flagTest = new FlagManager();

  AddManager(m_transforms);
  AddManager(m_bounds);
  AddManager(m_flagTest);
}

GameGroup::~GameGroup()
{
  delete m_transforms;
  delete m_bounds;
  delete m_flagTest;
}