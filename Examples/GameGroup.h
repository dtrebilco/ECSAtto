#pragma once

#include <ECS.h>
#include <memory>

class Transforms;
class GlobalTransforms;
class Bounds;
class GlobalBounds;

class FlagTest : public FlagManager {};

class GameGroup : public EntityGroup
{
public:

  GameGroup();
  ~GameGroup();

  std::unique_ptr<Transforms> m_transforms;
  std::unique_ptr<GlobalTransforms> m_globalTransforms;

  std::unique_ptr<Bounds> m_bounds;
  std::unique_ptr<GlobalBounds> m_globalBounds;
  std::unique_ptr<FlagTest> m_flagTest;
};

template<> inline Transforms& GetManager<Transforms>(GameGroup& i_group) { return *i_group.m_transforms; }
template<> inline GlobalTransforms& GetManager<GlobalTransforms>(GameGroup& i_group) { return *i_group.m_globalTransforms; }
template<> inline Bounds& GetManager<Bounds>(GameGroup& i_group) { return *i_group.m_bounds; }
template<> inline GlobalBounds& GetManager<GlobalBounds>(GameGroup& i_group) { return *i_group.m_globalBounds; }
template<> inline FlagTest& GetManager<FlagTest>(GameGroup& i_group) { return *i_group.m_flagTest; }

