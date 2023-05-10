#include <Scheduler.hpp>
#include <CPU.hpp>
Scheduler::Scheduler(size_t frequency)
{
    this->countsPerInterrupt = 1000000000 / frequency;
    this->activeThreadIdx = -1;
}
void Scheduler::reportISR(CPURegisters* regs)
{
    Logger::getInstance()->log("Unrecoverable error in thread %s. Terminating.\n", getActiveThread()->getName());
    getActiveThread()->terminated = true;
    asm volatile ("sti");
    for (;;);
}
void Scheduler::yield()
{
}
void Scheduler::yield(CPURegisters* registers)
{
}