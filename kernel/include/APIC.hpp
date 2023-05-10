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
    uint32_t* registers;
public:
    IOAPIC() = default;
    IOAPIC(uint32_t* registers);
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
    void setInterruptTimer(TimerHandler handler) override;
};
#endif