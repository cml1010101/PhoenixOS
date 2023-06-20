#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#include <PhoenixOS.hpp>
#include <efi.h>
#ifdef __cplusplus
class Font
{
public:
    uint8_t* glyphs;
    size_t fontSizeX, fontSizeY;
    size_t bytesPerGlyph;
    Font(void* src);
    uint8_t* getCharacterGlyph(unsigned char c);
};
class Window
{
public:
    uint32_t* framebuffer;
    size_t windowX, windowY;
    size_t width, height;
    Window(size_t x, size_t y, size_t w, size_t h);
    void fillRect(size_t x, size_t y, size_t w, size_t h, uint32_t color);
    void drawChar(size_t x, size_t y, unsigned char c, uint32_t color, Font* font);
};
void initializeGraphics(size_t framebuffer, size_t vertRes, size_t horiRes);
Window* generateWindow(size_t x, size_t y, size_t w, size_t h);
void initializeCursor();
#endif
#endif