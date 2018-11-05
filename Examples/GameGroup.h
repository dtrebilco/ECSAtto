#pragma once

#include <ECS.h>
#include <memory>

class Transforms;
class WorldTransforms;
class Bounds;
class WorldBounds;

class FlagTest : public FlagManager {};

class GameGroup : public EntityGroup
{
public:

  GameGroup();
  ~GameGroup();

  std::unique_ptr<Transforms> m_transforms;
  std::unique_ptr<WorldTransforms> m_worldTransforms;

  std::unique_ptr<Bounds> m_bounds;
  std::unique_ptr<WorldBounds> m_worldBounds;
  std::unique_ptr<FlagTest> m_flagTest;
};

template<> inline Transforms& GetManager<Transforms>(GameGroup& i_group) { return *i_group.m_transforms; }
template<> inline WorldTransforms& GetManager<WorldTransforms>(GameGroup& i_group) { return *i_group.m_worldTransforms; }
template<> inline Bounds& GetManager<Bounds>(GameGroup& i_group) { return *i_group.m_bounds; }
template<> inline WorldBounds& GetManager<WorldBounds>(GameGroup& i_group) { return *i_group.m_worldBounds; }
template<> inline FlagTest& GetManager<FlagTest>(GameGroup& i_group) { return *i_group.m_flagTest; }

