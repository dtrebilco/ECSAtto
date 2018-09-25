#include "ECS.h"
// DT_TODO class checks + white box testing

EntitySubID EntityGroup::AddEntity()
{
  // First check if there is a entity id that can be re-used
  if (m_deletedEntities.size() > 0)
  {
    EntitySubID retID = m_deletedEntities.back();

#ifdef ECS_DEBUG
    // Assert that there is no components/flags on the entity
    for (ComponentManager* c : m_managers)
    {
      AT_ASSERT(c->HasComponent(retID) == false);
    }
    for (FlagManager* f : m_flagManagers)
    {
      AT_ASSERT(f->HasComponent(retID) == false);
    }
#endif // ECS_DEBUG

    m_deletedEntities.pop_back();
    return retID;
  }

  // Check if the array sizes need to grow
  AT_ASSERT(m_entityMax < UINT16_MAX);
  if ((m_entityMax & 0x3F) == 0)
  {
    for (ComponentManager* c : m_managers)
    {
      if (m_entityMax == 0)
      {
        c->m_prevSum.push_back(0);
      }
      else
      {
        c->m_prevSum.push_back(c->m_prevSum.back() + PopCount64(c->m_bitData.back()));
      }
      c->m_bitData.push_back(0);
    }

    for (FlagManager* f : m_flagManagers)
    {
      f->m_bitData.push_back(0);
    }
  }

  EntitySubID retID = (EntitySubID)m_entityMax;
  m_entityMax++;
  return retID;
}

void EntityGroup::RemoveEntity(EntitySubID i_entity)
{
  AT_ASSERT(IsValid(i_entity));

  uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
  uint64_t invMask = ~mask;
  uint64_t preBitsMask = mask - 1;
  uint16_t index = (uint16_t)i_entity >> 6;

  // Loop for all component managers and remove
  for (ComponentManager* c : m_managers)
  {
    uint64_t testBits = c->m_bitData[index];
    uint64_t newBits = testBits & invMask;
    if (testBits != newBits)
    {
      // Debug check that there are no active accessors to the data
      c->m_accessCheck.CheckLock();

      uint16_t offset = c->m_prevSum[index] + PopCount64(testBits & preBitsMask);
      c->OnComponentRemove(offset);
      
      c->m_bitData[index] = newBits;

      // Update the counts
      for (uint32_t i = uint32_t(index) + 1; i < c->m_prevSum.size(); i++)
      {
        c->m_prevSum[i]--;
      }
    }
  }

  // Loop for all flag managers and remove the bit
  for (FlagManager* f : m_flagManagers)
  {
    uint64_t testBits = f->m_bitData[index];
    uint64_t newBits = testBits & invMask;
    if (testBits != newBits)
    {
      f->m_bitData[index] = newBits;
    }
  }

  // Add to the array of deleted entities
  // Insert in reverse order so that they are pulled out sequentially
  auto insertPos = std::lower_bound(m_deletedEntities.begin(), m_deletedEntities.end(), i_entity,
    [](EntitySubID a, EntitySubID b)
    {
      return (a > b);
    });

  // Do not insert if already in the list (catch double deletes)
  if (insertPos == m_deletedEntities.end() ||
      *insertPos != i_entity)
  {
    m_deletedEntities.insert(insertPos, i_entity);
  }
}

void EntityGroup::ReserveEntities(uint16_t i_count)
{
  // Get how many entities to reserve (in multiples of 64)
  uint32_t existingReserve = ((uint32_t)m_entityMax + 63) >> 6;  
  uint32_t reserveCount = ((uint32_t)i_count + 63) >> 6;
  if (reserveCount <= existingReserve)
  {
    return;
  }

  // Reserve all the arrays
  for (ComponentManager* c : m_managers)
  {
    c->m_bitData.reserve(reserveCount);
    c->m_prevSum.reserve(reserveCount);
  }

  for (FlagManager* f : m_flagManagers)
  {
    f->m_bitData.reserve(reserveCount);
  }
}

uint16_t ComponentManager::SetBit(EntitySubID i_entity)
{
  AT_ASSERT(!HasComponent(i_entity));

  uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
  uint64_t preBitsMask = mask - 1;
  uint16_t index = (uint16_t)i_entity >> 6;

  uint64_t testBits = m_bitData[index];
  uint64_t newBits = testBits | mask;
  uint16_t offset = m_prevSum[index] + PopCount64(testBits & preBitsMask);

  m_bitData[index] = newBits;

  // Update the counts
  for (uint32_t i = uint32_t(index) + 1; i < m_prevSum.size(); i++)
  {
    m_prevSum[i]++;
  }
  return offset;
}

uint16_t ComponentManager::ClearBit(EntitySubID i_entity)
{
  AT_ASSERT(HasComponent(i_entity));

  uint64_t mask = uint64_t(1) << ((uint16_t)i_entity & 0x3F);
  uint64_t preBitsMask = mask - 1;
  uint16_t index = (uint16_t)i_entity >> 6;

  uint64_t testBits = m_bitData[index];
  uint64_t newBits = testBits & ~mask;
  uint16_t offset = m_prevSum[index] + PopCount64(testBits & preBitsMask);

  m_bitData[index] = newBits;

  // Update the counts
  for (uint32_t i = uint32_t(index) + 1; i < m_prevSum.size(); i++)
  {
    m_prevSum[i]--;
  }
  return offset;
}

