#include <PhoenixOS.hpp>
#include <PhysicalMemoryManager.hpp>
#include <VirtualMemoryManager.hpp>
#include <CPU.hpp>
#include <Heap.hpp>
#include <efi.h>
#include <efilib.h>
#include <QemuLogger.hpp>
#include <XSDT.hpp>
#include <Module.hpp>
struct BootData
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    void* acpi;
    void* memoryMap;
    size_t mapSize;
    size_t descriptorSize;
    size_t magic;
    uint64_t kernelPhys, kernelPages;
    struct Module
    {
        char moduleName[16];
        size_t address, pages;
    } modules[10];
};
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
extern "C" void kernel_main(BootData* data)
{
    callGlobalConstructors();
    QemuLogger logger = QemuLogger(0x3F8);
    logger.log("0x%x - 0x%x\n", (uint64_t)&__init_array_start, (uint64_t)&__init_array_end);
    logger.log("Magic: 0x%x\n", data->magic);
    Logger::setInstance(&logger);
    XSDT::loadXSDT((XSDP*)data->acpi);
    PhysicalMemoryManager::instance = PhysicalMemoryManager(data->memoryMap, data->mapSize, data->descriptorSize);
    VirtualMemoryManager::initialize();
    CPU::initialize();
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
    // Vector<Module*> modules;
    // modules.add(new Module("KERNEL", data->kernelPhys, data->kernelPages));
    // for (size_t i = 0; i < 10; i++)
    // {
    //     if (data->modules[i].moduleName[0])
    //     {
    //         Logger::getInstance()->log("%s at 0x%x, %d\n", data->modules[i].moduleName, data->modules[i].address,
    //             data[i].modules->pages);
    //         modules.add(new Module(data->modules[i].moduleName, data->modules[i].address, data->modules[i].pages));
    //     }
    // }
    // new (&ModuleLoader::instance) ModuleLoader(modules);
    for (;;);
}
void otherFunction()
{
    Logger::getInstance()->log("Hello from other function!\n");
}