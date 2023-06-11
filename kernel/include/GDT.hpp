#ifndef GDT_HPP
#define GDT_HPP
#include <PhoenixOS.hpp>
struct __attribute__((packed)) GDTEntry
{
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMiddle;
    uint8_t access;
    uint8_t granularity;
    uint8_t baseHigh;
    GDTEntry();
    GDTEntry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
};
struct __attribute__((packed)) GDTSystemEntry
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t limit1: 4;
    uint8_t flags: 4;
    uint8_t base2;
    uint32_t base3;
    uint32_t rsv;
    GDTSystemEntry();
    GDTSystemEntry(uint64_t base, uint32_t limit, uint8_t access, uint8_t flags);
};
class GDT
{
private:
    uint64_t* gdtEntries;
public:
    GDT() = default;
    GDT(bool includeUserSegments);
    //GDTEntry& operator[](size_t i);
    //GDTEntry operator[](size_t i) const;
    void load(uint64_t cs, uint64_t ds, uint64_t es, uint64_t fs, uint64_t gs, uint64_t ss);
};
#endif