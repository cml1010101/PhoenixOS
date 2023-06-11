#ifndef APIC_HPP
#define APIC_HPP
#include <PhoenixOS.hpp>
#include <Timer.hpp>
class LAPIC;
class LAPICTimer : public Timer
{
private:
    LAPIC* lapic;
    size_t frequency, count;
    TimerHandler handler;
public:
    LAPICTimer() = default;
    LAPICTimer(LAPIC* lapic);
    void start() override;
    void stop() override;
    void setFrequency(size_t frequency);
    size_t getCount() override;
    inline void incrementCount()
    {
        count++;
    }
    size_t getFrequency() override;
    double getMicroseconds() override;
    void setInterruptHandler(TimerHandler handler) override;
    inline TimerHandler getInterruptHandler()
    {
        return handler;
    }
};
class LAPIC
{
private:
    friend class LAPICTimer;
    LAPICTimer timer;
    static uint32_t* registers;
    static void writeRegister(uint64_t reg, uint32_t val);
    static uint32_t readRegister(uint64_t reg);
public:
    static void initialize();
    LAPIC() = default;
    void enable();
    void sendEOI();
    size_t getID();
    void sendIPI(uint8_t destination, uint32_t dsh, uint32_t type, uint8_t vector);
    inline LAPICTimer* getTimer()
    {
        return &timer;
    }
    inline static uint64_t getAPICID()
    {
        return readRegister(0x20) >> 24;
    }
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
    void disableRedirection(uint64_t number);
};
#endif