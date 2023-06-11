#ifndef XSDT_HPP
#define XSDT_HPP
#include <PhoenixOS.hpp>
struct ACPIHeader
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemID[6];
    char oemTableID[8];
    uint32_t oemRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
};
struct __attribute__((packed)) XSDP
{
    char signature[8];
    uint8_t checksum;
    char oemID[6];
    uint8_t revision;
    uint32_t unused;
    uint32_t length;
    uint64_t xsdtAdress;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
};
class XSDT
{
private:
    ACPIHeader header;
    ACPIHeader* entries[];
    static XSDT* instance;
public:
    ACPIHeader* find(const char* name);
    static void loadXSDT(XSDP* xsdp);
    static void setupPaging();
    static XSDT* getInstance();
};
#endif