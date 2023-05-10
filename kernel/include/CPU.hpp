#ifndef CPU_HPP
#define CPU_HPP
#include <PhoenixOS.hpp>
#include <GDT.hpp>
#include <IDT.hpp>
#include <APIC.hpp>
#include <VirtualMemoryManager.hpp>
#include <Scheduler.hpp>
#include "cpuid.h"
typedef void(*InterruptHandler)(CPURegisters*);
class CPU
{
public:
    class Core
    {
    private:
        IDT idt;
        GDT gdt;
        LAPIC lapic;
        VirtualMemoryManager* vmm;
        InterruptHandler handlers[256];
        bool initialized;
        Scheduler scheduler;
    public:
        inline Core()
        {
            initialized = false;
            vmm = NULL;
            memset(handlers, 0, sizeof(handlers));
        }
        void setVirtualMemoryManager(VirtualMemoryManager* vmm);
        void resetMMU();
        void initializeGDT();
        void resetGDT();
        void initializeIDT();
        void resetIDT();
        void initializeLAPIC(LAPIC lapic);
        void initializePIC();
        void handleIRQ(CPURegisters* regs);
        void handleISR(CPURegisters* regs);
        inline void setInterruptHandler(size_t num, InterruptHandler handler)
        {
            handlers[num] = handler;
        }
        VirtualMemoryManager* getVirtualMemoryManager();
        inline bool isInitialized()
        {
            return initialized;
        }
        inline void setInitialized(bool initialized)
        {
            initialized = true;
        }
    };
private:
    size_t numCores;
    Core cores[16];
    IOAPIC apic;
    static CPU* instance;
public:
    CPU();
    bool supportsAPIC();
    Core& getCore(size_t i);
    Core& getCurrentCore();
    static inline void initialize()
    {
        instance = (CPU*)VirtualMemoryManager::getCurrentVirtualMemoryManager()->allocate(1);
        *instance = CPU();
    }
    static inline CPU* getInstance()
    {
        return instance;
    }
};
#endif