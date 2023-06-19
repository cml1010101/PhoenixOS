#include <PhoenixOS.hpp>
extern "C" void module_init()
{
    Logger::getInstance()->log("Hello from graphics module\n");
}