#include <cstdint>
#include <vector>

#include "Common.h"


class BitArray
{
public:

  inline void Add()
  {
    // Add another 64bits if necessary
    if ((m_size & 0x3F) == 0)
    {
      m_data.push_back(0);
    }
    m_size++;
  }

  inline void RemoveAt(uint32_t i_index)
  {
    AT_ASSERT(i_index < m_size);

    uint64_t mask = uint64_t(1) << (i_index & 0x3F);
    uint32_t dataIndex = i_index >> 6;
    
    // The bits to save
    uint64_t saveMask = (mask - 1);

    // Save the lower data bits
    uint64_t data = m_data[dataIndex];
    uint64_t saveData = data & saveMask;
    
    // Move the data down by one, and restore the saved data
    data >>= 1;
    m_data[dataIndex] = (data & ~saveMask) | saveData;

    // Loop for all above bits and move down
    for (uint32_t i = dataIndex + 1; i < (uint32_t)m_data.size(); i++)
    {
      m_data[i - 1] |= (m_data[i] & (uint64_t(1)) << 63);
      m_data[i] >>= 1;
    }

    m_size--;

    // Remove if fallen below a 64 bit count
    if ((m_size & 0x3F) == 0)
    {
      m_data.pop_back();
    }
  }

  inline bool Get(uint32_t i_index) const
  {
    uint64_t mask;
    const uint64_t& data = GetBitData(i_index, mask);
    return (data & mask) != 0;
  }
  
  inline void SetBit(uint32_t i_index)
  {
    uint64_t mask;
    uint64_t& data = GetBitData(i_index, mask);
    data |= mask;
  }

  inline void ClearBit(uint32_t i_index)
  {
    uint64_t mask;
    uint64_t& data = GetBitData(i_index, mask);
    data &= ~mask;
  }
  
  inline uint64_t& GetBitData(uint32_t i_index, uint64_t& o_mask)
  {
    AT_ASSERT(i_index < m_size);
    o_mask = uint64_t(1) << (i_index & 0x3F);
    return m_data[i_index >> 6];
  }

  inline const uint64_t& GetBitData(uint32_t i_index, uint64_t& o_mask) const
  {
    AT_ASSERT(i_index < m_size);
    o_mask = uint64_t(1) << (i_index & 0x3F);
    return m_data[i_index >> 6];
  }
  
  inline uint32_t GetSize() const
  {
    return m_size;
  }

  inline uint32_t GetCapacity() const
  {
    return (uint32_t)m_data.capacity() * 64;
  }

  inline void SetCapacity(uint32_t i_count)
  {
    if (i_count < GetCapacity())
    {
      return;
    }

    // Round up to multiple of 64
    m_data.reserve((i_count + 63) >> 6);
  }

private:

  uint32_t m_size;
  std::vector<uint64_t> m_data; //!< The array of bit data

};


