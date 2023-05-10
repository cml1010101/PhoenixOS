#ifndef PHYSICALMEMORYMANAGER_HPP
#define PHYSICALMEMORYMANAGER_HPP
#include <PhoenixOS.hpp>
class PhysicalMemoryManager
{
private:
    uint64_t* bitmap;
    size_t numTotalPages;
public:
    static PhysicalMemoryManager instance;
    PhysicalMemoryManager() = default;
    PhysicalMemoryManager(void* memoryMap, size_t memoryMapSize, size_t descriptorSize);
    inline uint64_t getPage(uint64_t idx)
    {
        return bitmap[idx / 64] & (1 << (idx % 64));
    }
    inline void setPage(uint64_t idx, uint64_t bit)
    {
        bitmap[idx / 64] = (bitmap[idx / 64] & ~(1 << (idx % 64))) | (bit << (idx % 64));
    }
    inline uint64_t findFirstFreePage()
    {
        for (size_t i = 0; i < numTotalPages; i++)
        {
            if (getPage(i) == 0) return i;
        }
        return -1;
    }
    inline uint64_t allocatePage()
    {
        uint64_t idx = findFirstFreePage();
        if (idx == -1) return -1;
        setPage(idx, 1);
        return idx;
    }
};
#endif