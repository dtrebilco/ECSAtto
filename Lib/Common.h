#pragma once

#include <cstdint>
#include <cassert>

#ifndef NDEBUG

#define AT_ASSERT(x) assert(x)
#include <atomic>
class DebugAccessCheck
{
public:
  inline ~DebugAccessCheck() { AT_ASSERT(m_counter == 0); }
  inline void AddLock()      { m_counter++; AT_ASSERT(m_counter > 0); }
  inline void ReleaseLock()  { m_counter--; AT_ASSERT(m_counter >= 0); }
  inline void CheckLock()    { AT_ASSERT(m_counter == 0); }
private:
  std::atomic_int32_t m_counter = 0;
};


#else

#define AT_ASSERT(x)

class DebugAccessCheck
{
public:
  inline void AddLock() {}
  inline void ReleaseLock() {}
  inline void CheckLock(){}
};

#endif


/// \brief Get the count of the number of bits set
/// Taken from https://en.wikipedia.org/wiki/Hamming_weight
inline uint16_t PopCount64(uint64_t x)
{
  const uint64_t m1 = 0x5555555555555555; //binary: 0101...
  const uint64_t m2 = 0x3333333333333333; //binary: 00110011..
  const uint64_t m4 = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
  const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

  x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
  x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
  x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
  return uint16_t((x * h01) >> 56);  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}

