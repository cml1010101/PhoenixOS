#include <PIT.hpp>
#include <CPU.hpp>
PIT* PIT::instance = NULL;
void PIT::start()
{
    instance = this;
    if (frequency != 0)
    {
        uint64_t divisor = 1193180 / frequency;
        outb(0x43, 0x36);
        uint8_t l = divisor & 0xFF;
        uint8_t h = (divisor >> 8) & 0xFF;
        outb(0x40, l);
        outb(0x40, h);
        count = 0;
        Logger::getInstance()->log("Set divisor to %d\n", divisor);
        CPU::getInstance()->getCurrentCore().setInterruptHandler(33, [](CPURegisters* regs) {
            getInstance()->incrementCount();
            if (getInstance()->getHandler()) getInstance()->getHandler()(regs);
        });
        if (CPU::getInstance()->supportsAPIC())
            CPU::getInstance()->getIOAPIC().setRedirection(0, LAPIC::getAPICID(), 33);
    }
}
void PIT::stop()
{
    CPU::getInstance()->getIOAPIC().disableRedirection(0);
}
void PIT::sleep(uint64_t microseconds)
{
    size_t target = count + microseconds * frequency / 1e6;
    while (count < target);
}
void PIT::setFrequency(uint64_t frequency)
{
    this->frequency = frequency;
}
uint64_t PIT::getCount()
{
    return count;
}
uint64_t PIT::getFrequency()
{
    return frequency;
}
double PIT::getMicroseconds()
{
    return (double)count / frequency * 1e6;
}
void PIT::setInterruptHandler(InterruptHandler handler)
{
    this->handler = handler;
}