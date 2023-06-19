#ifndef HEAP_HPP
#define HEAP_HPP
#include <PhoenixOS.hpp>
#include <VirtualMemoryManager.hpp>
class Heap
{
private:
    struct HeapEntry
    {
        uint64_t free: 1;
        uint64_t size: 63;
        HeapEntry* next;
        uint64_t magic;
    };
    HeapEntry* heapStart;
    static Heap* kernelHeap;
    static Heap* currentHeap;
public:
    Heap() = default;
    Heap(uint64_t pages, bool kernel);
    Heap(uint64_t pages, bool kernel, VirtualMemoryManager* vmm);
    static inline Heap* getKernelHeap()
    {
        return kernelHeap;
    }
    static inline void setKernelHeap(Heap* heap)
    {
        kernelHeap = heap;
    }
    static inline Heap* getCurrentHeap()
    {
        return currentHeap;
    }
    static inline void setCurrentHeap(Heap* heap)
    {
        currentHeap = heap;
    }
    void* allocate(size_t length);
    void* reallocate(void* ptr, size_t length);
    void free(void* ptr);
    void check();
    void clean();
    void print(Logger* logger);
    static void initialize();
};
#endif