#ifndef CPU_HPP
#define CPU_HPP
#include <PhoenixOS.hpp>
#include <GDT.hpp>
#include <IDT.hpp>
#include <APIC.hpp>
#include <VirtualMemoryManager.hpp>
#include <Scheduler.hpp>
#include <PIT.hpp>
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
        void handleInterrupt(CPURegisters* regs);
        void handleISR(CPURegisters* regs);
        inline Scheduler* getScheduler()
        {
            return &scheduler;
        }
        inline void setInterruptHandler(size_t num, InterruptHandler handler)
        {
            handlers[num] = handler;
        }
        VirtualMemoryManager* getVirtualMemoryManager();
        inline LAPIC& getLAPIC()
        {
            return lapic;
        }
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
    PIT pit;
    static CPU instance;
public:
    CPU() = default;
    void start();
    bool supportsAPIC();
    Core& getCore(size_t i);
    Core& getCurrentCore();
    inline IOAPIC& getIOAPIC()
    {
        return apic;
    }
    static inline void initialize()
    {
        instance.start();
    }
    static inline CPU* getInstance()
    {
        return &instance;
    }
    inline uint64_t getSegment(uint8_t ring, bool code)
    {
        uint8_t codeOffset = code ? 0 : 8;
        switch (ring)
        {
        case 0:
            return 0x8 + codeOffset;
        case 1:
            return 0x19 + codeOffset;
        case 2:
            return 0x2A + codeOffset;
        case 3:
            return 0x3B + codeOffset;
        default:
            return 0;
        }
    }
    inline size_t getNumberOfCores()
    {
        return numCores;
    }
    void initializeScheduling();
};
#endif