#include <Module.hpp>
#include <VirtualMemoryManager.hpp>
#include <PhysicalMemoryManager.hpp>
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
void Module::start()
{
    Thread* moduleInit = new Thread(name, (Runnable)getSymbol("module_init"));
    Scheduler::schedule(moduleInit);
}
void Module::load(bool loadProg)
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
    }
    str = (char*)(virtAddress + strtab->sh_offset);
    for (size_t j = 0; j < ehdr->e_shnum; j++)
    {
        if (shdr[j].sh_type == SHT_SYMTAB)
        {
            symtab = &shdr[j];
            sym = (Elf64_Sym*)(virtAddress + symtab->sh_offset);
            for (size_t i = 0; i < symtab->sh_size / symtab->sh_entsize; i++)
            {
                if (strcmp(&str[sym[i].st_name], "") == 0) continue;
                Symbol symbol;
                symbol.name = &str[sym[i].st_name];
                symbol.value = sym[i].st_value;
                symbols.add(symbol);
            }
        }
    }
    if (loadProg)
    {
        Logger::getInstance()->log("Loading %s into memory\n", name);
        for (size_t i = 0; i < ehdr->e_shnum; i++)
        {
            if (shdr[i].sh_type == SHT_PROGBITS)
            {
                uint64_t pages = (shdr[i].sh_size + 0xFFF) / 0x1000;
                shdr[i].sh_addr = (uint64_t)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocate(pages,
                    VMM_PRESENT | VMM_READ_WRITE);
                memcpy((void*)shdr[i].sh_addr, 
                    (const void*)(virtAddress + shdr[i].sh_offset), shdr[i].sh_size);
                memset((void*)(shdr[i].sh_addr + shdr[i].sh_size), 0,
                    pages * 0x1000 - shdr[i].sh_size);
            }
        }
    }
    Logger::getInstance()->log("Performing relocation\n");
    Elf64_Shdr* reltab = NULL;
    for (size_t i = 0; i < ehdr->e_shnum; i++)
    {
        if (shdr[i].sh_type == SHT_RELA)
        {
            reltab = &shdr[i];
            Elf64_Rela* relocations = (Elf64_Rela*)(virtAddress + reltab->sh_offset);
            Logger::getInstance()->log("Found rela: %s\n", (char*)(virtAddress + shstrtab->sh_offset + shdr[i].sh_name));
            for (size_t j = 0; j < reltab->sh_size / sizeof(Elf64_Rela); j++)
            {
                Elf64_Sym* symbolEntry = &sym[ELF64_R_SYM(relocations[j].r_info)];
                Elf64_Shdr* linkedShdr = &shdr[symbolEntry->st_shndx];
                uint64_t* relocAddress = (uint64_t*)(shdr[symbolEntry->st_shndx].sh_addr + relocations[i].r_offset);
                char* symbolName = &str[symbolEntry->st_name];
                Logger::getInstance()->log("Symbol name %s\n", symbolName);
                Logger::getInstance()->log("Relocation Addr 0x%x\n", relocAddress);
                if (symbolEntry->st_shndx == 0)
                {
                    Logger::getInstance()->log("Searching for kernel symbol %s\n", symbolName);
                    size_t symVal = ModuleLoader::instance.getSymbol(symbolName);
                    if (symVal == -1) Logger::getInstance()->log("Couldn't find %s\n", symbolName);
                    *relocAddress = symVal;
                }
                else
                {
                    Elf64_Shdr* section = &shdr[symbolEntry->st_shndx];
                    uint64_t* relocAddress = (uint64_t*)(shdr[symbolEntry->st_shndx].sh_addr + relocations[i].r_offset);
                    *relocAddress = section->sh_addr + symbolEntry->st_value + relocations[i].r_addend;
                }
            }
        }
    }
}
ModuleLoader ModuleLoader::instance;
ModuleLoader::ModuleLoader(Vector<Module*> modulesToLoad)
{
    modulesToLoad[0]->load(false);
    loadedModules.add(modulesToLoad[0]);
    for (size_t i = 1; i < modulesToLoad.size(); i++)
    {
        modulesToLoad[i]->load(true);
        loadedModules.add(modulesToLoad[i]);
        Logger::getInstance()->log("Modules: %d\n", loadedModules.size());
    }
    for (size_t i = 1; i < loadedModules.size(); i++)
    {
        loadedModules[i]->start();
    }
}
size_t ModuleLoader::getSymbol(const char* name)
{
    Logger::getInstance()->log("Searching for symbol %s\n", name);
    for (size_t i = 0; i < loadedModules.size(); i++)
    {
        Logger::getInstance()->log("Searching module %s\n", loadedModules[i]->getName());
        size_t val = loadedModules[i]->getSymbol(name);
        if (val != -1) return val;
    }
    return -1;
}