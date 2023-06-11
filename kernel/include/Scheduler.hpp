#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <PhoenixOS.hpp>
#include <VirtualMemoryManager.hpp>
#include <Timer.hpp>
class Thread
{
private:
    friend class Scheduler;
    CPURegisters registers;
    const char* name;
    bool active, terminated, joined, asleep, finished;
    uint64_t sleepTarget;
    Thread* joinTarget;
    uint8_t ringLevel;
    int threadIdx;
    VirtualMemoryManager* vmm;
    Thread* next;
public:
    Thread(const char* name, void(*entryPoint)(), uint8_t ring = 0);
    inline bool isActive()
    {
        return active;
    }
    inline bool isTerminated()
    {
        return terminated;
    }
    void join(Thread* thread)
    {
        joinTarget = thread;
        joined = true;
    }
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
    inline void kill()
    {
        finished = true;
    }
};
class Scheduler
{
private:
    Thread* queue;
    Thread* activeThread;
    size_t numActiveThreads;
    Timer* clockTimer;
    static Thread* kernelThread;
public:
    Scheduler() = default;
    Scheduler(Timer* interruptTimer, Timer* clockTimer, bool kernel);
    inline Thread* getActiveThread()
    {
        return activeThread;
    }
    inline size_t getNumberActiveThreads()
    {
        return numActiveThreads;
    }
    inline void scheduleLocal(Thread* thread)
    {
        if (queue == NULL) queue = thread;
        else
        {
            while (queue->next)
            {
                queue = queue->next;
            }
            queue->next = thread;
        }
        thread->active = true;
        numActiveThreads++;
    }
    void reportISR(CPURegisters* regs);
    void yield();
    void yield(CPURegisters* registers);
    Thread* getThread(int threadIdx);
    static inline Thread* getKernelThread()
    {
        return kernelThread;
    }
    static void schedule(Thread* thread);
};
#endif