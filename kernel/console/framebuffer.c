#include "framebuffer.h"

#include <stdint.h>

#define HYBRID_GOP_PIXEL_RGB_RESERVED 0U
#define HYBRID_GOP_PIXEL_BGR_RESERVED 1U

static uint32_t pack_pixel(uint8_t red, uint8_t green, uint8_t blue,
                           uint32_t pixel_format)
{
    if (pixel_format == HYBRID_GOP_PIXEL_RGB_RESERVED) {
        return UINT32_C(0xFF000000) | ((uint32_t)blue << 16U) |
               ((uint32_t)green << 8U) | (uint32_t)red;
    }
    return UINT32_C(0xFF000000) | ((uint32_t)red << 16U) |
           ((uint32_t)green << 8U) | (uint32_t)blue;
}

static void draw_rect(volatile uint32_t *pixels, uint32_t pitch,
                      uint32_t screen_width, uint32_t screen_height,
                      uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                      uint32_t color)
{
    uint32_t row;
    uint32_t column;

    if (x >= screen_width || y >= screen_height) {
        return;
    }
    if (width > screen_width - x) {
        width = screen_width - x;
    }
    if (height > screen_height - y) {
        height = screen_height - y;
    }

    for (row = 0U; row < height; ++row) {
        for (column = 0U; column < width; ++column) {
            pixels[(uint64_t)(y + row) * pitch + (x + column)] = color;
        }
    }
}

void hybrid_framebuffer_draw_boot_screen(const hybrid_framebuffer_info_t *framebuffer)
{
    volatile uint32_t *pixels;
    uint32_t background;
    uint32_t accent;
    uint32_t panel;
    uint32_t bar_x;
    uint32_t bar_y;
    uint32_t bar_width;
    uint32_t bar_height;
    uint32_t i;
    uint64_t required_bytes;

    if (framebuffer == 0 || framebuffer->base == 0U ||
        framebuffer->width == 0U || framebuffer->height == 0U ||
        framebuffer->pixels_per_scanline < framebuffer->width ||
        (framebuffer->pixel_format != HYBRID_GOP_PIXEL_RGB_RESERVED &&
         framebuffer->pixel_format != HYBRID_GOP_PIXEL_BGR_RESERVED)) {
        return;
    }

    if ((uint64_t)framebuffer->height >
        UINT64_MAX / ((uint64_t)framebuffer->pixels_per_scanline * 4U)) {
        return;
    }
    required_bytes = (uint64_t)framebuffer->pixels_per_scanline *
                     (uint64_t)framebuffer->height * 4U;
    if (required_bytes > framebuffer->size_bytes) {
        return;
    }

    pixels = (volatile uint32_t *)(uintptr_t)framebuffer->base;
    background = pack_pixel(5U, 10U, 24U, framebuffer->pixel_format);
    accent = pack_pixel(0U, 205U, 230U, framebuffer->pixel_format);
    panel = pack_pixel(12U, 28U, 49U, framebuffer->pixel_format);

    draw_rect(pixels, framebuffer->pixels_per_scanline,
              framebuffer->width, framebuffer->height,
              0U, 0U, framebuffer->width, framebuffer->height, background);

    bar_width = framebuffer->width / 3U;
    if (bar_width < 120U) {
        bar_width = framebuffer->width > 24U ? framebuffer->width - 24U : framebuffer->width;
    }
    bar_height = framebuffer->height > 700U ? 12U : 8U;
    bar_x = (framebuffer->width - bar_width) / 2U;
    bar_y = framebuffer->height / 2U + 34U;

    draw_rect(pixels, framebuffer->pixels_per_scanline,
              framebuffer->width, framebuffer->height,
              bar_x, bar_y, bar_width, bar_height, panel);
    draw_rect(pixels, framebuffer->pixels_per_scanline,
              framebuffer->width, framebuffer->height,
              bar_x, bar_y, bar_width / 2U, bar_height, accent);

    for (i = 0U; i < 4U; ++i) {
        uint32_t segment_width = 22U;
        uint32_t gap = 10U;
        uint32_t total = 4U * segment_width + 3U * gap;
        uint32_t start_x = framebuffer->width > total ?
            (framebuffer->width - total) / 2U : 0U;
        draw_rect(pixels, framebuffer->pixels_per_scanline,
                  framebuffer->width, framebuffer->height,
                  start_x + i * (segment_width + gap),
                  framebuffer->height / 2U + 4U,
                  segment_width, 4U, accent);
    }
}
