#include <APIC.hpp>
#include <PhysicalMemoryManager.hpp>
#include <VirtualMemoryManager.hpp>
#include <XSDT.hpp>
#define APIC_BASE_MSR 0x1B
#define APIC_BASE_MSR_ENABLE 0x800
#define APIC_REG_EOI 0xB0
#define APIC_REG_SIV 0xF0
#define APIC_REG_ID 0x20
#define APIC_REG_ICR1 0x300
#define APIC_REG_ICR2 0x310
void LAPIC::writeRegister(uint64_t reg, uint32_t val)
{
    registers[reg] = val;
}
uint32_t LAPIC::readRegister(uint64_t reg)
{
    return registers[reg];
}
LAPIC::LAPIC(uint32_t* registers)
{
    VirtualMemoryManager::getKernelVirtualMemoryManager()->mapPage((uint64_t)(this->registers =
        (uint32_t*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocateAddress(1)), (uint64_t)registers,
        VMM_PRESENT | VMM_CACHE_DISABLE);
}
void LAPIC::enable()
{
    setMSR(APIC_BASE_MSR, (uint64_t)registers | APIC_BASE_MSR_ENABLE);
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
LAPIC LAPIC::getLAPIC()
{
    return LAPIC((uint32_t*)(getMSR(APIC_BASE_MSR) & 0xFFFFFF000));
}
IOAPIC::IOAPIC(uint32_t* address)
{
    VirtualMemoryManager::getKernelVirtualMemoryManager()->mapPage((uint64_t)(this->registers =
        (uint32_t*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocateAddress(1)), (uint64_t)registers,
        VMM_PRESENT | VMM_CACHE_DISABLE);
}
LAPICTimer::LAPICTimer(LAPIC& lapic) : lapic(lapic)
{
    frequency = 0;
}
void LAPICTimer::start()
{
}