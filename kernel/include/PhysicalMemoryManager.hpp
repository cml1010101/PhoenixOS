#ifndef PHYSICALMEMORYMANAGER_HPP
#define PHYSICALMEMORYMANAGER_HPP
#include <PhoenixOS.hpp>
#include <efi.h>
#include <efilib.h>
class PhysicalMemoryManager
{
private:
    uint64_t* bitmap;
    uint64_t* bitmapPhys;
    size_t numTotalPages;
    inline bool isUsable(uint32_t memoryType)
    {
        switch (memoryType)
        {
        case EfiReservedMemoryType:
        case EfiLoaderCode:
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiUnusableMemory:
        case EfiACPIMemoryNVS:
        case EfiACPIReclaimMemory:
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        case EfiPalCode:
        case EfiMaxMemoryType:
            return false;
        default:
            return true;
        };
    }
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
    inline void setPages(uint64_t idx, uint64_t bit, size_t pages)
    {
        for (size_t i = 0; i < pages; i++)
        {
            setPage(idx + i, bit);
        }
    }
    inline uint64_t findFirstFreePages(size_t count)
    {
        for (size_t i = 0; i < numTotalPages; i++)
        {
            if (getPage(i) == 0)
            {
                bool flag = false;
                for (size_t j = 1; j < count; j++)
                {
                    if (getPage(i + j) != 0)
                    {
                        flag = true;
                        break;
                    }
                }
                if (!flag) return i;
            }
        }
        return -1;
    }
    inline uint64_t allocatePage()
    {
        uint64_t idx = findFirstFreePages(1);
        if (idx == (size_t)-1) return -1;
        setPage(idx, 1);
        return idx;
    }
    inline uint64_t allocatePages(size_t num)
    {
        uint64_t idx = findFirstFreePages(num);
        if (idx == (size_t)-1) return -1;
        setPages(idx, 1, num);
        return idx;
    }
    void setupPaging();
    void printStatus(Logger* logger);
};
#endif