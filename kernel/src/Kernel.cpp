#include <PhoenixOS.hpp>
#include <PhysicalMemoryManager.hpp>
#include <VirtualMemoryManager.hpp>
#include <CPU.hpp>
#include <Heap.hpp>
#include <efi.h>
#include <efilib.h>
#include <QemuLogger.hpp>
#include <XSDT.hpp>
struct BootData
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    void* acpi;
    void* memoryMap;
    size_t mapSize;
    size_t descriptorSize;
    size_t magic;
};
void __attribute__((constructor)) test()
{
    QemuLogger logger = QemuLogger(0x3F8);
    logger.log("Hello from Phoenix OS\n");
}
extern char __init_array_start;
extern char __init_array_end;
extern "C" void kernel_main(BootData* data)
{
    QemuLogger logger = QemuLogger(0x3F8);
    logger.log("0x%x - 0x%x\n", (uint64_t)&__init_array_start, (uint64_t)&__init_array_end);
    logger.log("Magic: 0x%x\n", data->magic);
    Logger::setInstance(&logger);
    XSDT::loadXSDT((XSDP*)data->acpi);
    PhysicalMemoryManager::instance = PhysicalMemoryManager(data->memoryMap, data->mapSize, data->descriptorSize);
    VirtualMemoryManager::initialize();
    CPU::initialize();
    Heap::initialize();
    PIT pit = PIT();
    pit.setFrequency(1e+5);
    pit.start();
    CPU::getInstance()->initializeScheduling();
    for (;;);
}