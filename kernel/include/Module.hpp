#ifndef MODULE_HPP
#define MODULE_HPP
#include <PhoenixOS.hpp>
#include <elf.h>
class Module
{
private:
    char* name;
    uint64_t physAddress;
    uint64_t virtAddress;
    uint64_t pages;
    Elf64_Sym* sym;
    char* str;
    struct Symbol
    {
        char* name;
        size_t value;
    };
    Vector<Symbol> symbols;
public:
    Module() = default;
    Module(char* name, uint64_t physAddress, uint64_t pages);
    uint64_t getSymbolValue(size_t table, size_t idx);
    size_t getSymbol(const char* name);
    void load(bool loadProg);
    void start();
    inline const char* getName()
    {
        return name;
    }
};
class ModuleLoader
{
private:
    friend class Module;
    Vector<Module*> loadedModules;
public:
    static ModuleLoader instance;
    ModuleLoader() = default;
    ModuleLoader(Vector<Module*> modulesToLoad);
    size_t getSymbol(const char* name);
};
#endif