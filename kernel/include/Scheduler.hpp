#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <PhoenixOS.hpp>
class Thread
{
private:
    friend class Scheduler;
    CPURegisters registers;
    const char* name;
    bool active, terminated;
    uint8_t ringLevel;
    int threadIdx;
    VirtualMemoryManager* vmm;
public:
    Thread(void(*entryPoint)());
    inline bool isActive()
    {
        return active;
    }
    inline bool isTerminated()
    {
        return terminated;
    }
    void join();
    inline const char* getName()
    {
        return name;
    }
    inline uint8_t getRingLevel()
    {
        return ringLevel;
    }
    inline int getThreadIdx()
    {
        return threadIdx;
    }
};
class Scheduler
{
private:
    LinkedList<Thread*> threads;
    int activeThreadIdx;
    size_t countsPerInterrupt;
    static Thread* kernelThread;
public:
    Scheduler() = default;
    Scheduler(size_t frequency);
    inline Thread* getActiveThread()
    {
        return (activeThreadIdx != -1) ? threads[activeThreadIdx] : NULL;
    }
    inline void schedule(Thread* thread)
    {
        threads.add(thread);
    }
    void reportISR(CPURegisters* regs);
    void yield();
    void yield(CPURegisters* registers);
    Thread* getThread(int threadIdx);
    static inline Thread* getKernelThread()
    {
        return kernelThread;
    }
};
#endif