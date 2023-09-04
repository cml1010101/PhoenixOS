#ifndef PHOENIXOS_HPP
#define PHOENIXOS_HPP
#include <stddef.h>
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uintptr_t;
typedef signed int int32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int64_t;
typedef void(*Runnable)();
extern "C" void* malloc(size_t size);
extern "C" void* realloc(void* ptr, size_t size);
extern "C" void free(void* ptr);
extern "C" void memset(void* dest, int val, size_t length);
extern "C" void memcpy(void* dest, const void* src, size_t length);
extern "C" int memcmp(const void* a, const void* b, size_t length);
extern "C" int strcmp(const char* a, const char* b);
extern "C" size_t strlen(const char* a);
extern "C" const char* itoa(int n, int base);
extern "C" const char* uitoa(uint32_t n, int base);
extern "C" const char* ltoa(long n, int base);
extern "C" const char* ultoa(size_t n, int base);
inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile ("in %%dx, %%al": "=a"(data): "d"(port));
    return data;
}
inline void outb(uint16_t port, uint8_t data)
{
    asm volatile ("out %%al, %%dx":: "a"(data), "d"(port));
}
inline uint32_t inl(uint16_t port)
{
    uint32_t data;
    asm volatile ("in %%dx, %%eax": "=a"(data): "d"(port));
    return data;
}
inline void outl(uint16_t port, uint32_t data)
{
    asm volatile ("out %%eax, %%dx":: "a"(data), "d"(port));
}
inline uint64_t getMSR(uint32_t msr)
{
    uint32_t low, high;
    asm volatile ("rdmsr": "=a"(low), "=d"(high): "c"(msr));
    return ((uint64_t)high << 32) | (uint64_t)low;
}
inline void setMSR(uint32_t msr, uint64_t val)
{
    asm volatile ("wrmsr":: "a"(val & 0xFFFFFFFF), "d"(val >> 32), "c"(msr));
}
struct __attribute__((packed)) SystemPointer
{
    uint16_t limit;
    uint64_t base;
};
struct CPURegisters
{
    size_t gs, es, fs, ds;
    size_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx, rax;
    size_t num, code;
    size_t rip, cs, rflags, rsp, ss;
};
class Logger
{
protected:
    static Logger* instance;
public:
    virtual void log(const char* frmt, ...)
    {
        (void)frmt;
    }
    virtual void panic(const char* frmt, ...)
    {
        (void)frmt;
    }
    inline static Logger* getInstance()
    {
        return instance;
    }
    inline static void setInstance(Logger* logger)
    {
        instance = logger;
    }
};
class Spinlock
{
private:
    volatile bool acquired;
public:
    inline Spinlock()
    {
        acquired = false;
    }
    inline void acquire()
    {
        while (acquired);
        acquired = true;
    }
    inline void release()
    {
        acquired = false;
    }
};
template<typename T>
class Vector
{
private:
    T* arr;
    size_t len;
public:
    inline Vector()
    {
        arr = NULL;
        len = 0;
    }
    inline T remove(size_t i)
    {
        T t = arr[i];
        len--;
        if (len == 0)
        {
            free(arr);
            arr = NULL;
            return t;
        }
        else
        {
            memcpy(&arr[i], &arr[i + 1], (len - i) * sizeof(T));
            arr = (T*)realloc(arr, len * sizeof(T));
            return t;
        }
    }
    inline void insert(size_t i, T t)
    {
        len++;
        if (arr == NULL)
        {
            arr = (T*)malloc(len * sizeof(T));
        }
        else
        {
            arr = (T*)realloc(arr, len * sizeof(T));
            memcpy(&arr[i + 1], &arr[i], (len - i) * sizeof(T));
        }
        arr[i] = t;
    }
    inline size_t add(T t)
    {
        len++;
        if (arr == NULL)
        {
            arr = (T*)malloc(len * sizeof(T));
        }
        else
        {
            arr = (T*)realloc(arr, len * sizeof(T));
        }
        arr[len - 1] = t;
        return len - 1;
    }
    inline T& operator[](size_t i)
    {
        return arr[i];
    }
    inline T operator[](size_t i) const
    {
        return arr[i];
    }
    inline size_t size() const
    {
        return len;
    }
};
template<typename T>
class LinkedList
{
public:
    struct LinkedListEntry
    {
        T t;
        LinkedListEntry* next;
    };
private:
    LinkedListEntry* firstEntry;
    size_t len;
public:
    inline LinkedList()
    {
        firstEntry = NULL;
        len = 0;
    }
    inline T remove(size_t i)
    {
        LinkedListEntry* entry = firstEntry;
        LinkedListEntry* previous = NULL;
        for (size_t j = 0; j < i; j++)
        {
            previous = entry;
            entry = entry->next;
        }
        if (previous != NULL) previous->next = entry->next;
        else firstEntry = entry->next;
        T t = entry->t;
        free(entry);
        len--;
        return t;
    }
    inline void insert(size_t i, T t)
    {
        if (len > 0)
        {
            LinkedListEntry* entry = firstEntry;
            LinkedListEntry* previous = NULL;
            for (size_t j = 0; j < i; j++)
            {
                previous = entry;
                entry = entry->next;
            }
            LinkedListEntry* newEntry = (LinkedListEntry*)malloc(sizeof(LinkedListEntry));
            newEntry->t = t;
            if (previous != NULL) previous->next = newEntry;
            else firstEntry = newEntry;
            newEntry->next = entry;
        }
        else
        {
            firstEntry = (LinkedListEntry*)malloc(sizeof(LinkedListEntry));
            firstEntry->t = t;
            firstEntry->next = NULL;
        }
        len++;
    }
    inline size_t add(T t)
    {
        len++;
        if (firstEntry == NULL)
        {
            firstEntry = (LinkedListEntry*)malloc(len * sizeof(T));
            firstEntry->t = t;
            firstEntry->next = NULL;
        }
        else
        {
            LinkedListEntry* entry = firstEntry;
            while (entry->next)
            {
                entry = entry->next;
            }
            LinkedListEntry* newEntry = (LinkedListEntry*)malloc(sizeof(LinkedListEntry));
            newEntry->t = t;
            entry->next = newEntry;
            newEntry->next = NULL;
        }
        return len - 1;
    }
    inline T& operator[](size_t i)
    {
        LinkedListEntry* entry = firstEntry;
        for (size_t j = 0; j < i; j++)
        {
            entry = entry->next;
        }
        return entry->t;
    }
    inline T operator[](size_t i) const
    {
        LinkedListEntry* entry = firstEntry;
        for (size_t j = 0; j < i; j++)
        {
            entry = entry->next;
        }
        return entry->t;
    }
    inline size_t size() const
    {
        return len;
    }
};
class FileSystem;
class File
{
private:
    friend class FileSystem;
    void* handle;
    FileSystem* fileSystem;
    File(void* handle, FileSystem* fileSystem);
public:
    File() = default;
    bool isFolder();
    size_t getSize();
    void read(void* dest, size_t length);
    void write(const void* src, size_t length);
};
class FileSystem
{
protected:
    friend class File;
    inline File createFile(void* handle)
    {
        return File(handle, this);
    }
    inline void* getFileHandle(File file)
    {
        return file.handle;
    }
    virtual bool isFolder(File file) = 0;
    virtual size_t getSize(File file) = 0;
    virtual void read(File file, void* dest, size_t length) = 0;
    virtual void write(File file, const void* src, size_t length) = 0;
public:
    virtual File openFile(const char* path) = 0;
};
inline void* operator new(size_t, void* p) throw()
{
    return p;
}
inline void* operator new[](size_t, void* p) throw()
{
    return p;
}
inline void operator delete(void*, void*) throw()
{
}
inline void operator delete[](void*, void*) throw()
{
}
#endif