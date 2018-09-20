#include "GameGroup.h"
#include "Components/Transform.h"
#include "Components/Bounds.h"

GameGroup::GameGroup()
{
  m_transforms = std::make_unique<TransformManager>();
  m_globalTransforms = std::make_unique<GlobalTransformManager>();

  m_bounds = std::make_unique<BoundingManager>();
  m_globalBounds = std::make_unique<GlobalBoundingManager>();

  m_flagTest = std::make_unique<FlagTest>();

  AddManager(&*m_transforms);
  AddManager(&*m_globalTransforms);
  AddManager(&*m_bounds);
  AddManager(&*m_globalBounds);
  AddManager(&*m_flagTest);
}

GameGroup::~GameGroup()
{
}

