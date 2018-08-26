#include "GameGroup.h"
#include "Components/Transform.h"
#include "Components/Bounds.h"

GameGroup::GameGroup()
{
  m_transforms = std::make_unique<TransformManager>();
  m_bounds = std::make_unique<BoundingManager>();
  //m_bounds = new BoundingManagerSIMD();
  m_flagTest = std::make_unique<FlagManager>();

  AddManager(&*m_transforms);
  AddManager(&*m_bounds);
  AddManager(&*m_flagTest);
}

