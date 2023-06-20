#include <PhoenixOS.hpp>
extern "C" void module_init()
{
    asm volatile ("out %%al, %%dx":: "a"('x'), "d"(0x3F8));
    memset((void*)0, 0, 0);
}