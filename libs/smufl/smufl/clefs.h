/*
 * Copyright (C) 2023 Marijn Kruisselbrink <mek@google.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SMUFL_CLEFS_H
#define SMUFL_CLEFS_H

#include "smufl/glyph.h"

namespace SMuFL
{

struct Clef {
    const char* const name;
    const Glyph glyph;
    // 0 = bottom line, 8 = top line
    const int clef_position;
    // midi note number at position
    const uint8_t clef_note;

    uint8_t note_for_position(int note_pos) const;
    int position_for_note(uint8_t note) const;

    uint8_t lowest_note_on_bar() const { return note_for_position(-1); }
    uint8_t highest_note_on_bar() const { return note_for_position(9); };
};

// Terminated by name=nullptr.
extern Clef g_clefs[];

} // namespace SMuFL

#endif // SMUFL_CLEFS_H
