#pragma once

#include <ECS.h>
#include <memory>

class TransformManager;
class GlobalTransformManager;
class BoundingManager;
class GlobalBoundingManager;

class FlagTest : public FlagManager {};

class GameGroup : public EntityGroup
{
public:

  GameGroup();
  ~GameGroup();

  std::unique_ptr<TransformManager> m_transforms;
  std::unique_ptr<GlobalTransformManager> m_globalTransforms;

  std::unique_ptr<BoundingManager> m_bounds;
  std::unique_ptr<GlobalBoundingManager> m_globalBounds;
  std::unique_ptr<FlagTest> m_flagTest;
};

template<> inline TransformManager& GetManager<TransformManager>(GameGroup& i_group) { return *i_group.m_transforms; }
template<> inline GlobalTransformManager& GetManager<GlobalTransformManager>(GameGroup& i_group) { return *i_group.m_globalTransforms; }
template<> inline BoundingManager& GetManager<BoundingManager>(GameGroup& i_group) { return *i_group.m_bounds; }
template<> inline GlobalBoundingManager& GetManager<GlobalBoundingManager>(GameGroup& i_group) { return *i_group.m_globalBounds; }
template<> inline FlagTest& GetManager<FlagTest>(GameGroup& i_group) { return *i_group.m_flagTest; }

