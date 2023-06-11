#include <PhysicalMemoryManager.hpp>
#include <QemuLogger.hpp>
#include <VirtualMemoryManager.hpp>
struct
{
    size_t start;
    size_t free: 1;
    size_t pages: 63;
} blocks[128];
extern char _kernel_start, _kernel_end, _cpu_init_start, _cpu_init_end;
PhysicalMemoryManager PhysicalMemoryManager::instance;
PhysicalMemoryManager::PhysicalMemoryManager(void* memoryMap, size_t memoryMapSize, size_t descriptorSize)
{
    Logger::getInstance()->log("Memory map is at 0x%x and has a size of %d with a descriptorSize of %d\n",
        memoryMap, memoryMapSize, descriptorSize);
    memset(blocks, 0, sizeof(blocks));
    size_t k = 0;
    for (size_t i = 0; i < memoryMapSize / descriptorSize; i++)
    {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)(memoryMap + i * descriptorSize);
        if (k == 0)
        {
            blocks[0].start = desc->PhysicalStart;
            blocks[0].free = isUsable(desc->Type);
            blocks[0].pages = desc->NumberOfPages;
        }
        else if (blocks[k - 1].free == isUsable(desc->Type))
        {
            k--;
            blocks[k].pages += desc->NumberOfPages;
        }
        else
        {
            blocks[k].start = desc->PhysicalStart;
            blocks[k].free = isUsable(desc->Type);
            blocks[k].pages = desc->NumberOfPages;
        }
        k++;
    }
    size_t maxPages = 0;
    for (size_t i = 0; i < k; i++)
    {
        Logger::getInstance()->log("Memory block: 0x%x to 0x%x: %s\n", blocks[i].start, blocks[i].start + (blocks[i].pages << 12),
            blocks[i].free ? "Free" : "Reserved");
        if (blocks[i].start + (blocks[i].pages << 12) > maxPages)
            maxPages = blocks[i].start + (blocks[i].pages << 12);
    }
    Logger::getInstance()->log("%d total pages\n", maxPages);
    size_t bitmapSize = ((maxPages + 63) / 64) * 8;
    for (size_t i = 0; i < k; i++)
    {
        if ((blocks[i].pages << 12) >= bitmapSize && blocks[i].free)
        {
            bitmapPhys = bitmap = (uint64_t*)blocks[i].start;
            memset(bitmap, 0, bitmapSize);
            setPages(blocks[i].start >> 12, 1, (bitmapSize + 4095) / 4096);
            break;
        }
    }
    for (size_t i = 0; i < k; i++)
    {
        if (!blocks[i].free)
        {
            setPages(blocks[i].start >> 12, 1, blocks[i].pages);
        }
    }
    uint64_t cpuInit = ((uint64_t)&_cpu_init_start) >> 12;
    uint64_t cpuPages = (((uint64_t)&_cpu_init_end) >> 12) - (((uint64_t)&_cpu_init_start) >> 12);
    setPages(cpuInit, 1, cpuPages);
    uint64_t kernelInit = ((uint64_t)&_kernel_start) >> 12;
    uint64_t kernelPages = (((uint64_t)&_kernel_end) >> 12) - (((uint64_t)&_kernel_start) >> 12);
    setPages(cpuInit, 1, cpuPages);
    setPage(0, 1);
    numTotalPages = maxPages;
}
void PhysicalMemoryManager::setupPaging()
{
    size_t bitmapSize = ((numTotalPages + 63) / 64) * 8;
    size_t numBitmapPages = (bitmapSize + 4095) / 4096;
    uint64_t bitmapVirt = VirtualMemoryManager::getKernelVirtualMemoryManager()->allocateAddress(
        numBitmapPages
    );
    Logger::getInstance()->log("Mapping bitmap: %d size\n", numBitmapPages);
    VirtualMemoryManager::getKernelVirtualMemoryManager()->map(bitmapVirt, (uint64_t)bitmapPhys,
        VMM_PRESENT | VMM_READ_WRITE, numBitmapPages);
    bitmap = (uint64_t*)bitmapVirt;
}