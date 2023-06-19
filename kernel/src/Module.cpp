#include <Module.hpp>
#include <VirtualMemoryManager.hpp>
#include "../include/elf.h"
#include <Scheduler.hpp>
#include <Heap.hpp>
Module::Module(char* name, uint64_t physAddress, uint64_t pages)
{
    this->name = name;
    this->physAddress = physAddress;
    this->pages = pages;
    virtAddress = VirtualMemoryManager::getKernelVirtualMemoryManager()->allocateAddress(pages);
    VirtualMemoryManager::getKernelVirtualMemoryManager()->map(virtAddress, physAddress, VMM_PRESENT
        | VMM_READ_WRITE, pages);
}
void Module::loadSymbols()
{
    Logger::getInstance()->log("Parsing %s\n", name);
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)virtAddress;
    Logger::getInstance()->log("Ehdr->? = %s\n", ehdr->e_ident);
    Elf64_Shdr* shdr = (Elf64_Shdr*)(virtAddress + ehdr->e_shoff);
    Logger::getInstance()->log("&shdr = 0x%x, &ehdr = 0x%x\n", shdr, ehdr);
    Elf64_Shdr* symtab;
    size_t shStrIdx = ehdr->e_shstrndx;
    Elf64_Shdr* shstrtab = &shdr[shStrIdx];
    Elf64_Shdr* strtab;
    for (size_t i = 0; i < ehdr->e_shnum; i++)
    {
        if (strcmp(".strtab", (char*)(virtAddress + shstrtab->sh_offset + shdr[i].sh_name)) == 0)
        {
            strtab = &shdr[i];
        }
        Logger::getInstance()->log("Found %s\n", (char*)(virtAddress + shstrtab->sh_offset + shdr[i].sh_name));
    }
    char* str = (char*)(virtAddress + strtab->sh_offset);
    for (size_t j = 0; j < ehdr->e_shnum; j++)
    {
        if (shdr[j].sh_type == SHT_SYMTAB)
        {
            symtab = &shdr[j];
            Elf64_Sym* sym = (Elf64_Sym*)(virtAddress + symtab->sh_offset);
            for (size_t i = 0; i < symtab->sh_size / symtab->sh_entsize; i++)
            {
                if (strcmp(&str[sym[i].st_name], "") == 0) continue;
                Logger::getInstance()->log("Found symbol %s\n", &str[sym[i].st_name]);
                char* symbolNameOld = &str[sym[i].st_name];
                char* symbolName = new char[strlen(symbolNameOld) + 1];
                memcpy(symbolName, symbolNameOld, strlen(symbolNameOld) + 1);
                Symbol symbol;
                symbol.name = name;
                symbol.value = sym[i].st_value;
                symbols.add(symbol);
            }
        }
    }
}
uint64_t Module::getSymbol(const char* name)
{
    for (size_t i = 0; i < symbols.size(); i++)
    {
        if (strcmp(symbols[i].name, name) == 0)
        {
            return symbols[i].value;
        }
    }
    return -1;
}
ModuleLoader ModuleLoader::instance;
ModuleLoader::ModuleLoader(Vector<Module*> modulesToLoad)
{
    for (size_t i = 0; i < modulesToLoad.size(); i++)
    {
        modulesToLoad[i]->loadSymbols();
    }
    loadedModules = modulesToLoad;
}