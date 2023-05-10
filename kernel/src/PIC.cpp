#include <PIC.hpp>
#define PIC_REG_CMD1 0x20
#define PIC_REG_DATA1 0x21
#define PIC_REG_CMD2 0xA0
#define PIC_REG_DATA2 0xA1
void PIC::enable()
{
    uint8_t primaryMask = inb(PIC_REG_DATA1);
    uint8_t secondaryMask = inb(PIC_REG_DATA2);
    outb(PIC_REG_CMD1, 0x11);
    outb(PIC_REG_CMD2, 0x11);
    outb(PIC_REG_DATA1, 32);
    outb(PIC_REG_DATA2, 40);
    outb(PIC_REG_DATA1, 4);
    outb(PIC_REG_DATA2, 2);
    outb(PIC_REG_DATA1, 1);
    outb(PIC_REG_DATA2, 1);
    outb(PIC_REG_DATA1, primaryMask);
    outb(PIC_REG_DATA2, secondaryMask);
}
void PIC::maskIRQ(size_t irqNum)
{
    if (irqNum > 7)
    {
        outb(PIC_REG_DATA2, inb(PIC_REG_DATA2) | (1 << irqNum));
    }
    else
    {
        outb(PIC_REG_DATA1, inb(PIC_REG_DATA1) | (1 << irqNum));
    }
}
void PIC::unmaskIRQ(size_t irqNum)
{
    if (irqNum > 7)
    {
        outb(PIC_REG_DATA2, inb(PIC_REG_DATA2) & ~(1 << irqNum));
    }
    else
    {
        outb(PIC_REG_DATA1, inb(PIC_REG_DATA1) & ~(1 << irqNum));
    }
}
void PIC::disable()
{
    outb(PIC_REG_DATA1, 0xFF);
    outb(PIC_REG_DATA2, 0xFF);
}
void PIC::sendEOI(size_t irqNum)
{
    if (irqNum > 7)
    {
        outb(PIC_REG_CMD2, 0x20);
    }
    outb(PIC_REG_CMD1, 0x20);
}