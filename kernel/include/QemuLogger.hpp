#ifndef QEMULOGGER_HPP
#define QEMULOGGER_HPP
#include <PhoenixOS.hpp>
class QemuLogger : public Logger
{
private:
    uint16_t ioBase;
public:
    QemuLogger() = default;
    inline QemuLogger(uint16_t ioBase)
    {
        this->ioBase = ioBase;
        outb(ioBase + 1, 0);
        outb(ioBase + 3, 0x80);
        outb(ioBase, 3);
        outb(ioBase + 1, 0);
        outb(ioBase + 3, 3);
        outb(ioBase + 2, 0xC7);
        outb(ioBase + 4, 0xB);
        outb(ioBase + 4, 0x1E);
        outb(ioBase, 0xAE);
        if (inb(ioBase) != 0xAE)
        {
            return;
        }
        outb(ioBase + 4, 0xF);
    }
    void putc(char c);
    void print(const char* str);
    void log(const char* frmt, ...) override;
    void panic(const char* frmt, ...) override;
};
#endif