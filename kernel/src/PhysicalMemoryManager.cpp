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
PhysicalMemoryManager::PhysicalMemoryManager(swiftboot::MemoryMap map)
{
    Logger::getInstance()->log("Memory map is at 0x%x with %d descriptors\n", map.descriptors, map.numDescriptors);
    memset(blocks, 0, sizeof(blocks));
    size_t k = 0;
    for (size_t i = 0; i < map.numDescriptors; i++)
    {
        swiftboot::MemoryMapDescriptor* desc = &map.descriptors[i];
        if (k == 0)
        {
            blocks[0].start = desc->physicalAddress;
            blocks[0].free = isUsable(desc->type);
            blocks[0].pages = desc->numberOfPages;
        }
        else if (blocks[k - 1].free == isUsable(desc->type))
        {
            k--;
            blocks[k].pages += desc->numberOfPages;
        }
        else
        {
            blocks[k].start = desc->physicalAddress;
            blocks[k].free = isUsable(desc->type);
            blocks[k].pages = desc->numberOfPages;
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
    setPages(kernelInit, 1, kernelPages);
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
void PhysicalMemoryManager::printStatus(Logger* logger)
{
    size_t i = 0, k = 0, flag = getPage(0);
    while ((i + k) < numTotalPages)
    {
        if (!!getPage(i + k) != !!flag)
        {
            logger->log("%d pages at 0x%x: %s\n", k, i << 12, flag ? "Taken" : "Free");
            i += k;
            k = 0;
            if (i < numTotalPages)
                flag = getPage(i);
        }
        else k++;
    }
}