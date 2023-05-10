#include <XSDT.hpp>
XSDT* XSDT::instance = NULL;
ACPIHeader* XSDT::find(const char* name)
{
    for (size_t i = 0; i < (header.length - sizeof(ACPIHeader)) / 8; i++)
    {
        if (memcmp(name, entries[i]->signature, 4) == 0)
        {
            return entries[i];
        }
    }
    return NULL;
}
void XSDT::loadXSDT(XSDP* xsdp)
{
    instance = (XSDT*)xsdp->xsdtAdress;
}
XSDT* XSDT::getInstance()
{
    return instance;
}