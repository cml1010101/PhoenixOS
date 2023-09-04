#include <Heap.hpp>
#include <CPU.hpp>
#define HEAP_MAGIC 0xBEEFCAFEBEEFCAFE
Heap* Heap::currentHeap;
Heap* Heap::kernelHeap;
Heap::Heap(size_t pages, bool kernel) : Heap(pages, kernel, CPU::getInstance()->getCurrentCore().getVirtualMemoryManager())
{
}
Heap::Heap(size_t pages, bool kernel, VirtualMemoryManager* vmm)
{
    heapStart = (HeapEntry*)vmm->allocate(pages, kernel ?
        VMM_PRESENT | VMM_READ_WRITE
        : VMM_PRESENT | VMM_READ_WRITE | VMM_USER);
    heapStart->free = 1;
    heapStart->size = (pages << 12) - sizeof(HeapEntry);
    heapStart->magic = HEAP_MAGIC;
    heapStart->next = NULL;
}
void* Heap::allocate(size_t length)
{
    HeapEntry* entry = heapStart;
    while (entry)
    {
        if (entry->magic != HEAP_MAGIC)
        {
            print(Logger::getInstance());
            Logger::getInstance()->panic("Corrupted heap\n");
        }
        if (entry->free)
        {
            if (entry->size > (length + sizeof(HeapEntry)))
            {
                entry->free = 0;
                HeapEntry* newEntry = (HeapEntry*)&((char*)&entry[1])[length];
                newEntry->free = 1;
                newEntry->size = entry->size - sizeof(HeapEntry) - length;
                newEntry->next = entry->next;
                newEntry->magic = HEAP_MAGIC;
                entry->size = length;
                entry->next = newEntry;
                return &entry[1];
            }
            else if (entry->size >= length)
            {
                entry->free = 0;
                return &entry[1];
            }
        }
        entry = entry->next;
    }
    return NULL;
}
void* Heap::reallocate(void* ptr, size_t newLength)
{
    HeapEntry* entry = (HeapEntry*)ptr - 1;
    if (!entry->next->free)
    {
        void* newPtr = allocate(newLength);
        memcpy(newPtr, ptr, entry->size);
        free(ptr);
        return newPtr;
    }
    size_t availableSize = entry->next->size + sizeof(HeapEntry) + entry->size;
    if (availableSize > (newLength + sizeof(HeapEntry)))
    {
        HeapEntry* newEntry = (HeapEntry*)&((char*)&entry[1])[newLength];
        newEntry->free = 1;
        newEntry->size = availableSize - sizeof(HeapEntry) - newLength;
        newEntry->next = entry->next->next;
        newEntry->magic = HEAP_MAGIC;
        entry->size = newLength;
        entry->next = newEntry;
        return &entry[1];
    }
    else if (availableSize >= newLength)
    {
        entry->size = newLength;
        entry->next = entry->next->next;
        return &entry[1];
    }
    void* newPtr = allocate(newLength);
    memcpy(newPtr, ptr, entry->size);
    free(ptr);
    return newPtr;
}
void Heap::free(void* ptr)
{
    HeapEntry* entry = (HeapEntry*)ptr - 1;
    entry->free = 1;
    clean();
}
void Heap::check()
{
    HeapEntry* entry = heapStart;
    while (entry)
    {
        if (entry->magic != HEAP_MAGIC)
        {
            print(Logger::getInstance());
            Logger::getInstance()->panic("Corrupted Heap\n");
        }
        entry = entry->next;
    }
}
void Heap::clean()
{
    HeapEntry* entry = heapStart;
    while (entry->next)
    {
        if (entry->magic != HEAP_MAGIC)
        {
            print(Logger::getInstance());
            Logger::getInstance()->panic("Corrupted Heap\n");
        }
        if (entry->free && entry->next->free)
        {
            entry->size += entry->next->size + sizeof(HeapEntry);
            entry->next = entry->next->next;
        }
        if (!entry->next) return;
        entry = entry->next;
    }
}
void Heap::print(Logger* logger)
{
    HeapEntry* entry = heapStart;
    while (entry)
    {
        logger->log("%d bytes at 0x%x: %s\n", entry->size, &entry[1], entry->free ? "Free" : "Taken");
        if (entry->magic != HEAP_MAGIC) break;
        entry = entry->next;
    }
}
extern "C" void* malloc(size_t size)
{
    return Heap::getCurrentHeap()->allocate(size);
}
extern "C" void* realloc(void* ptr, size_t size)
{
    return Heap::getCurrentHeap()->reallocate(ptr, size);
}
extern "C" void free(void* ptr)
{
    Heap::getCurrentHeap()->free(ptr);
}
void Heap::initialize()
{
    kernelHeap = (Heap*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocate(1, VMM_PRESENT | VMM_READ_WRITE);
    *kernelHeap = Heap(0x100, true);
    currentHeap = kernelHeap;
}
void* operator new(size_t size)
{
    return Heap::getCurrentHeap()->allocate(size);
}
void* operator new[](size_t size)
{
    return Heap::getCurrentHeap()->allocate(size);
}
void operator delete(void* ptr)
{
    Heap::getCurrentHeap()->free(ptr);
}
void operator delete[](void* ptr)
{
    Heap::getCurrentHeap()->free(ptr);
}
void operator delete(void* ptr, size_t size)
{
    (void)size;
    Heap::getCurrentHeap()->free(ptr);
}
void operator delete[](void* ptr, size_t size)
{
    (void)size;
    Heap::getCurrentHeap()->free(ptr);
}