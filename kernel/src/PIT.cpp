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
        CPU::getInstance()->getCurrentCore().setInterruptHandler(0, [](CPURegisters* regs) {
            getInstance()->incrementCount();
        });
        CPU::getInstance()->getIOAPIC().setRedirection(0, LAPIC::getLAPIC().getID(), 32);
    }
}
void PIT::stop()
{
    CPU::getInstance()->getIOAPIC().disableRedirection(0);
}
void PIT::sleep(uint64_t nanoseconds)
{
    size_t target = count + nanoseconds * frequency / 1e9;
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
double PIT::getNanoseconds()
{
    return (double)count / frequency * 1e9;
}
void PIT::setInterruptHandler(InterruptHandler handler)
{
    this->handler = handler;
}