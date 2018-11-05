#include "GameGroup.h"
#include "Components/Transforms.h"
#include "Components/Bounds.h"

GameGroup::GameGroup()
{
  m_transforms = std::make_unique<Transforms>();
  m_worldTransforms = std::make_unique<WorldTransforms>();

  m_bounds = std::make_unique<Bounds>();
  m_worldBounds = std::make_unique<WorldBounds>();

  AddManager(&*m_transforms);
  AddManager(&*m_worldTransforms);

  AddManager(&*m_bounds);
  AddManager(&*m_worldBounds);
}

GameGroup::~GameGroup()
{
}

