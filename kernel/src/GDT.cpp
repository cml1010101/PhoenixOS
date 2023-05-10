#include <GDT.hpp>
#include <VirtualMemoryManager.hpp>
#include <PhysicalMemoryManager.hpp>
#include <stdarg.h>
extern "C" void load_gdt(SystemPointer* address, uint64_t cs, uint64_t ds, uint64_t es,
    uint64_t fs, uint64_t gs, uint64_t ss);
GDTEntry::GDTEntry()
{
    limit0 = 0;
    base0 = 0;
    base1 = 0;
    access = 0;
    limit1 = 0;
    flags = 0;
    base2 = 0;
}
GDTEntry::GDTEntry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    limit0 = limit & 0xFFFF;
    base0 = base & 0xFFFF;
    base1 = (base >> 16) & 0xFF;
    this->access = access;
    limit1 = (limit >> 16) & 0xF;
    this->flags = flags;
    base2 = (base >> 24) & 0xFF;
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
    VirtualMemoryManager::getCurrentVirtualMemoryManager()->mapPage(
        (uint64_t)(gdtEntries = (GDTEntry*)VirtualMemoryManager::getCurrentVirtualMemoryManager()->allocateAddress(1)),
        PhysicalMemoryManager::instance.allocatePage(),
        VMM_PRESENT | VMM_READ_WRITE
    );
    if (includeUserSegments)
    {
        gdtEntries[0] = GDTEntry();
        gdtEntries[1] = GDTEntry(0, 0xFFFFF, 0x9A, 0xA);
        gdtEntries[2] = GDTEntry(0, 0xFFFFF, 0x92, 0xC);
    }
    else
    {
        gdtEntries[0] = GDTEntry();
        gdtEntries[1] = GDTEntry(0, 0xFFFFF, 0x9A, 0xA);
        gdtEntries[2] = GDTEntry(0, 0xFFFFF, 0x92, 0xC);
    }
}
GDTEntry& GDT::operator[](size_t i)
{
    return gdtEntries[i];
}
GDTEntry GDT::operator[](size_t i) const
{
    return gdtEntries[i];
}
void GDT::load(uint64_t cs, uint64_t ds, uint64_t es, uint64_t fs, uint64_t gs, uint64_t ss)
{
    SystemPointer address;
    address.base = (uint64_t)VirtualMemoryManager::getCurrentVirtualMemoryManager()->getPhysicalAddress(gdtEntries);
    address.limit = (20 * sizeof(GDTEntry)) - 1;
    load_gdt(&address, cs, ds, es, fs, gs, ss);
}