#include <XSDT.hpp>
#include <PhysicalMemoryManager.hpp>
#include <VirtualMemoryManager.hpp>
XSDT* XSDT::instance = NULL;
ACPIHeader* XSDT::find(const char* name)
{
    for (size_t i = 0; i < (header.length - sizeof(ACPIHeader)) / 8; i++)
    {
        if (memcmp(name, entries[i]->signature, 4) == 0)
        {
            return entries[i];
        }
    }
    return NULL;
}
void XSDT::loadXSDT(XSDP* xsdp)
{
    instance = (XSDT*)xsdp->xsdtAdress;
    PhysicalMemoryManager::instance.setPages((uint64_t)instance / 0x1000, 1, (instance->header.length + 0xFFF) / 0x1000);
    for (size_t i = 0; i < (instance->header.length - sizeof(ACPIHeader)) / 8; i++)
    {
        PhysicalMemoryManager::instance.setPages((uint64_t)instance->entries[i] / 0x1000, 1,
            (instance->entries[i]->length + 0xFFF) / 0x1000);
    }
}
void XSDT::setupPaging()
{
    uint64_t virt = VirtualMemoryManager::
        getKernelVirtualMemoryManager()->
        allocateAddress((instance->header.length + 0xFFF) / 0x1000);
    VirtualMemoryManager::getKernelVirtualMemoryManager()->map(virt, (uint64_t)instance & VMM_ADDR, VMM_PRESENT | VMM_READ_WRITE
        | VMM_CACHE_DISABLE, (instance->header.length + 0xFFF) / 0x1000);
    uint64_t instanceAddr = virt | ((uint64_t)instance & ~VMM_ADDR);
    for (size_t i = 0; i < (instance->header.length - sizeof(ACPIHeader)) / 8; i++)
    {
        virt = VirtualMemoryManager::
            getKernelVirtualMemoryManager()->
            allocateAddress((instance->entries[i]->length + 0xFFF) / 0x1000);
        VirtualMemoryManager::getKernelVirtualMemoryManager()->map(virt, (uint64_t)instance->entries[i], VMM_PRESENT | VMM_READ_WRITE
            | VMM_CACHE_DISABLE, (instance->entries[i]->length + 0xFFF) / 0x1000);
        instance->entries[i] = (ACPIHeader*)virt;
    }
    instance = (XSDT*)instanceAddr;
}
XSDT* XSDT::getInstance()
{
    return instance;
}