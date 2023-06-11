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
    size_t alignedLength = length;
    if (length & 7)
    {
        alignedLength &= ~7;
        alignedLength += 8;
    }
    HeapEntry* entry = heapStart;
    while (entry)
    {
        if (entry->free)
        {
            if (entry->size > (alignedLength + sizeof(HeapEntry)))
            {
                entry->free = 0;
                HeapEntry* newEntry = (HeapEntry*)&((char*)&entry[1])[alignedLength];
                newEntry->free = 1;
                newEntry->size = entry->size - sizeof(HeapEntry) - alignedLength;
                newEntry->next = entry->next;
                newEntry->magic = HEAP_MAGIC;
                entry->size = length;
                entry->next = newEntry;
                return &entry[1];
            }
            else if (entry->size >= alignedLength)
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
    if (!entry->next)
    {
        void* newPtr = allocate(newLength);
        memcpy(newPtr, ptr, entry->size);
        free(ptr);
        return newPtr;
    }
    if (!entry->next->free)
    {
        void* newPtr = allocate(newLength);
        memcpy(newPtr, ptr, entry->size);
        free(ptr);
        return newPtr;
    }
    size_t alignedLength = newLength;
    if (alignedLength & 7)
    {
        alignedLength &= ~7;
        alignedLength += 8;
    }
    size_t availableSize = entry->next->size + sizeof(HeapEntry) + ((uint64_t)entry->next - (uint64_t)&entry[1]);
    if (availableSize > (alignedLength + sizeof(HeapEntry)))
    {
        HeapEntry* newEntry = (HeapEntry*)&((char*)&entry[1])[alignedLength];
        newEntry->free = 1;
        newEntry->size = entry->size - sizeof(HeapEntry) - alignedLength;
        newEntry->next = entry->next;
        newEntry->magic = HEAP_MAGIC;
        entry->size = newLength;
        entry->next = newEntry;
        return &entry[1];
    }
    else if (availableSize >= alignedLength)
    {
        entry->size = newLength;
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
            Logger::getInstance()->panic("Corrupted Heap");
        }
        entry = entry->next;
    }
}
void Heap::clean()
{
    HeapEntry* entry = heapStart;
    HeapEntry* previous = NULL;
    while (entry)
    {
        if (entry->magic != HEAP_MAGIC)
        {
            Logger::getInstance()->panic("Corrupted Heap");
        }
        if (previous == NULL && !entry->free)
        {
            previous = entry;
            entry = entry->next;
            continue;
        }
        if (previous->free)
        {
            if (entry->next)
            {
                previous->next = entry->next;
                previous->size = (uint64_t)entry->next - (uint64_t)&previous[1];
                entry = entry->next;
                continue;
            }
            else
            {
                previous->size += entry->size + sizeof(HeapEntry);
                previous->next = NULL;
                entry = NULL;
                continue;
            }
        }
        else
        {
            uint64_t actualSize = (uint64_t)entry - (uint64_t)&previous[1];
            if (actualSize - previous->size >= 8)
            {
                size_t shift = (actualSize - previous->size) & ~0x7;
                HeapEntry* newEntry = (HeapEntry*)((char*)entry - shift);
                HeapEntry* next = entry->next;
                size_t newSize = shift + entry->size;
                newEntry->free = 1;
                newEntry->magic = HEAP_MAGIC;
                newEntry->next = next;
                newEntry->size = newSize;
            }
        }
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
    *kernelHeap = Heap(0x1000, true);
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