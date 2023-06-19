#ifndef MODULE_HPP
#define MODULE_HPP
#include <PhoenixOS.hpp>
class Module
{
private:
    char* name;
    uint64_t physAddress;
    uint64_t virtAddress;
    uint64_t pages;
    struct Symbol
    {
        char* name;
        size_t value;
    };
    Vector<Symbol> symbols;
public:
    Module() = default;
    Module(char* name, uint64_t physAddress, uint64_t pages);
    void loadSymbols();
    size_t getSymbol(const char* name);
    void start();
};
class ModuleLoader
{
private:
    Vector<Module*> loadedModules;
public:
    static ModuleLoader instance;
    ModuleLoader() = default;
    ModuleLoader(Vector<Module*> modulesToLoad);
};
#endif