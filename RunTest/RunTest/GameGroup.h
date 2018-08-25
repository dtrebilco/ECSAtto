#include "../../Lib/ECS.h"

class TransformManager;
class BoundingManager;
class BoundingManagerSIMD;

class GameGroup : public EntityGroup
{
public:

  GameGroup();
  ~GameGroup();

  TransformManager* m_transforms = nullptr;
  BoundingManager* m_bounds = nullptr;
  //BoundingManagerSIMD* m_bounds = nullptr;
  FlagManager* m_flagTest = nullptr;
};

template<>
inline TransformManager* GameGroup::* GetManager<TransformManager, GameGroup>() { return &GameGroup::m_transforms; }


