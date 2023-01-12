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

#ifndef SCORE_CLEF_H_
#define SCORE_CLEF_H_

#include "smufl/glyph.h"

namespace Score {

struct Clef {
	const char *const name;
	const SMuFL::Glyph glyph;
	// 0 = bottom line, 8 = top line
	const int clef_position;
	// midi note number at position
	const uint8_t clef_note;
	// how low the lowest key signature sharp should be above the bottom line
	const uint8_t key_signature_sharp_offset = 0;
	// how low the lowest key signature flat should be above the bottom line
	const uint8_t key_signature_flat_offset = 0;

	uint8_t note_for_position (int note_pos) const;
	int position_for_note (uint8_t note) const;

	uint8_t
	lowest_note_on_bar() const
	{
		return note_for_position (0);
	}
	uint8_t
	highest_note_on_bar() const
	{
		return note_for_position (8);
	};

	int notes_on_bar (uint8_t note_min, uint8_t note_max) const;

	static Clef treble_clef;
	static Clef bass_clef;
};

// Terminated by nullptr.
extern Clef *g_clefs[];

} // namespace Score

#endif // SCORE_CLEF_H_
