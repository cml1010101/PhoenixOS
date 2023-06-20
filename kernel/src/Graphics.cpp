#include <Graphics.hpp>
#include <VirtualMemoryManager.hpp>
#include <Scheduler.hpp>
#define PSF_FONT_MAGIC 0x864ab572
struct PSFHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
};
Font::Font(void* ptr)
{
    PSFHeader* psfHeader = (PSFHeader*)ptr;
    if (psfHeader->magic == PSF_FONT_MAGIC)
    {
        bytesPerGlyph = psfHeader->bytesperglyph;
        fontSizeX = psfHeader->width;
        fontSizeY = psfHeader->height;
        glyphs = new uint8_t[bytesPerGlyph * 256];
        memcpy(glyphs, psfHeader + 1, 256 * bytesPerGlyph);
    }
}
uint8_t* Font::getCharacterGlyph(unsigned char c)
{
    size_t idx = (size_t)c * bytesPerGlyph;
    return &glyphs[idx];
}
extern "C" char _binary_font_psf_start;
extern "C" char _binary_font_psf_end;
Window::Window(size_t x, size_t y, size_t w, size_t h)
    : windowX(x), windowY(y), width(w), height(h)
{
    framebuffer = (uint32_t*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocate(w * h * 4,
        VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE);
    memset(framebuffer, 0xFF, w * h * 4);
}
void Window::fillRect(size_t x, size_t y, size_t w, size_t h, uint32_t color)
{
    size_t where = y * width + x;
    for (size_t i = 0; i < h; i++)
    {
        for (size_t j = 0; j < w; j++)
        {
            framebuffer[where + j] = color;
        }
        where += width;
    }
}
void Window::drawChar(size_t x, size_t y, unsigned char c, uint32_t color, Font* font)
{
    size_t where = y * width + x;
    uint8_t* glyph = font->getCharacterGlyph(c);
    for (size_t i = 0; i < font->fontSizeY; i++)
    {
        for (size_t j = 0; j < font->fontSizeX; j++)
        {
            if (glyph[i] & (1 << j)) framebuffer[where + j] = color;
        }
        where += width;
    }
}
Vector<Window*> windows;
uint32_t* graphicsFramebuffer, *graphicsSecondary;
size_t graphicsWidth, graphicsHeight;
void graphicsUpdate()
{
    while (true)
    {
        size_t where = 0;
        for (size_t i = 0; i < graphicsHeight; i++)
        {
            for (size_t j = 0; j < graphicsWidth; j++)
            {
                graphicsSecondary[where + j] = 0;
            }
            where += graphicsWidth;
        }
        for (size_t i = 0; i < windows.size(); i++)
        {
            size_t where0 = windows[i]->windowY * graphicsWidth + windows[i]->windowX;
            size_t where1 = 0;
            for (size_t j = 0; j < windows[i]->height; j++)
            {
                for (size_t k = 0; k < windows[i]->width; k++)
                {
                    graphicsSecondary[where0 + k] = windows[i]->framebuffer[where1 + k];
                }
                where1 += windows[i]->width;
                where0 += graphicsWidth;
            }
        }
        where = 0;
        for (size_t i = 0; i < graphicsHeight; i++)
        {
            for (size_t j = 0; j < graphicsWidth; j++)
            {
                graphicsFramebuffer[where + j] = graphicsSecondary[where + j];
            }
            where += graphicsWidth;
        }
    }
}
void initializeGraphics(uint64_t framebuffer, uint64_t horizontalRes, uint64_t verticalRes)
{
    windows = Vector<Window*>();
    graphicsFramebuffer = (uint32_t*)framebuffer;
    Logger::getInstance()->log("Found framebuffer: 0x%x\n", framebuffer);
    graphicsWidth = horizontalRes;
    graphicsHeight = verticalRes;
    Logger::getInstance()->log("Allocating framebuffer\n");
    // graphicsSecondary = (uint32_t*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocate((graphicsWidth * graphicsHeight * 4 + 0xFFF) / 0x1000,
    //     VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE);
    Logger::getInstance()->log("Mapping graphics framebuffer\n");
    VirtualMemoryManager::getKernelVirtualMemoryManager()->map((uint64_t)(graphicsFramebuffer = (uint32_t*)VirtualMemoryManager::getKernelVirtualMemoryManager()->allocateAddress((graphicsWidth * graphicsHeight * 4 + 0xFFF) / 0x1000)), framebuffer,
        VMM_PRESENT | VMM_READ_WRITE | VMM_CACHE_DISABLE,
        (graphicsWidth * graphicsHeight * 4 + 0xFFF) / 0x1000);
    Logger::getInstance()->log("Initialized Graphics\n");
    memset(graphicsFramebuffer, 0xFF, graphicsWidth * graphicsHeight * 4);
    //Thread* graphics = new Thread("GRAPHICS", graphicsUpdate);
    //Scheduler::schedule(graphics);
}
Window* generateWindow(size_t x, size_t y, size_t w, size_t h)
{
    Window* window = new Window(x, y, w, h);
    windows.add(window);
    return window;
}