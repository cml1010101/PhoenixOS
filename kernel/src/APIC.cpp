#include <APIC.hpp>
#include <PhysicalMemoryManager.hpp>
#include <VirtualMemoryManager.hpp>
#include <PIT.hpp>
#include <XSDT.hpp>
#include <CPU.hpp>
#define APIC_BASE_MSR 0x1B
#define APIC_BASE_MSR_ENABLE 0x800
#define APIC_REG_EOI 0xB0
#define APIC_REG_SIV 0xF0
#define APIC_REG_ID 0x20
#define APIC_REG_ICR1 0x300
#define APIC_REG_ICR2 0x310
#define APIC_REG_TPR 0x80
uint32_t* LAPIC::registers;
uint64_t LAPIC::registersPhys;
void LAPIC::writeRegister(uint64_t reg, uint32_t val)
{
    registers[reg / 4] = val;
}
uint32_t LAPIC::readRegister(uint64_t reg)
{
    return registers[reg / 4];
}
void LAPIC::initialize()
{
    registersPhys = getMSR(APIC_BASE_MSR) & 0xFFFFFF000;
    VirtualMemoryManager::getKernelVirtualMemoryManager()->mapPage((uint64_t)(registers =
        (uint32_t*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocateAddress(1)), registersPhys,
        VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE);
}
void LAPIC::enable()
{
    setMSR(APIC_BASE_MSR, getMSR(APIC_BASE_MSR) | APIC_BASE_MSR_ENABLE);
    writeRegister(APIC_REG_SIV, readRegister(APIC_REG_SIV) | 0x100);
}
void LAPIC::sendEOI()
{
    writeRegister(APIC_REG_EOI, 0);
}
size_t LAPIC::getID()
{
    return readRegister(APIC_REG_ID);
}
void LAPIC::sendIPI(uint8_t destination, uint32_t dsh, uint32_t type, uint8_t vector)
{
    uint32_t high = ((uint32_t)destination) << 24;
    uint32_t low = dsh | type | vector;
    writeRegister(APIC_REG_ICR2, high);
    writeRegister(APIC_REG_ICR1, low);
}
IOAPIC::IOAPIC(uint32_t* address)
{
    VirtualMemoryManager::getKernelVirtualMemoryManager()->mapPage((uint64_t)(this->registers =
       (uint32_t*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocateAddress(1)), (uint64_t)address,
        VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE);
}
void IOAPIC::setRedirection(size_t number, uint64_t destination, uint64_t vector)
{
    Logger::getInstance()->log("Mapping global %d to %d\n", number, vector);
    writeRegister(number * 2 + 0x10, vector);
    writeRegister(number * 2 + 0x11, destination << 24);
}
void IOAPIC::disableRedirection(size_t number)
{
    writeRegister(number * 2 + 0x10, 1 << 16);
}
uint64_t IOAPIC::getIOAPICVER()
{
    return readRegister(1);
}
LAPICTimer::LAPICTimer(LAPIC* lapic) : lapic(lapic)
{
    frequency = 0;
}
void LAPICTimer::start()
{
    lapic->writeRegister(0x320, 0xFE);
    lapic->writeRegister(0x3E0, 3);
    lapic->writeRegister(0x380, 0xFFFFFFFF);
    Logger::getInstance()->log("Preparing to enter planned sleep\n");
    PIT::getInstance()->sleep(10000);
    Logger::getInstance()->log("Slept for 10000us\n");
    lapic->writeRegister(0x320, 0xFE | (1 << 16));
    uint64_t ticks = 0xFFFFFFFF - lapic->readRegister(0x390);
    double cpuFrequency = ticks * 100;
    double count = cpuFrequency / frequency;
    count = 0;
    lapic->writeRegister(0x320, 0xFE | (1 << 17));
    lapic->writeRegister(0x3E0, 3);
    lapic->writeRegister(0x380, (uint32_t)count);
    CPU::getInstance()->getCurrentCore().setInterruptHandler(254, [](CPURegisters* regs) {
        auto timer = CPU::getInstance()->getCurrentCore().getLAPIC().getTimer();
        timer->incrementCount();
        if (timer->getInterruptHandler()) timer->getInterruptHandler()(regs);
    });
}
void LAPICTimer::stop()
{
    lapic->writeRegister(0x320, 0xFE | (1 << 16));
}
void LAPICTimer::setFrequency(uint64_t frequency)
{
    this->frequency = frequency;
}
uint64_t LAPICTimer::getCount()
{
    return count;
}
uint64_t LAPICTimer::getFrequency()
{
    return frequency;
}
double LAPICTimer::getMicroseconds()
{
    return (double)count / frequency * 1e6;
}
void LAPICTimer::setInterruptHandler(InterruptHandler handler)
{
    this->handler = handler;
}