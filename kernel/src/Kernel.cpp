#include <PhoenixOS.hpp>
#include <PhysicalMemoryManager.hpp>
#include <VirtualMemoryManager.hpp>
#include <CPU.hpp>
#include <Heap.hpp>
#include <SwiftBoot.hpp>
#include <QemuLogger.hpp>
#include <XSDT.hpp>
#include <Module.hpp>
void __attribute__((constructor)) test()
{
    QemuLogger logger = QemuLogger(0x3F8);
    logger.log("Hello from Phoenix OS\n");
}
extern char __init_array_start;
extern char __init_array_end;
extern char start_ctors, end_ctors;
void callGlobalConstructors()
{
    Runnable* constructors = (Runnable*)&__init_array_start;
    size_t numConstructors = ((uint64_t)&__init_array_end - (uint64_t)&__init_array_start) / sizeof(Runnable);
    for (size_t i = 0; i < numConstructors; i++)
    {
        constructors[i]();
    }
}
void otherFunction();
extern "C" void kernel_main(swiftboot::BootInfo* data)
{
    callGlobalConstructors();
    QemuLogger logger = QemuLogger(0x3F8);
    logger.log("0x%x - 0x%x\n", (uint64_t)&__init_array_start, (uint64_t)&__init_array_end);
    Logger::setInstance(&logger);
    XSDT::loadXSDT((XSDP*)data->acpi);
    logger.log("Initializing Physical Memory Manager\n");
    PhysicalMemoryManager::instance = PhysicalMemoryManager(data->map);
    logger.log("Initializing Virtual Memory Manager\n");
    VirtualMemoryManager::initialize();
    logger.log("Initializing CPU\n");
    CPU::initialize();
    logger.log("Initializing Heap\n");
    Heap::initialize();
    int* dat = new int[5];
    Logger::getInstance()->log("new int[5] = 0x%x\n", dat);
    delete[] dat;
    Logger::getInstance()->log("Initializing PIT\n");
    PIT pit = PIT();
    pit.setFrequency(1e+5);
    pit.start();
    Logger::getInstance()->log("Started PIT\n");
    Logger::getInstance()->log("Starting scheduling...\n");
    asm volatile ("sti");
    CPU::getInstance()->initializeScheduling();
    Logger::getInstance()->log("Scheduling!!!\n");
    Logger::getInstance()->log("Hello from kernel function!\n");
    Scheduler::schedule(new Thread("otherFunction", otherFunction));
    Logger::getInstance()->log("Hello from kernel main\n");
    for (;;);
}
void otherFunction()
{
    Logger::getInstance()->log("Hello from other function!\n");
}