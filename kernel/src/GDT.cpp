#include <GDT.hpp>
#include <VirtualMemoryManager.hpp>
#include <PhysicalMemoryManager.hpp>
#include <CPU.hpp>
#include <stdarg.h>
extern "C" void load_gdt(SystemPointer* address, uint64_t cs, uint64_t ds, uint64_t es,
    uint64_t fs, uint64_t gs, uint64_t ss);
GDTEntry::GDTEntry()
{
    limitLow = 0;
    baseLow = 0;
    baseMiddle = 0;
    access = 0;
    granularity = 0;
    baseHigh = 0;
}
GDTEntry::GDTEntry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    limitLow = limit & 0xFFFF;
    baseLow = base & 0xFFFF;
    baseMiddle = (base >> 16) & 0xFF;
    this->access = access;
    granularity = ((limit >> 16) & 0xF) | (flags << 4);
    baseHigh = (base >> 24) & 0xFF;
    Logger::getInstance()->log("%x\n", *(uint64_t*)this);
}
GDTSystemEntry::GDTSystemEntry()
{
    limit0 = 0;
    base0 = 0;
    base1 = 0;
    access = 0;
    limit1 = 0;
    flags = 0;
    base2 = 0;
    base3 = 0;
    rsv = 0;
}
GDTSystemEntry::GDTSystemEntry(uint64_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    limit0 = limit & 0xFFFF;
    base0 = base & 0xFFFF;
    base1 = (base >> 16) & 0xFF;
    this->access = access;
    limit1 = (limit >> 16) & 0xF;
    this->flags = flags;
    base2 = (base >> 24) & 0xFF;
    base3 = (base >> 32) & 0xFFFFFFFF;
    rsv = 0;
}
GDT::GDT(bool includeUserSegments)
{
    CPU::getInstance()->getCurrentCore().getVirtualMemoryManager()->checkMagic();
    gdtEntries = (uint64_t*)CPU::getInstance()->getCurrentCore().getVirtualMemoryManager()->allocate(
        1,
        VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE
    );
    CPU::getInstance()->getCurrentCore().getVirtualMemoryManager()->checkMagic();
    Logger::getInstance()->log("&gdtEntries = 0x%x\n", gdtEntries);
    Logger::getInstance()->log("Physical &gdtEntries = 0x%x\n", CPU::getInstance()->getCurrentCore().getVirtualMemoryManager()
        ->getPhysicalAddress(gdtEntries));
    if (includeUserSegments)
    {
        gdtEntries[0] = 0;
        gdtEntries[1] = (1UL << 44UL) | (1UL << 47UL) | (1UL << 41UL) | (1UL << 43UL) | (1UL << 53UL);
        gdtEntries[2] = (1UL << 44UL) | (1UL << 47UL) | (1UL << 41UL);
    }
    else
    {
        new(&gdtEntries[0])GDTEntry();
        new(&gdtEntries[1])GDTEntry(0, 0xFFFFF, 0x9A, 0xA);
        new(&gdtEntries[2])GDTEntry(0, 0xFFFFF, 0x92, 0xC);
    }
}
void GDT::load(uint64_t cs, uint64_t ds, uint64_t es, uint64_t fs, uint64_t gs, uint64_t ss)
{
    SystemPointer address;
    address.base = (uint64_t)gdtEntries;
    address.limit = (20 * sizeof(GDTEntry)) - 1;
    load_gdt(&address, cs, ds, es, fs, gs, ss);
}