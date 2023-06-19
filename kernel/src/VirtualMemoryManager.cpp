#include <VirtualMemoryManager.hpp>
#include <PhysicalMemoryManager.hpp>
#include <CPU.hpp>
#include <XSDT.hpp>
VirtualMemoryManager* VirtualMemoryManager::kernelVirtualMemoryManager;
extern "C" char _kernel_start;
extern "C" char _kernel_end, _cpu_init_start, _cpu_init_end;
uint64_t VirtualMemoryManager::findFree(uint64_t pages, uint64_t minimumAdress)
{
    size_t i = minimumAdress;
    VirtualAddressAllocation* allocationTable = useVirtualAddresses ? allocationTableVirt : allocationTablePhys;
    size_t k = 0;
    while (i < (1UL << 52))
    {
        bool flag = false;
        for (size_t j = 0; j < 255; j++)
        {
            if (allocationTable[j].size == 0) break;
            if ((allocationTable[j].address >= i && allocationTable[j].address < (i + (pages << 12)))
                || ((allocationTable[j].address + (allocationTable[j].size << 12)) > i
                    && (allocationTable[j].address + (allocationTable[j].size << 12)) <= (i + (pages << 12))))
            {
                i = allocationTable[j].address + (allocationTable[j].size << 12);
                flag = true;
                break;
            }
        }
        if (flag)
        {
            allocationTable = useVirtualAddresses ? allocationTableVirt : allocationTablePhys;
            k = 0;
            continue;
        }
        if (!allocationTable[255].size)
        {
            return i;
        }
        allocationTable = (VirtualAddressAllocation*)allocationTable[255].address;
        k++;
    }
    Logger::getInstance()->panic("Uh oh. Out of virtual memory. This should never happen!!!\n");
    return -1;
}
VirtualMemoryManager::VirtualMemoryManager(bool kernel)
{
    useVirtualAddresses = false;
    pml4Phys = (uint64_t*)(PhysicalMemoryManager::instance.allocatePages(2) << 12);
    allocationTablePhys = (VirtualAddressAllocation*)(PhysicalMemoryManager::instance.allocatePage() << 12);
    memset(pml4Phys, 0, 0x2000);
    memset(allocationTablePhys, 0, 0x1000);
    asm volatile ("mov %%rsp, %0": "=r"(rsp));
    rsp &= VMM_ADDR;
    rsp -= 0x10000;
    allocationTablePhys[0].address = 0;
    allocationTablePhys[0].size = 1;
    allocationTablePhys[2].address = (uint64_t)this;
    allocationTablePhys[2].size = 1;
    allocationTablePhys[3].address = rsp;
    allocationTablePhys[3].size = 32;
    allocationTablePhys[4].address = (uint64_t)&_cpu_init_start;
    allocationTablePhys[4].size = ((uint64_t)&_cpu_init_end - (uint64_t)&_cpu_init_start + 0xFFF) / 0x1000;
    PhysicalMemoryManager::instance.setPages((rsp >> 12) - 16, 1, 48);
    allocationTablePhys[1].address = (uint64_t)&_kernel_start;
    allocationTablePhys[1].size = ((uint64_t)&_kernel_end - (uint64_t)&_kernel_start) / 0x1000;
    allocationTablePhys[255].address = PhysicalMemoryManager::instance.allocatePage() << 12;
    allocationTablePhys[255].size = 1;
    VirtualAddressAllocation* nextPage = (VirtualAddressAllocation*)allocationTablePhys[255].address;
    memset(nextPage, 0, 0x1000);
    Logger::getInstance()->log("Mapping kernel data\n");
    map((uint64_t)&_kernel_start, (uint64_t)&_kernel_start, VMM_READ_WRITE | VMM_PRESENT, allocationTablePhys[1].size);
    map((uint64_t)&_cpu_init_start, (uint64_t)&_cpu_init_start, VMM_READ_WRITE | VMM_PRESENT, allocationTablePhys[4].size);
    Logger::getInstance()->log("Mapping old stack\n");
    map((rsp & VMM_ADDR) - 0x10000, (rsp & VMM_ADDR) - 0x10000, VMM_PRESENT | VMM_READ_WRITE, 48);
}
void VirtualMemoryManager::mapPage(uint64_t virt, uint64_t phys, uint64_t flags)
{
    if (magic != 0xDEADBEEFCAFEBEAD)
    {
        Logger::getInstance()->panic("Magic invalid!\n");
    }
    //Logger::getInstance()->log("Using virtual addresses? %s\n", useVirtualAddresses ? "Yes" : "No");
    uint64_t pml4Idx = (virt >> 39) & 0x1FF;
    uint64_t pdptIdx = (virt >> 30) & 0x1FF;
    uint64_t pdIdx = (virt >> 21) & 0x1FF;
    uint64_t ptIdx = (virt >> 12) & 0x1FF;
    if (useVirtualAddresses)
    {
        if (!(pml4Virt[pml4Idx] & VMM_PRESENT))
        {
            uint64_t paddr = PhysicalMemoryManager::instance.allocatePages(2) << 12;
            uint64_t vaddr = allocateAddress(2, 0xA00000000);
            map(vaddr, paddr, VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE, 2);
            memset((void*)vaddr, 0, 0x2000);
            pml4Virt[pml4Idx] = paddr | flags;
            pml4Virt[512 + pml4Idx] = vaddr;
        }
        uint64_t* pdpt = (uint64_t*)pml4Virt[512 + pml4Idx];
        if (!(pdpt[pdptIdx] & VMM_PRESENT))
        {
            uint64_t paddr = PhysicalMemoryManager::instance.allocatePages(2) << 12;
            uint64_t vaddr = allocateAddress(2, 0xA00000000);
            map(vaddr, paddr, VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE, 2);
            memset((void*)vaddr, 0, 0x2000);
            pdpt[pdptIdx] = paddr | flags;
            pdpt[512 + pdptIdx] = vaddr;
        }
        uint64_t* pd = (uint64_t*)pdpt[512 + pdptIdx];
        if (!(pd[pdIdx] & VMM_PRESENT))
        {
            uint64_t paddr = PhysicalMemoryManager::instance.allocatePage() << 12;
            uint64_t vaddr = allocateAddress(1, 0xA00000000);
            map(vaddr, paddr, VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE, 1);
            memset((void*)vaddr, 0, 0x1000);
            pd[pdIdx] = paddr | flags;
        }
        uint64_t* pt = (uint64_t*)pd[512 + pdIdx];
        pt[ptIdx] = phys | flags;
    }
    else
    {
        if (!(pml4Phys[pml4Idx] & VMM_PRESENT))
        {
            uint64_t paddr = PhysicalMemoryManager::instance.allocatePages(2) << 12;
            uint64_t vaddr = allocateAddress(2, 0xA00000000);
            memset((void*)paddr, 0, 0x2000);
            pml4Phys[pml4Idx] = paddr | flags;
            pml4Phys[512 + pml4Idx] = vaddr;
            map(vaddr, paddr, VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE, 2);
        }
        uint64_t* pdpt = (uint64_t*)(pml4Phys[pml4Idx] & VMM_ADDR);
        if (!(pdpt[pdptIdx] & VMM_PRESENT))
        {
            uint64_t paddr = PhysicalMemoryManager::instance.allocatePages(2) << 12;
            uint64_t vaddr = allocateAddress(2, 0xA00000000);
            memset((void*)paddr, 0, 0x2000);
            pdpt[pdptIdx] = paddr | flags;
            pdpt[512 + pdptIdx] = vaddr;
            map(vaddr, paddr, VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE, 2);
        }
        uint64_t* pd = (uint64_t*)(pdpt[pdptIdx] & VMM_ADDR);
        if (!(pd[pdIdx] & VMM_PRESENT))
        {
            uint64_t paddr = PhysicalMemoryManager::instance.allocatePage() << 12;
            uint64_t vaddr = allocateAddress(1, 0xA00000000);
            memset((void*)paddr, 0, 0x1000);
            pd[pdIdx] = paddr | flags;
            pd[pdIdx + 512] = vaddr;
            mapPage(vaddr, paddr, VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE);
        }
        uint64_t* pt = (uint64_t*)(pd[pdIdx] & VMM_ADDR);
        pt[ptIdx] = phys | flags;
    }
    asm volatile ("mov %cr3, %rax; mov %rax, %cr3");
}
uint64_t VirtualMemoryManager::allocateAddress(uint64_t pages, uint64_t minimumAddress)
{
    size_t i = findFree(pages, minimumAddress);
    VirtualAddressAllocation alloc;
    alloc.address = i;
    alloc.size = pages;
    addAddressAllocation(alloc);
    return i;
}
void VirtualMemoryManager::checkMagic()
{
    if (magic != 0xDEADBEEFCAFEBEAD)
    {
        Logger::getInstance()->panic("Magic is invalid.\n");
    }
}
void VirtualMemoryManager::addAddressAllocation(VirtualAddressAllocation allocation)
{
    VirtualAddressAllocation* allocationTable = useVirtualAddresses ? allocationTableVirt : allocationTablePhys;
    while (true)
    {
        if (!allocationTable[255].size)
        {
            uint64_t phys = PhysicalMemoryManager::instance.allocatePage() << 12;
            if (useVirtualAddresses)
            {
                allocationTable[255].address = findFree(1);
                mapPage(allocationTable[255].address, phys, VMM_PRESENT | VMM_READ_WRITE);
                allocationTable[255].size = 1;
                memset((void*)allocationTable[255].address, 0, 0x1000);
            }
            else
            {
                allocationTable[255].address = phys;
                allocationTable[255].size = 1;
                memset((void*)allocationTable[255].address, 0, 0x1000);
            }
        }
        bool done = false;
        for (size_t j = 0; j < 255; j++)
        {
            if (!allocationTable[j].size)
            {
                allocationTable[j] = allocation;
                done = true;
                break;
            }
        }
        if (done) break;
        allocationTable = (VirtualAddressAllocation*)allocationTable[255].address;
    }
}
void* VirtualMemoryManager::allocate(size_t pages, size_t flags)
{
    uint64_t virt = allocateAddress(pages);
    for (size_t i = 0; i < pages; i++)
    {
        uint64_t phys = PhysicalMemoryManager::instance.allocatePage() << 12;
        mapPage(virt + (i << 12), phys, flags);
    }
    return (void*)virt;
}
uint64_t VirtualMemoryManager::getPhysicalAddress(void* address)
{
    uint64_t virt = (uint64_t)address;
    uint64_t pml4Idx = (virt >> 39) & 0x1FF;
    uint64_t pdptIdx = (virt >> 30) & 0x1FF;
    uint64_t pdIdx = (virt >> 21) & 0x1FF;
    uint64_t ptIdx = (virt >> 12) & 0x1FF;
    if (useVirtualAddresses)
    {
        uint64_t* pdpt = (uint64_t*)pml4Virt[pml4Idx + 512];
        uint64_t* pd = (uint64_t*)pdpt[pdptIdx + 512];
        uint64_t* pt = (uint64_t*)pd[pdIdx + 512];
        return pt[ptIdx] & VMM_ADDR;
    }
    else
    {
        uint64_t* pdpt = (uint64_t*)(pml4Phys[pml4Idx] & VMM_ADDR);
        uint64_t* pd = (uint64_t*)(pdpt[pdptIdx] & VMM_ADDR);
        uint64_t* pt = (uint64_t*)(pd[pdIdx] & VMM_ADDR);
        return pt[ptIdx] & VMM_ADDR;
    }
}
VirtualMemoryManager* VirtualMemoryManager::clone()
{
    static Spinlock lock;
    lock.acquire();
    VirtualMemoryManager* memoryManager = (VirtualMemoryManager*)allocate(1, VMM_PRESENT | VMM_READ_WRITE);
    memoryManager->useVirtualAddresses = true;
    memoryManager->remap = true;
    memoryManager->pml4Virt = (uint64_t*)allocateAddress(2);
    memoryManager->pml4Phys = (uint64_t*)(PhysicalMemoryManager::instance.allocatePages(2) << 12);
    map((uint64_t)memoryManager->pml4Virt, (uint64_t)memoryManager->pml4Phys, VMM_PRESENT | VMM_READ_WRITE, 2);
    Logger::getInstance()->log("Memsetting pml4 to 0, 0x%x(0x%x)\n", memoryManager->pml4Virt, memoryManager->pml4Phys);
    memset(memoryManager->pml4Virt, 0, 0x2000);
    VirtualAddressAllocation* allocationTable = allocationTableVirt;
    VirtualAddressAllocation* newAllocationTable =
        allocationTablePhys = (VirtualAddressAllocation*)PhysicalMemoryManager::instance.allocatePage();
    while (true)
    {
        for (size_t i = 0; i < 255; i++)
        {
            newAllocationTable[i] = allocationTable[i];
        }
        if (allocationTable[255].address)
        {
            uint64_t phys = PhysicalMemoryManager::instance.allocatePage() << 12;
            uint64_t virt = allocateAddress(1);
            mapPage(virt, phys, VMM_PRESENT | VMM_READ_WRITE);
            newAllocationTable[255].address = virt;
            newAllocationTable[255].size = 1;
            newAllocationTable = (VirtualAddressAllocation*)virt;
        }
        else break;
    }
    Logger::getInstance()->log("Copying page table contents\n");
    for (size_t i = 0; i < 512; i++)
    {
        if (pml4Virt[i] & 1 && !(memoryManager->pml4Virt[i] & 1))
        {
            Logger::getInstance()->log("Copying PDPT\n");
            uint64_t* pdptVirt = (uint64_t*)pml4Virt[i + 512];
            uint64_t newPdptPhys = PhysicalMemoryManager::instance.allocatePages(2) << 12;
            uint64_t* newPdptVirt = (uint64_t*)allocateAddress(2, 0xA00000000);
            map((uint64_t)newPdptVirt, newPdptPhys, VMM_PRESENT | VMM_READ_WRITE, 2);
            Logger::getInstance()->log("Mapped new PDPT\n");
            memset(newPdptVirt, 0, 0x2000);
            memoryManager->pml4Virt[i] = newPdptPhys | VMM_PRESENT | VMM_READ_WRITE;
            memoryManager->pml4Phys[i + 512] = (uint64_t)newPdptVirt;
            for (size_t j = 0; j < 512; j++)
            {
                if (pdptVirt[j] & 1 && !(newPdptVirt[j] & 1))
                {
                    Logger::getInstance()->log("Copying PD\n");
                    uint64_t* pdVirt = (uint64_t*)pdptVirt[i + 512];
                    uint64_t newPdPhys = PhysicalMemoryManager::instance.allocatePages(2) << 12;
                    uint64_t* newPdVirt = (uint64_t*)allocateAddress(2, 0xA00000000);
                    map((uint64_t)newPdVirt, newPdPhys, VMM_PRESENT | VMM_READ_WRITE, 2);
                    memset(newPdVirt, 0, 0x2000);
                    newPdptVirt[j] = newPdPhys | VMM_PRESENT | VMM_READ_WRITE;
                    newPdptVirt[j + 512] = (uint64_t)newPdVirt;
                    for (size_t k = 0; k < 512; k++)
                    {
                        Logger::getInstance()->log("Copying PT\n");
                        uint64_t* ptVirt = (uint64_t*)pdVirt[i + 512];
                        uint64_t newPtPhys = PhysicalMemoryManager::instance.allocatePage() << 12;
                        uint64_t* newPtVirt = (uint64_t*)allocateAddress(1, 0xA00000000);
                        map((uint64_t)newPdVirt, newPdPhys, VMM_PRESENT | VMM_READ_WRITE, 1);
                        memcpy(newPtVirt, ptVirt, 0x1000);
                        newPdVirt[k] = newPtPhys | VMM_PRESENT | VMM_READ_WRITE;
                        newPdVirt[k + 512] = (uint64_t)newPtVirt;
                    }
                }
            }
        }
    }
    lock.release();
    return memoryManager;
}
VirtualMemoryManager* VirtualMemoryManager::getKernelVirtualMemoryManager()
{
    return kernelVirtualMemoryManager;
}
void VirtualMemoryManager::initialize()
{
    kernelVirtualMemoryManager = (VirtualMemoryManager*)(PhysicalMemoryManager::instance.allocatePage() << 12);
    *kernelVirtualMemoryManager = VirtualMemoryManager(true);
    kernelVirtualMemoryManager->mapPage((uint64_t)kernelVirtualMemoryManager, (uint64_t)kernelVirtualMemoryManager,
        VMM_PRESENT | VMM_READ_WRITE);
    Logger::getInstance()->log("kernel addr: 0x%x\n", kernelVirtualMemoryManager);
    VirtualAddressAllocation alloc;
    alloc.size = 1;
    alloc.address = (uint64_t)kernelVirtualMemoryManager;
    kernelVirtualMemoryManager->addAddressAllocation(alloc);
    kernelVirtualMemoryManager->printAllocationTable(Logger::getInstance());
}
void VirtualMemoryManager::printAllocationTable(Logger* logger)
{
    VirtualAddressAllocation* allocationTable = useVirtualAddresses ? allocationTableVirt : allocationTablePhys;
    size_t k = 0;
    while (allocationTable)
    {
        for (size_t i = 0; i < 255; i++)
        {
            if (!allocationTable[i].size) continue;
            logger->log("[%d, %d]: %d pages at 0x%x\n", k, i, allocationTable[i].size, allocationTable[i].address);
        }
        k++;
        allocationTable = (VirtualAddressAllocation*)allocationTable[255].address;
    }
}
void VirtualMemoryManager::reset()
{
    static Spinlock lock;
    lock.acquire();
    if (!useVirtualAddresses)
    {
        VirtualAddressAllocation* allocationTable = allocationTablePhys;
        allocationTableVirt = (VirtualAddressAllocation*)allocateAddress(1);
        mapPage((uint64_t)allocationTableVirt, (uint64_t)allocationTablePhys, VMM_PRESENT | VMM_READ_WRITE);
        while (allocationTable[255].address)
        {
            VirtualAddressAllocation* nextTable = (VirtualAddressAllocation*)allocationTable[255].address;
            allocationTable[255].address = allocateAddress(1);
            mapPage(allocationTable[255].address, (uint64_t)nextTable, VMM_PRESENT | VMM_READ_WRITE);
            allocationTable = nextTable;
        }
        pml4Virt = (uint64_t*)allocateAddress(2);
        map((uint64_t)pml4Virt, (uint64_t)pml4Phys, VMM_PRESENT | VMM_READ_WRITE, 2);
        PhysicalMemoryManager::instance.setupPaging();
        XSDT::setupPaging();
        useVirtualAddresses = true;
    }
    else if (remap)
    {
        for (size_t i = 0; i < 512; i++)
        {
            if (pml4Virt[i + 512])
            {
                uint64_t* pdpt = (uint64_t*)pml4Virt[i + 512];
                pml4Virt[i + 512] = allocateAddress(2);
                map(pml4Virt[i + 512], pml4Virt[i] & VMM_ADDR, VMM_PRESENT | VMM_READ_WRITE, 2);
                for (size_t j = 0; j < 512; j++)
                {
                    uint64_t* pd = (uint64_t*)pdpt[j + 512];
                    pdpt[j + 512] = allocateAddress(2);
                    map(pdpt[j + 512], pdpt[i] & VMM_ADDR, VMM_PRESENT | VMM_READ_WRITE, 2);
                    for (size_t k = 0; k < 512; k++)
                    {
                        pd[k + 512] = allocateAddress(1);
                        map(pd[k + 512], pd[k] & VMM_ADDR, VMM_PRESENT | VMM_READ_WRITE, 2);
                    }
                }
            }
        }
        remap = false;
    }
    asm volatile ("mov %0, %%cr3":: "r"((uint64_t)pml4Phys));
    checkMagic();
    lock.release();
    useVirtualAddresses = true;
}