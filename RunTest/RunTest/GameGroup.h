#pragma once

#include "../../Lib/ECS.h"

#include "Components/Transform.h"
#include "Components/Bounds.h"

class TransformManager;
class BoundingManager;
class BoundingManagerSIMD;

class GameGroup : public EntityGroup
{
public:

  GameGroup();
  ~GameGroup();

  TransformManager m_transforms;
  BoundingManager m_bounds;
  //BoundingManagerSIMD* m_bounds = nullptr;
  FlagManager m_flagTest;
};

template<>
inline TransformManager GameGroup::* GetManager<TransformManager, GameGroup>() { return &GameGroup::m_transforms; }


