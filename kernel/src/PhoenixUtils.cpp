#include <PhoenixOS.hpp>
#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif
extern "C" void memcpy(void* dest, const void* src, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        ((char*)dest)[i] = ((const char*)src)[i];
    }
}
extern "C" int memcmp(const void* a, const void* b, size_t len)
{
    int flag = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (flag = (((const char*)a)[i] - ((const char*)b)[i]))
        {
            return flag;
        }
    }
    return 0;
}
extern "C" int strcmp(const char* a, const char* b)
{
    int flag = 0;
    size_t i = 0;
    while (a[i] && b[i])
    {
        if (flag = (((const char*)a)[i] - ((const char*)b)[i]))
        {
            return flag;
        }
        i++;
    }
    return a[i] - b[i];
}
extern "C" size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}
extern "C" void memset(void* dest, int val, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        ((char*)dest)[i] = val;
    }
}
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;
extern "C" void __stack_chk_fail(void)
{
    Logger::getInstance()->panic("Stack smashing detected...\n");
}
extern "C" void swap(char* a, char* b)
{
    char c = *a;
    *a = *b;
    *b = c;
}
extern "C" void reverse(char* str, int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap(str+start, str + end);
        start++;
        end--;
    }
}
char itoa_buffer[50];
extern "C" const char* itoa(int num, int base)
{
    char* str;
    int i = 0;
    int isNegative = 0;
    if (num == 0)
    {
        str = itoa_buffer;
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    size_t len = 0;
    if (num < 0 && base == 10)
    {
        len = 1;
        isNegative = 1;
        num = -num;
    }
    int n = num;
    while (n != 0)
    {
        len++;
        n = n / base;
    }
    str = itoa_buffer;
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
        num = num / base;
    }
    if (isNegative)
        str[i++] = '-';
    str[i] = '\0';
    reverse(str, i);
    return str;
}
char uitoa_buffer[50];
extern "C" const char* uitoa(uint32_t num, int base)
{
    char* str;
    int i = 0;
    if (num == 0)
    {
        str = uitoa_buffer;
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    size_t len = 0;
    size_t n = num;
    while (n != 0)
    {
        len++;
        n = n / base;
    }
    str = uitoa_buffer;
    while (num != 0)
    {
        size_t rem = num % base;
        str[i++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
        num = num / base;
    }
    str[i] = '\0';
    reverse(str, i);
    return str;
}
char ltoa_buffer[50];
extern "C" const char* ltoa(long num, int base)
{
    char* str;
    int i = 0;
    int isNegative = 0;
    if (num == 0)
    {
        str = ltoa_buffer;
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    size_t len = 0;
    if (num < 0 && base == 10)
    {
        len = 1;
        isNegative = 1;
        num = -num;
    }
    long n = num;
    while (n != 0)
    {
        len++;
        n = n / base;
    }
    str = ltoa_buffer;
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
        num = num / base;
    }
    if (isNegative)
        str[i++] = '-';
    str[i] = '\0';
    reverse(str, i);
    return str;
}
char ultoa_buffer[50];
extern "C" const char* ultoa(uint64_t num, int base)
{
    char* str;
    int i = 0;
    if (num == 0)
    {
        str = ultoa_buffer;
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    size_t len = 0;
    size_t n = num;
    while (n != 0)
    {
        len++;
        n = n / base;
    }
    str = ultoa_buffer;
    while (num != 0)
    {
        size_t rem = num % base;
        str[i++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
        num = num / base;
    }
    str[i] = '\0';
    reverse(str, i);
    return str;
}
extern "C" void __cxa_pure_virtual()
{
    Logger::getInstance()->panic("Pure Virtual Function Called!!!\n");
}
namespace __cxxabiv1 
{
	__extension__ typedef int __guard __attribute__((mode(__DI__)));
	extern "C" int __cxa_guard_acquire (__guard *);
	extern "C" void __cxa_guard_release (__guard *);
	extern "C" void __cxa_guard_abort (__guard *);
	extern "C" int __cxa_guard_acquire (__guard *g) 
	{
		return !*(char *)(g);
	}
	extern "C" void __cxa_guard_release (__guard *g)
	{
		*(char *)g = 1;
	}
	extern "C" void __cxa_guard_abort (__guard *)
	{
	}
}