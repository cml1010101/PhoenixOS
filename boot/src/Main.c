#include <stdint.h>
#include <stddef.h>
#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <memory.h>
#define TARGET_WIDTH 1920
#define TARGET_HEIGHT 1080
typedef struct
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    void* acpi;
    void* memoryMap;
    size_t mapSize;
    size_t descriptorSize;
} BootData;
typedef void(*KERNEL_MAIN)(BootData);
void __panic(EFI_STATUS code, int line)
{
    CHAR16 codeStr[48];
    StatusToString(codeStr, code);
    Print(L"Error: %s at line %d\n", codeStr, line);
    for (;;);
}
#define PANIC(code) __panic(code, __LINE__)
EFI_FILE_HANDLE getVolume(EFI_HANDLE ImageHandle)
{
    EFI_STATUS status;
    EFI_LOADED_IMAGE* loadedImage = NULL;
    EFI_GUID loadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE* ioInterface = NULL;
    EFI_GUID fileSystemGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE volume;
    status = uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, 
        &loadedImageProtocolGuid, &loadedImage);
    if (status) PANIC(status);
    status = uefi_call_wrapper(BS->HandleProtocol, 3, loadedImage->DeviceHandle,
        &fileSystemGuid, &ioInterface);
    if (status) PANIC(status);
    status = uefi_call_wrapper(ioInterface->OpenVolume, 2, ioInterface, &volume);
    if (status) PANIC(status);
    return volume;
}
UINT64 getFileSize(EFI_FILE_HANDLE file)
{
    UINT64 size;
    EFI_FILE_INFO* fileInfo = LibFileInfo(file);
    size = fileInfo->FileSize;
    FreePool(fileInfo);
    return size;
}
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS status;
    InitializeLib(ImageHandle, SystemTable);
    Print(L"Welcome to the Smart OS.\n");
    status = SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    if (status) PANIC(status);
    Print(L"Loading filesystem.\n");
    EFI_FILE_HANDLE volume = getVolume(ImageHandle);
    Print(L"Loading kernel.\n");
    CHAR16* kernelPath = L"KERNEL.ELF";
    EFI_FILE_HANDLE kernelHandle;
    status = uefi_call_wrapper(volume->Open, 5, volume, &kernelHandle,
        kernelPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY, EFI_FILE_HIDDEN, EFI_FILE_SYSTEM);
    if (status) PANIC(status);
    UINT64 kernelSize = getFileSize(kernelHandle);
    void* kernelBuffer = AllocatePool(kernelSize);
    status = uefi_call_wrapper(kernelHandle->Read, 3, kernelHandle, &kernelSize, kernelBuffer);
    if (status) PANIC(status);
    status = uefi_call_wrapper(kernelHandle->Close, 1, kernelHandle);
    if (status) PANIC(status);
    Elf64_Ehdr* kernelHeader = (Elf64_Ehdr*)kernelBuffer;
    Print(L"Interpreting kernel header.\n");
    Elf64_Phdr* programHeaders = (Elf64_Phdr*)((CHAR8*)kernelBuffer + kernelHeader->e_phoff);
    Elf64_Addr highest = 0, lowest = (uint64_t)-1;
    for (size_t i = 0; i < kernelHeader->e_phnum; i++)
    {
        if (programHeaders[i].p_type == PT_LOAD)
        {
            uint64_t chunkEnd = programHeaders[i].p_vaddr + programHeaders[i].p_memsz;
            if (chunkEnd > highest) highest = chunkEnd;
            if (programHeaders[i].p_vaddr < lowest) lowest = programHeaders[i].p_vaddr;
        }
    }
    size_t kernelAllocSize = highest - lowest;
    size_t kernelPages = (kernelAllocSize + EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE;
    size_t kernelRegion = lowest;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderCode,
        kernelPages, &kernelRegion);
    if (status) PANIC(status);
    if (kernelRegion != lowest) PANIC(-1);
    for (size_t i = 0; i < kernelHeader->e_phnum; i++)
    {
        if (programHeaders[i].p_type == PT_LOAD)
        {
            memcpy(programHeaders[i].p_vaddr, 
                (CHAR8*)kernelBuffer + programHeaders[i].p_offset, programHeaders[i].p_filesz);
            memset((void*)(programHeaders[i].p_vaddr + programHeaders[i].p_filesz), 0,
                programHeaders[i].p_memsz - programHeaders[i].p_filesz);
        }
    }
    KERNEL_MAIN kernelMain = (KERNEL_MAIN)kernelHeader->e_entry;
    FreePool(kernelBuffer);
    Print(L"Loading ACPI table.\n");
    void* acpi;
    EFI_GUID acpiGuid = ACPI_20_TABLE_GUID;
    for (size_t i = 0; i < SystemTable->NumberOfTableEntries; i++)
    {
        if (CompareGuid(&acpiGuid, &SystemTable->ConfigurationTable[i].VendorGuid) == 0)
        {
            acpi = SystemTable->ConfigurationTable[i].VendorTable;
            break;
        }
    }
    Print(L"Loading graphics.\n");
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* modeInfo;
    size_t modeInfoSize;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, &gop);
    if (status) PANIC(status);
    for (size_t i = 0; i < gop->Mode->MaxMode; i++)
    {
        status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &modeInfoSize, &modeInfo);
        if (status) PANIC(status);
        if (modeInfo->HorizontalResolution == TARGET_WIDTH &&
            modeInfo->VerticalResolution == TARGET_HEIGHT)
        {
            status = uefi_call_wrapper(gop->SetMode, 2, gop, i);
            if (status) PANIC(status);
            break;
        }
    }
    Print(L"Loading memory map.\n");
    size_t numEntries, mapKey, memoryMapSize, descriptorSize, descriptorVersion;
    void* map;
    map = LibMemoryMap(&numEntries, &mapKey, &descriptorSize, &descriptorVersion);
    memoryMapSize = descriptorSize * numEntries;
    Print(L"Entering kernel.\n");
    BootData data;
    data.gop = gop;
    data.acpi = acpi;
    data.memoryMap = map;
    data.mapSize = memoryMapSize;
    data.descriptorSize = descriptorSize;
    status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, mapKey);
    kernelMain(data);
    return -1;
}