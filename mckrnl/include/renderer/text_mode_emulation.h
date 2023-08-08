#pragma once

#include <renderer/psf1_font.h>
#include <config.h>

void init_text_mode_emulation(psf1_font_t font);

void text_mode_emulation_update();

#ifdef TEXT_MODE_EMULATION
#define EMU_UPDATE() text_mode_emulation_update()
#else
#define EMU_UPDATE()
#endif

void draw_char(uint32_t x, uint32_t y, char c, uint32_t color, uint32_t bgcolor);