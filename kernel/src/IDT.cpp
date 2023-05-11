#include <IDT.hpp>
#include <VirtualMemoryManager.hpp>
#include <PhysicalMemoryManager.hpp>
extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();
extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();
extern "C" void int254();
extern "C" void int255();
extern "C" void load_idt(SystemPointer* ptr);
IDTEntry::IDTEntry()
{
    offset0 = 0;
    selector = 0;
    ist = 0;
    rsv0 = 0;
    type = 0;
    dpl = 0;
    p = 0;
    offset1 = 0;
    offset2 = 0;
    rsv1 = 0;
}
IDTEntry::IDTEntry(uint64_t offset, uint16_t selector, uint8_t type, uint8_t dpl, uint8_t ist)
{
    offset0 = offset & 0xFFFF;
    this->selector = selector;
    this->ist = ist;
    rsv0 = 0;
    this->type = type;
    this->dpl = dpl;
    p = 1;
    offset1 = (offset >> 16) & 0xFFFF;
    offset2 = (offset >> 32) & 0xFFFFFFFF;
    rsv1 = 0;
}
IDT::IDT(bool unused)
{
    VirtualMemoryManager::getCurrentVirtualMemoryManager()->mapPage(
        (uint64_t)(entries = (IDTEntry*)VirtualMemoryManager::getCurrentVirtualMemoryManager()->allocateAddress(1)),
        PhysicalMemoryManager::instance.allocatePage(),
        VMM_PRESENT | VMM_READ_WRITE
    );
    entries[0] = IDTEntry((uint64_t)isr0, 0x8, 0b1110, 0, 0);
    entries[1] = IDTEntry((uint64_t)isr1, 0x8, 0b1110, 0, 0);
    entries[2] = IDTEntry((uint64_t)isr2, 0x8, 0b1110, 0, 0);
    entries[3] = IDTEntry((uint64_t)isr3, 0x8, 0b1110, 0, 0);
    entries[4] = IDTEntry((uint64_t)isr4, 0x8, 0b1110, 0, 0);
    entries[5] = IDTEntry((uint64_t)isr5, 0x8, 0b1110, 0, 0);
    entries[6] = IDTEntry((uint64_t)isr6, 0x8, 0b1110, 0, 0);
    entries[7] = IDTEntry((uint64_t)isr7, 0x8, 0b1110, 0, 0);
    entries[8] = IDTEntry((uint64_t)isr8, 0x8, 0b1110, 0, 0);
    entries[9] = IDTEntry((uint64_t)isr9, 0x8, 0b1110, 0, 0);
    entries[10] = IDTEntry((uint64_t)isr10, 0x8, 0b1110, 0, 0);
    entries[11] = IDTEntry((uint64_t)isr11, 0x8, 0b1110, 0, 0);
    entries[12] = IDTEntry((uint64_t)isr12, 0x8, 0b1110, 0, 0);
    entries[13] = IDTEntry((uint64_t)isr13, 0x8, 0b1110, 0, 0);
    entries[14] = IDTEntry((uint64_t)isr14, 0x8, 0b1110, 0, 0);
    entries[15] = IDTEntry((uint64_t)isr15, 0x8, 0b1110, 0, 0);
    entries[16] = IDTEntry((uint64_t)isr16, 0x8, 0b1110, 0, 0);
    entries[17] = IDTEntry((uint64_t)isr17, 0x8, 0b1110, 0, 0);
    entries[18] = IDTEntry((uint64_t)isr18, 0x8, 0b1110, 0, 0);
    entries[19] = IDTEntry((uint64_t)isr19, 0x8, 0b1110, 0, 0);
    entries[20] = IDTEntry((uint64_t)isr20, 0x8, 0b1110, 0, 0);
    entries[21] = IDTEntry((uint64_t)isr21, 0x8, 0b1110, 0, 0);
    entries[22] = IDTEntry((uint64_t)isr22, 0x8, 0b1110, 0, 0);
    entries[23] = IDTEntry((uint64_t)isr23, 0x8, 0b1110, 0, 0);
    entries[24] = IDTEntry((uint64_t)isr24, 0x8, 0b1110, 0, 0);
    entries[25] = IDTEntry((uint64_t)isr25, 0x8, 0b1110, 0, 0);
    entries[26] = IDTEntry((uint64_t)isr26, 0x8, 0b1110, 0, 0);
    entries[27] = IDTEntry((uint64_t)isr27, 0x8, 0b1110, 0, 0);
    entries[28] = IDTEntry((uint64_t)isr28, 0x8, 0b1110, 0, 0);
    entries[29] = IDTEntry((uint64_t)isr29, 0x8, 0b1110, 0, 0);
    entries[30] = IDTEntry((uint64_t)isr30, 0x8, 0b1110, 0, 0);
    entries[31] = IDTEntry((uint64_t)isr31, 0x8, 0b1110, 0, 0);
    entries[32] = IDTEntry((uint64_t)irq0, 0x8, 0b1110, 0, 0);
    entries[33] = IDTEntry((uint64_t)irq1, 0x8, 0b1110, 0, 0);
    entries[34] = IDTEntry((uint64_t)irq2, 0x8, 0b1110, 0, 0);
    entries[35] = IDTEntry((uint64_t)irq3, 0x8, 0b1110, 0, 0);
    entries[36] = IDTEntry((uint64_t)irq4, 0x8, 0b1110, 0, 0);
    entries[37] = IDTEntry((uint64_t)irq5, 0x8, 0b1110, 0, 0);
    entries[38] = IDTEntry((uint64_t)irq6, 0x8, 0b1110, 0, 0);
    entries[39] = IDTEntry((uint64_t)irq7, 0x8, 0b1110, 0, 0);
    entries[40] = IDTEntry((uint64_t)irq8, 0x8, 0b1110, 0, 0);
    entries[41] = IDTEntry((uint64_t)irq9, 0x8, 0b1110, 0, 0);
    entries[42] = IDTEntry((uint64_t)irq10, 0x8, 0b1110, 0, 0);
    entries[43] = IDTEntry((uint64_t)irq11, 0x8, 0b1110, 0, 0);
    entries[44] = IDTEntry((uint64_t)irq12, 0x8, 0b1110, 0, 0);
    entries[45] = IDTEntry((uint64_t)irq13, 0x8, 0b1110, 0, 0);
    entries[46] = IDTEntry((uint64_t)irq14, 0x8, 0b1110, 0, 0);
    entries[47] = IDTEntry((uint64_t)irq15, 0x8, 0b1110, 0, 0);
    entries[254] = IDTEntry((uint64_t)int254, 0x8, 0b1110, 0, 0);
    entries[255] = IDTEntry((uint64_t)int255, 0x8, 0b1110, 0, 0);
}
void IDT::load()
{
    SystemPointer address;
    address.base = (uint64_t)VirtualMemoryManager::getCurrentVirtualMemoryManager()->getPhysicalAddress(entries);
    address.limit = (256 * sizeof(IDTEntry)) - 1;
    load_idt(&address);
}