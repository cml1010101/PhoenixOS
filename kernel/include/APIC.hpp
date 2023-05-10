#ifndef APIC_HPP
#define APIC_HPP
#include <PhoenixOS.hpp>
#include <Timer.hpp>
class LAPIC
{
private:
    friend class LAPICTimer;
    uint32_t* registers;
    void writeRegister(uint64_t reg, uint32_t val);
    uint32_t readRegister(uint64_t reg);
public:
    LAPIC() = default;
    LAPIC(uint32_t* address);
    void enable();
    void sendEOI();
    size_t getID();
    void sendIPI(uint8_t destination, uint32_t dsh, uint32_t type, uint8_t vector);
    Timer* getTimer();
    static LAPIC getLAPIC();
};
class IOAPIC
{
private:
    volatile uint32_t* registers;
    inline void writeRegister(uint32_t reg, uint32_t val)
    {
        registers[0] = reg;
        registers[4] = val;
    }
    inline uint32_t readRegister(uint32_t reg)
    {
        registers[0] = reg;
        return registers[4];
    }
public:
    IOAPIC() = default;
    IOAPIC(uint32_t* registers);
    void setRedirection(uint64_t number, uint64_t destination, uint64_t vector);
};
class LAPICTimer : public Timer
{
private:
    LAPIC& lapic;
    size_t frequency, count;
    TimerHandler handler;
public:
    LAPICTimer(LAPIC& lapic);
    void start() override;
    void stop() override;
    void setFrequency(size_t frequency);
    size_t getCount() override;
    size_t getFrequency() override;
    double getNanoseconds() override;
    void setInterruptHandler(TimerHandler handler) override;
};
#endif