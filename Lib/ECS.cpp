#include "ECS.h"
#include <algorithm>

// DT_TODO class checks + white box testing

namespace
{
  inline bool DeletedSorter(EntitySubID a, EntitySubID b)
  {
    return (a > b);
  }
}

EntitySubID EntityGroup::AddEntity()
{
  // First check if there is a entity id that can be re-used
  if (m_deletedEntities.size() > 0)
  {
    EntitySubID retID = m_deletedEntities.back();
    m_deletedEntities.pop_back();
    return retID;
  }

  // Check if the array sizes need to grow
  AT_ASSERT(m_entityCount < UINT16_MAX);
  if ((m_entityCount & 0x3F) == 0)
  {
    for (ComponentManager* c : m_managers)
    {
      c->m_prevSum.push_back(c->m_componentCount);
      c->m_bitData.push_back(0);
    }

    for (FlagManager* f : m_flagManagers)
    {
      f->m_bitData.push_back(0);
    }
  }

  EntitySubID retID = (EntitySubID)m_entityCount;
  m_entityCount++;
  return retID;
}

void EntityGroup::RemoveEntity(GroupID i_groupID, EntitySubID i_entitySubID)
{
  AT_ASSERT(IsValid(i_entitySubID));

  uint64_t mask = uint64_t(1) << ((uint16_t)i_entitySubID & 0x3F);
  uint64_t invMask = ~mask;
  uint64_t preBitsMask = mask - 1;
  uint16_t index = (uint16_t)i_entitySubID >> 6;

  // Loop for all component managers and remove
  EntityID entityID{ i_groupID, i_entitySubID };
  for (ComponentManager* c : m_managers)
  {
    uint64_t testBits = c->m_bitData[index];
    uint64_t newBits = testBits & invMask;
    if (testBits != newBits)
    {
      // Debug check that there are no active accessors to the data
      c->m_accessCheck.CheckLock();

      uint16_t offset = c->m_prevSum[index] + PopCount64(testBits & preBitsMask);
      c->OnComponentRemove(entityID, offset);
      
      c->m_bitData[index] = newBits;

      // Update the counts
      c->m_componentCount--;
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
  auto insertPos = std::lower_bound(m_deletedEntities.begin(), m_deletedEntities.end(), i_entitySubID, DeletedSorter);

  // Do not insert if already in the list (catch double deletes)
  if (insertPos == m_deletedEntities.end() ||
      *insertPos != i_entitySubID)
  {
    m_deletedEntities.insert(insertPos, i_entitySubID);
  }
}

bool EntityGroup::IsDeleted(EntitySubID i_entitySubID) const
{
  return std::binary_search(m_deletedEntities.begin(), m_deletedEntities.end(), i_entitySubID, DeletedSorter);
}

void EntityGroup::ReserveEntities(uint16_t i_count)
{
  // Get how many entities to reserve (in multiples of 64)
  uint32_t existingReserve = ((uint32_t)m_entityCount + 63) >> 6;  
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

uint16_t ComponentManager::SetBit(EntitySubID i_entitySubID)
{
  AT_ASSERT(!HasComponent(i_entitySubID));

  uint64_t mask = uint64_t(1) << ((uint16_t)i_entitySubID & 0x3F);
  uint64_t preBitsMask = mask - 1;
  uint16_t index = (uint16_t)i_entitySubID >> 6;

  uint64_t testBits = m_bitData[index];
  uint64_t newBits = testBits | mask;
  uint16_t offset = m_prevSum[index] + PopCount64(testBits & preBitsMask);

  m_bitData[index] = newBits;

  // Update the counts
  m_componentCount++;
  for (uint32_t i = uint32_t(index) + 1; i < m_prevSum.size(); i++)
  {
    m_prevSum[i]++;
  }
  return offset;
}

uint16_t ComponentManager::ClearBit(EntitySubID i_entitySubID)
{
  AT_ASSERT(HasComponent(i_entitySubID) && m_componentCount > 0);

  uint64_t mask = uint64_t(1) << ((uint16_t)i_entitySubID & 0x3F);
  uint64_t preBitsMask = mask - 1;
  uint16_t index = (uint16_t)i_entitySubID >> 6;

  uint64_t testBits = m_bitData[index];
  uint64_t newBits = testBits & ~mask;
  uint16_t offset = m_prevSum[index] + PopCount64(testBits & preBitsMask);

  m_bitData[index] = newBits;

  // Update the counts
  m_componentCount--;
  for (uint32_t i = uint32_t(index) + 1; i < m_prevSum.size(); i++)
  {
    m_prevSum[i]--;
  }
  return offset;
}

