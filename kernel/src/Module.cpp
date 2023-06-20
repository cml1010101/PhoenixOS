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
uint64_t Module::getSymbolValue(size_t table, size_t idx)
{
    if (table == SHN_UNDEF || idx == SHN_UNDEF) return 0;
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)virtAddress;
    Elf64_Shdr* shdr = (Elf64_Shdr*)(virtAddress + ehdr->e_shoff);
    Elf64_Sym* sym = (Elf64_Sym*)(virtAddress + shdr[table].sh_offset);
    Elf64_Sym* symbol = &sym[idx];
    if (symbol->st_shndx == SHN_UNDEF)
    {
        Elf64_Shdr* strtab = &shdr[shdr[table].sh_link];
        const char* name = (const char*)(virtAddress + strtab->sh_offset + symbol->st_name);
        uint64_t target = ModuleLoader::instance.getSymbol(name);
        if (target == 0)
        {
            if (ELF64_ST_BIND(symbol->st_info) & STB_WEAK)
            {
                return 0;
            }
            return -1;
        }
        return target;
    }
    else if (symbol->st_shndx == SHN_ABS)
    {
        return symbol->st_value;
    }
    Elf64_Shdr* target = &shdr[symbol->st_shndx];
    return virtAddress + symbol->st_value + target->sh_offset;
}
void Module::load(bool loadProg)
{
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)virtAddress;
    Elf64_Shdr* shdr = (Elf64_Shdr*)(virtAddress + ehdr->e_shoff);
    size_t shStrIdx = ehdr->e_shstrndx;
    Elf64_Shdr* shstrtab = &shdr[shStrIdx];
    Elf64_Shdr* strtab;
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
                shdr[i].sh_offset = shdr[i].sh_addr - virtAddress;
                Logger::getInstance()->log("Copied program into memory: %s at 0x%x\n",
                    (char*)(virtAddress + shstrtab->sh_offset + shdr[i].sh_name), shdr[i].sh_addr);
            }
            else if (shdr[i].sh_type == SHT_NOBITS)
            {
                if (!shdr[i].sh_size) continue;
                if (shdr[i].sh_flags & SHF_ALLOC)
                {
                    uint64_t pages = (shdr[i].sh_size + 0xFFF) / 0x1000;
                    shdr[i].sh_addr = (uint64_t)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocate(pages,
                        VMM_PRESENT | VMM_READ_WRITE);
                    memset((void*)shdr[i].sh_addr, 0,
                        pages * 0x1000);
                    shdr[i].sh_offset = shdr[i].sh_addr - virtAddress;
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
                    Elf64_Shdr* target = &shdr[reltab->sh_info];
                    size_t addr = virtAddress + target->sh_offset;
                    size_t ref = (size_t)(addr + relocations[j].r_offset);
                    size_t symval = 0;
                    if (ELF64_R_SYM(relocations[j].r_info) != SHN_UNDEF)
                    {
                        symval = getSymbolValue(reltab->sh_link, ELF64_R_SYM(relocations[j].r_info));
                    }
                    switch (ELF64_R_TYPE(relocations[j].r_info))
                    {
                    case R_X86_64_NONE:
                        break;
                    case R_X86_64_64:
                        *(uint64_t*)ref = symval + *(uint64_t*)ref;
                        break;
                    case R_X86_64_PC64:
                        *(uint64_t*)ref = symval + *(uint64_t*)ref - (uint64_t)ref;
                        break;
                    case R_X86_64_PC32:
                        *(uint32_t*)ref = symval + *(uint32_t*)ref - (uint32_t)ref;
                        break;
                    case R_X86_64_32:
                        *(uint32_t*)ref = symval + *(uint32_t*)ref;
                        break;
                    default:
                        Logger::getInstance()->log("Unrelocatable symbol: 0x%x.\n", ELF64_R_TYPE(relocations[j].r_info));
                        break;
                    }
                }
            }
        }
    }
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
            Elf64_Shdr* symtab = &shdr[j];
            sym = (Elf64_Sym*)(virtAddress + symtab->sh_offset);
            for (size_t i = 0; i < symtab->sh_size / symtab->sh_entsize; i++)
            {
                if (strcmp(&str[sym[i].st_name], "") == 0) continue;
                Symbol symbol;
                symbol.name = &str[sym[i].st_name];
                symbol.value = getSymbolValue(j, i);
                symbols.add(symbol);
                Logger::getInstance()->log("Found symbol: %s\n", symbol.name);
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