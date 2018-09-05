#pragma once

#include "../../Lib/ECS.h"
#include <memory>

class TransformManager;
class GlobalTransformManager;
class BoundingManager;
class BoundingManagerSIMD;

class GameGroup : public EntityGroup
{
public:

  GameGroup();
  ~GameGroup();

  std::unique_ptr<TransformManager> m_transforms;
  std::unique_ptr<GlobalTransformManager> m_globalTransforms;

  std::unique_ptr<BoundingManager> m_bounds;
  //BoundingManagerSIMD* m_bounds = nullptr;
  std::unique_ptr<FlagManager> m_flagTest;
};

template<>
inline TransformManager* GetManager<TransformManager, GameGroup>(GameGroup* i_group) { return &*i_group->m_transforms; }

template<>
inline GlobalTransformManager* GetManager<GlobalTransformManager, GameGroup>(GameGroup* i_group) { return &*i_group->m_globalTransforms; }

template<>
inline BoundingManager* GetManager<BoundingManager, GameGroup>(GameGroup* i_group) { return &*i_group->m_bounds; }

template<>
inline FlagManager* GetManager<FlagManager, GameGroup>(GameGroup* i_group) { return &*i_group->m_flagTest; }

