#ifndef TIMER_HPP
#define TIMER_HPP
#include <PhoenixOS.hpp>
typedef void(*TimerHandler)(CPURegisters* regs);
class Timer
{
public:
    Timer() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual size_t getCount() = 0;
    virtual size_t getFrequency() = 0;
    virtual double getMicroseconds() = 0;
    virtual void setInterruptHandler(TimerHandler handler) = 0;
};
#endif