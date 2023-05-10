#ifndef VIRTUALMEMORYMANAGER_HPP
#define VIRTUALMEMORYMANAGER_HPP
#include <PhoenixOS.hpp>
#define VMM_PRESENT (1 << 0)
#define VMM_READ_WRITE (1 << 1)
#define VMM_CACHE_DISABLE (1 << 4)
class VirtualMemoryManager
{
public:
    VirtualMemoryManager();
    void mapPage(uint64_t virt, uint64_t phys, uint64_t flags);
    uint64_t allocateAddress(uint64_t pages);
    void* allocate(uint64_t pages);
    uint64_t getPhysicalAddress();
    uint64_t getPhysicalAddress(void* virtualAddress);
    VirtualMemoryManager* clone();
    static VirtualMemoryManager* getCurrentVirtualMemoryManager();
    static VirtualMemoryManager* getKernelVirtualMemoryManager();
};
#endif