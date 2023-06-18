#ifndef VIRTUALMEMORYMANAGER_HPP
#define VIRTUALMEMORYMANAGER_HPP
#include <PhoenixOS.hpp>
#define VMM_PRESENT (1 << 0)
#define VMM_READ_WRITE (1 << 1)
#define VMM_USER (1 << 2)
#define VMM_CACHE_DISABLE (1 << 4)
#define VMM_ADDR 0x000FFFFFFFFFF000
class VirtualMemoryManager
{
private:
    uint64_t* pml4Virt;
    uint64_t* pml4Phys;
    struct VirtualAddressAllocation
    {
        uint64_t address;
        uint64_t size;
    };
    VirtualAddressAllocation* allocationTablePhys;
    VirtualAddressAllocation* allocationTableVirt;
    uint64_t findFree(uint64_t pages);
    uint64_t rsp = 0;
    static VirtualMemoryManager* kernelVirtualMemoryManager;
    bool useVirtualAddresses, remap;
    uint64_t magic = 0xDEADBEEFCAFEBEAD;
public:
    VirtualMemoryManager() = default;
    VirtualMemoryManager(bool kernel);
    void mapPage(uint64_t virt, uint64_t phys, uint64_t flags);
    inline void map(uint64_t virt, uint64_t phys, uint64_t flags, uint64_t pages)
    {
        if (pages == 0) Logger::getInstance()->panic("Cannot map 0 pages\n");
        Logger::getInstance()->log("Mapping %d pages from phys 0x%x(0x%x) to virt 0x%x(0x%x)\n", pages, phys, phys + (pages << 12),
            virt, virt + (pages << 12));
        for (size_t i = 0; i < pages; i++)
        {
            mapPage(virt + (i << 12), phys + (i << 12), flags);
        }
    }
    uint64_t allocateAddress(uint64_t pages, uint64_t minimumAddress = 0x0);
    void* allocate(uint64_t pages, uint64_t flags);
    inline uint64_t getPhysicalAddress()
    {
        return (uint64_t)pml4Phys;
    }
    inline uint64_t getRSP()
    {
        return rsp;
    }
    uint64_t getPhysicalAddress(void* virtualAddress);
    void addAddressAllocation(VirtualAddressAllocation allocation);
    VirtualMemoryManager* clone();
    static VirtualMemoryManager* getKernelVirtualMemoryManager();
    static void initialize();
    void printAllocationTable(Logger* logger);
    void reset();
    void checkMagic();
};
#endif