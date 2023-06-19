#include <Scheduler.hpp>
#include <CPU.hpp>
Thread* Scheduler::kernelThread;
extern "C" void _kill()
{
    CPU::getInstance()->getCurrentCore().getScheduler()->getActiveThread()->kill();
    for (;;);
}
int nextThreadIdx = 0;
Thread::Thread(const char* name, void(*entryPoint)(), uint8_t ring)
{
    Logger::getInstance()->log("Creating thread '%s' that enters at 0x%x at ring %d\n", name, entryPoint, ring);
    this->ringLevel = ring;
    this->asleep = false;
    this->active = false;
    this->finished = false;
    this->joined = false;
    this->terminated = false;
    this->threadIdx = nextThreadIdx++;
    this->name = name;
    memset(&registers, 0, sizeof(registers));
    this->registers.rax = nextThreadIdx;
    this->registers.rip = (uint64_t)entryPoint;
    this->registers.cs = CPU::getInstance()->getSegment(ring, true);
    this->registers.ds = this->registers.es = this->registers.fs = this->registers.gs = this->registers.ss
        = CPU::getInstance()->getSegment(ring, false);
    asm volatile ("pushfq; pop %0": "=r"(registers.rflags));
    this->registers.rflags |= ring << 12;
    Logger::getInstance()->log("About to clone virtual memory manager\n");
    this->vmm = ringLevel > 0 ? VirtualMemoryManager::getKernelVirtualMemoryManager()->clone()
        : VirtualMemoryManager::getKernelVirtualMemoryManager();
    this->registers.rsp = (uint64_t)vmm->allocate(16, (ringLevel > 0) ? (VMM_PRESENT | VMM_READ_WRITE | VMM_USER)
        : (VMM_PRESENT | VMM_READ_WRITE)) - 8;
    *(uint64_t*)registers.rsp = (uint64_t)_kill;
    Logger::getInstance()->log("Created thread '%s'\n", name);
}
Scheduler::Scheduler(Timer* interruptTimer, Timer* clockTimer, bool kernel)
{
    this->queue = NULL;
    this->activeThread = NULL;
    this->numActiveThreads = 0;
    this->clockTimer = clockTimer;
    if (kernel)
    {
        Logger::getInstance()->log("Creating kernel thread\n");
        scheduleLocal(kernelThread = new Thread("kernel", (Runnable)&&finish));
        Logger::getInstance()->log("Scheduled kernel thread\n");
        Logger::getInstance()->log("&kernelThread = 0x%x\n", kernelThread);
        activeThread = kernelThread;
    }
    interruptTimer->setInterruptHandler([](CPURegisters* regs) {
        CPU::getInstance()->getCurrentCore().getScheduler()->yield(regs);
    });
finish:;
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
    asm volatile ("push %rax");
    asm volatile ("mov %%rbx, %%rax": "=a"(activeThread->registers.rbx));
    asm volatile ("mov %%rcx, %%rax": "=a"(activeThread->registers.rcx));
    asm volatile ("mov %%rdx, %%rax": "=a"(activeThread->registers.rdx));
    asm volatile ("mov %%rsi, %%rax": "=a"(activeThread->registers.rsi));
    asm volatile ("mov %%rdi, %%rax": "=a"(activeThread->registers.rdi));
    asm volatile ("mov %%rbp, %%rax": "=a"(activeThread->registers.rbp));
    asm volatile ("mov %%r8, %%rax": "=a"(activeThread->registers.r8));
    asm volatile ("mov %%r9, %%rax": "=a"(activeThread->registers.r9));
    asm volatile ("mov %%r10, %%rax": "=a"(activeThread->registers.r10));
    asm volatile ("mov %%r11, %%rax": "=a"(activeThread->registers.r11));
    asm volatile ("mov %%r12, %%rax": "=a"(activeThread->registers.r12));
    asm volatile ("mov %%r13, %%rax": "=a"(activeThread->registers.r13));
    asm volatile ("mov %%r14, %%rax": "=a"(activeThread->registers.r14));
    asm volatile ("mov %%r15, %%rax": "=a"(activeThread->registers.r15));
    asm volatile ("pushfq");
    asm volatile ("pop %%rax": "=a"(activeThread->registers.rflags));
    asm volatile ("pop %%rax": "=a"(activeThread->registers.rax));
    asm volatile ("mov %%rsp, %%rax": "=a"(activeThread->registers.rsp));
    activeThread->registers.rip = (uint64_t)&&ret;
find_thread:
    if (activeThread->next) activeThread = activeThread->next;
    else activeThread = queue;
    if (activeThread->terminated || activeThread->finished)
    {
        Thread* t = queue;
        while (t->next != activeThread)
            t = t->next;
        t->next = t->next->next;
        goto find_thread;
    }
    if ((activeThread->joined || activeThread->asleep) && activeThread == queue && activeThread->next == NULL)
        goto load_thread;
    if (activeThread->joined)
    {
        if (activeThread->joinTarget->finished || activeThread->joinTarget->terminated)
        {
            activeThread->joined = false;
        }
        else goto find_thread;
    }
    if (activeThread->asleep)
    {
        if (activeThread->sleepTarget <= clockTimer->getMicroseconds())
        {
            activeThread->asleep = false;
        }
        else goto find_thread;
    }
load_thread:
    CPU::getInstance()->getCurrentCore().setVirtualMemoryManager(activeThread->vmm);
    CPU::getInstance()->getCurrentCore().resetMMU();
    asm volatile ("push %%rax":: "a"(activeThread->registers.ss));
    asm volatile ("push %%rax":: "a"(activeThread->registers.rsp));
    asm volatile ("push %%rax":: "a"(activeThread->registers.rflags));
    asm volatile ("push %%rax":: "a"(activeThread->registers.cs));
    asm volatile ("push %%rax":: "a"(activeThread->registers.rip));
    asm volatile ("push %%rax":: "a"(activeThread->registers.rax));
    asm volatile ("mov %%rax, %%r15":: "a"(activeThread->registers.r15));
    asm volatile ("mov %%rax, %%r14":: "a"(activeThread->registers.r14));
    asm volatile ("mov %%rax, %%r13":: "a"(activeThread->registers.r13));
    asm volatile ("mov %%rax, %%r12":: "a"(activeThread->registers.r12));
    asm volatile ("mov %%rax, %%r11":: "a"(activeThread->registers.r11));
    asm volatile ("mov %%rax, %%r10":: "a"(activeThread->registers.r10));
    asm volatile ("mov %%rax, %%r9":: "a"(activeThread->registers.r9));
    asm volatile ("mov %%rax, %%r8":: "a"(activeThread->registers.r8));
    asm volatile ("mov %%rax, %%rbp":: "a"(activeThread->registers.rbp));
    asm volatile ("mov %%rax, %%rsi":: "a"(activeThread->registers.rsi));
    asm volatile ("mov %%rax, %%rdx":: "a"(activeThread->registers.rdx));
    asm volatile ("mov %%rax, %%rcx":: "a"(activeThread->registers.rcx));
    asm volatile ("mov %%rax, %%rbx":: "a"(activeThread->registers.rbx));
    asm volatile ("mov %%rax, %%ds":: "a"(activeThread->registers.ds));
    asm volatile ("mov %%rax, %%es":: "a"(activeThread->registers.es));
    asm volatile ("mov %%rax, %%fs":: "a"(activeThread->registers.fs));
    asm volatile ("mov %%rax, %%gs":: "a"(activeThread->registers.gs));
    asm volatile ("pop %rax");
    asm volatile ("iretq");
ret:;
}
void Scheduler::yield(CPURegisters* registers)
{
    memcpy(&activeThread->registers, registers, sizeof(CPURegisters));
find_thread:
    if (activeThread->next) activeThread = activeThread->next;
    else activeThread = queue;
    if ((activeThread->joined || activeThread->asleep) && activeThread == queue && activeThread->next == NULL)
        goto load_thread;
    if (activeThread->joined)
    {
        if (activeThread->joinTarget->finished || activeThread->joinTarget->terminated)
        {
            activeThread->joined = false;
        }
        else goto find_thread;
    }
    if (activeThread->asleep)
    {
        if (activeThread->sleepTarget <= clockTimer->getMicroseconds())
        {
            activeThread->asleep = false;
        }
        else goto find_thread;
    }
load_thread:
    CPU::getInstance()->getCurrentCore().setVirtualMemoryManager(activeThread->vmm);
    CPU::getInstance()->getCurrentCore().resetMMU();
    memcpy(registers, &activeThread->registers, sizeof(CPURegisters));
}
Thread* Scheduler::getThread(int idx)
{
    Thread* t = queue;
    while (t->getThreadIdx() != idx) t = t->next;
    return t;
}
void Scheduler::schedule(Thread* thread)
{
    size_t leastThreads = CPU::getInstance()->getCore(0).getScheduler()->getNumberActiveThreads();
    size_t leastThreadsIdx = 0;
    for (size_t i = 1; i < CPU::getInstance()->getNumberOfCores(); i++)
    {
        size_t threads = CPU::getInstance()->getCore(i).getScheduler()->getNumberActiveThreads();
        if (threads < leastThreads)
        {
            leastThreads = threads;
            leastThreadsIdx = i;
        }
    }
    CPU::getInstance()->getCore(leastThreadsIdx).getScheduler()->scheduleLocal(thread);
}