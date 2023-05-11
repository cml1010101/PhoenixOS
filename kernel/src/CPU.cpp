#include <CPU.hpp>
#include <XSDT.hpp>
#include <PIC.hpp>
#include <cpuid.h>
#include <Scheduler.hpp>
enum {
    CPUID_FEAT_ECX_SSE3         = 1 << 0, 
    CPUID_FEAT_ECX_PCLMUL       = 1 << 1,
    CPUID_FEAT_ECX_DTES64       = 1 << 2,
    CPUID_FEAT_ECX_MONITOR      = 1 << 3,  
    CPUID_FEAT_ECX_DS_CPL       = 1 << 4,  
    CPUID_FEAT_ECX_VMX          = 1 << 5,  
    CPUID_FEAT_ECX_SMX          = 1 << 6,  
    CPUID_FEAT_ECX_EST          = 1 << 7,  
    CPUID_FEAT_ECX_TM2          = 1 << 8,  
    CPUID_FEAT_ECX_SSSE3        = 1 << 9,  
    CPUID_FEAT_ECX_CID          = 1 << 10,
    CPUID_FEAT_ECX_SDBG         = 1 << 11,
    CPUID_FEAT_ECX_FMA          = 1 << 12,
    CPUID_FEAT_ECX_CX16         = 1 << 13, 
    CPUID_FEAT_ECX_XTPR         = 1 << 14, 
    CPUID_FEAT_ECX_PDCM         = 1 << 15, 
    CPUID_FEAT_ECX_PCID         = 1 << 17, 
    CPUID_FEAT_ECX_DCA          = 1 << 18, 
    CPUID_FEAT_ECX_SSE4_1       = 1 << 19, 
    CPUID_FEAT_ECX_SSE4_2       = 1 << 20, 
    CPUID_FEAT_ECX_X2APIC       = 1 << 21, 
    CPUID_FEAT_ECX_MOVBE        = 1 << 22, 
    CPUID_FEAT_ECX_POPCNT       = 1 << 23, 
    CPUID_FEAT_ECX_TSC          = 1 << 24, 
    CPUID_FEAT_ECX_AES          = 1 << 25, 
    CPUID_FEAT_ECX_XSAVE        = 1 << 26, 
    CPUID_FEAT_ECX_OSXSAVE      = 1 << 27, 
    CPUID_FEAT_ECX_AVX          = 1 << 28,
    CPUID_FEAT_ECX_F16C         = 1 << 29,
    CPUID_FEAT_ECX_RDRAND       = 1 << 30,
    CPUID_FEAT_ECX_HYPERVISOR   = 1 << 31,
    CPUID_FEAT_EDX_FPU          = 1 << 0,  
    CPUID_FEAT_EDX_VME          = 1 << 1,  
    CPUID_FEAT_EDX_DE           = 1 << 2,  
    CPUID_FEAT_EDX_PSE          = 1 << 3,  
    CPUID_FEAT_EDX_TSC          = 1 << 4,  
    CPUID_FEAT_EDX_MSR          = 1 << 5,  
    CPUID_FEAT_EDX_PAE          = 1 << 6,  
    CPUID_FEAT_EDX_MCE          = 1 << 7,  
    CPUID_FEAT_EDX_CX8          = 1 << 8,  
    CPUID_FEAT_EDX_APIC         = 1 << 9,  
    CPUID_FEAT_EDX_SEP          = 1 << 11, 
    CPUID_FEAT_EDX_MTRR         = 1 << 12, 
    CPUID_FEAT_EDX_PGE          = 1 << 13, 
    CPUID_FEAT_EDX_MCA          = 1 << 14, 
    CPUID_FEAT_EDX_CMOV         = 1 << 15, 
    CPUID_FEAT_EDX_PAT          = 1 << 16, 
    CPUID_FEAT_EDX_PSE36        = 1 << 17, 
    CPUID_FEAT_EDX_PSN          = 1 << 18, 
    CPUID_FEAT_EDX_CLFLUSH      = 1 << 19, 
    CPUID_FEAT_EDX_DS           = 1 << 21, 
    CPUID_FEAT_EDX_ACPI         = 1 << 22, 
    CPUID_FEAT_EDX_MMX          = 1 << 23, 
    CPUID_FEAT_EDX_FXSR         = 1 << 24, 
    CPUID_FEAT_EDX_SSE          = 1 << 25, 
    CPUID_FEAT_EDX_SSE2         = 1 << 26, 
    CPUID_FEAT_EDX_SS           = 1 << 27, 
    CPUID_FEAT_EDX_HTT          = 1 << 28, 
    CPUID_FEAT_EDX_TM           = 1 << 29, 
    CPUID_FEAT_EDX_IA64         = 1 << 30,
    CPUID_FEAT_EDX_PBE          = 1 << 31
};
void CPU::Core::setVirtualMemoryManager(VirtualMemoryManager* vmm)
{
    this->vmm = vmm;
}
void CPU::Core::resetMMU()
{
    asm volatile ("mov %0, %%cr3":: "r"(vmm->getPhysicalAddress()));
}
void CPU::Core::initializeGDT()
{
    gdt = GDT(true);
}
void CPU::Core::resetGDT()
{
    gdt.load(0x8, 0x10, 0x10, 0x10, 0x10, 0x10);
}
void CPU::Core::initializeIDT()
{
    idt = IDT(true);
}
void CPU::Core::resetIDT()
{
    idt.load();
}
void CPU::Core::initializePIC()
{
    PIC::enable();
}
void CPU::Core::initializeLAPIC(LAPIC lapic)
{
    this->lapic = lapic;
    PIC::disable();
    lapic.enable();
}
void CPU::Core::handleInterrupt(CPURegisters* regs)
{
    if (CPU::getInstance()->supportsAPIC()) lapic.sendEOI();
    else if (regs->num > 31 && regs->num < 48) PIC::sendEOI(regs->num);
    if (handlers[regs->num]) handlers[regs->num](regs);
}
const char* isrMessages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};
void CPU::Core::handleISR(CPURegisters* regs)
{
    size_t intNum = regs->num;
    if (handlers[intNum]) handlers[intNum](regs);
    Logger::getInstance()->log("[0x%x]: %s(0x%x)\n", regs->rip, isrMessages[regs->num], regs->code);
    if (scheduler.getActiveThread() == Scheduler::getKernelThread())
    {
        for (;;);
    }
    else
    {
        scheduler.reportISR(regs);
    }
}
VirtualMemoryManager* CPU::Core::getVirtualMemoryManager()
{
    return vmm;
}
extern "C" void cpu_start_16();
extern "C" uint64_t cpu_start_stack;
extern "C" uint32_t cpu_start_cr3;
extern "C" uint64_t cpu_start_id;
CPU* CPU::instance = NULL;
CPU::CPU()
{
    numCores = 1;
    cores[0].setVirtualMemoryManager(VirtualMemoryManager::getKernelVirtualMemoryManager());
    cores[0].resetMMU();
    cores[0].initializeGDT();
    cores[0].resetGDT();
    cores[0].initializeIDT();
    cores[0].resetIDT();
    pit.setFrequency(1e5);
    pit.start();
    uint32_t lapicIDs[32];
    if (supportsAPIC())
    {
        numCores = 0;
        auto madt = XSDT::getInstance()->find("APIC");
        uint8_t* data = (uint8_t*)&madt[1];
        uint32_t ioapicAddress;
        for (size_t i = 0; i < madt->length - sizeof(ACPIHeader); i += data[i + 1])
        {
            switch (data[i])
            {
            case 0:
                lapicIDs[numCores++] = data[3];
                break;
            case 1:
                ioapicAddress = *(uint32_t*)(&data[4]);
                break;
            }
        }
        apic = IOAPIC((uint32_t*)ioapicAddress);
        cores[0].initializeLAPIC(LAPIC::getLAPIC());
        uint64_t stack = (uint64_t)VirtualMemoryManager::getCurrentVirtualMemoryManager()->allocate(4) + 0x4000;
        for (size_t i = 1; i < numCores; i++)
        {
            cores[i].setVirtualMemoryManager(VirtualMemoryManager::getKernelVirtualMemoryManager());
            cpu_start_id = i;
            cpu_start_stack = stack;
            cpu_start_cr3 = cores[i].getVirtualMemoryManager()->getPhysicalAddress();
            LAPIC::getLAPIC().sendIPI(i, 0, 5 << 8, 0);
            LAPIC::getLAPIC().sendIPI(i, 0, 6 << 8, (uint64_t)&cpu_start_16 >> 12);
            while (!cores[i].isInitialized());
        }
    }
    else
    {
        cores[0].initializePIC();
    }
}
bool CPU::supportsAPIC()
{
    uint32_t eax, unused, edx;
    __get_cpuid(1, &eax, &unused, &unused, &edx);
    return edx & CPUID_FEAT_EDX_APIC;
}
CPU::Core& CPU::getCore(size_t i)
{
    return cores[i];
}
CPU::Core& CPU::getCurrentCore()
{
    return cores[LAPIC::getLAPIC().getID()];
}
extern "C" void core_initialization_routine(uint64_t id)
{
    CPU::Core& core = CPU::getInstance()->getCore(id);
    core.resetMMU();
    core.initializeGDT();
    core.resetGDT();
    core.initializeIDT();
    core.resetIDT();
    core.initializeLAPIC(LAPIC::getLAPIC());
    core.getLAPIC().getTimer()->setFrequency(1e+5);
    core.getLAPIC().getTimer()->start();
    core.getLAPIC().getTimer()->setInterruptHandler(
    [](CPURegisters* registers)
    {
        CPU::getInstance()->getCurrentCore().schedule(registers);
    });
    core.setInitialized(true);
}
extern "C" void irq_handler(CPURegisters* regs)
{
    CPU::getInstance()->getCurrentCore().handleInterrupt(regs);
}
extern "C" void int_handler(CPURegisters* regs)
{
    CPU::getInstance()->getCurrentCore().handleInterrupt(regs);
}
extern "C" void isr_handler(CPURegisters* regs)
{
    CPU::getInstance()->getCurrentCore().handleISR(regs);
}