#include <QemuLogger.hpp>
#include <stdarg.h>
Logger* Logger::instance;
void QemuLogger::putc(char c)
{
    while (!(inb(ioBase + 5) & 0x20));
    outb(ioBase, c);
}
void QemuLogger::print(const char* str)
{
    size_t i = 0;
    while (str[i])
    {
        putc(str[i++]);
    }
}
void QemuLogger::log(const char* frmt, ...)
{
    // static Spinlock lock;
    // lock.acquire();
    va_list ls;
    va_start(ls, frmt);
    size_t i = 0;
    while (frmt[i])
    {
        if (frmt[i] == '%')
        {
            i++;
            if (frmt[i] == 'u')
            {
                print(ultoa(va_arg(ls, uint64_t), 10));
            }
            else if (frmt[i] == 'x')
            {
                print(ultoa(va_arg(ls, uint64_t), 16));
            }
            else if (frmt[i] == 'd')
            {
                print(itoa(va_arg(ls, int), 10));
            }
            else if (frmt[i] == 's')
            {
                print((const char*)va_arg(ls, size_t));
            }
            else if (frmt[i] == 'c')
            {
                putc(va_arg(ls, int));
            }
            else if (frmt[i] == '%')
            {
                putc('%');
            }
        }
        else putc(frmt[i]);
        i++;
    }
    // lock.release();
}
void QemuLogger::panic(const char* frmt, ...)
{
    // static Spinlock lock;
    // lock.acquire();
    va_list ls;
    va_start(ls, frmt);
    size_t i = 0;
    while (frmt[i])
    {
        if (frmt[i] == '%')
        {
            i++;
            if (frmt[i] == 'u')
            {
                print(ultoa(va_arg(ls, uint64_t), 10));
            }
            else if (frmt[i] == 'x')
            {
                print(ultoa(va_arg(ls, uint64_t), 16));
            }
            else if (frmt[i] == 'd')
            {
                print(itoa(va_arg(ls, int), 10));
            }
            else if (frmt[i] == 's')
            {
                print((const char*)va_arg(ls, size_t));
            }
            else if (frmt[i] == 'c')
            {
                putc(va_arg(ls, int));
            }
            else if (frmt[i] == '%')
            {
                putc('%');
            }
        }
        else putc(frmt[i]);
        i++;
    }
    // lock.release();
    for (;;);
}