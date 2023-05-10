#ifndef PIC_HPP
#define PIC_HPP
#include <PhoenixOS.hpp>
class PIC
{
public:
    static void enable();
    static void maskIRQ(size_t irqNum);
    static void unmaskIRQ(size_t irqNum);
    static void disable();
    static void sendEOI(size_t irqNum);
};
#endif