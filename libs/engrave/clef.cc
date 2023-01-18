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

#include "engrave/clef.h"

namespace Engrave {

uint8_t
Clef::note_for_position (int note_pos) const
{
	int octaves = 0;
	while (note_pos < clef_position) {
		note_pos += 7;
		octaves++;
	}
	while (note_pos - clef_position >= 7) {
		note_pos -= 7;
		octaves--;
	}
	static const uint8_t note_differences[12][7] = {
		/*     C  D  E  F  G  A   B   C   D   E*/
		/*C*/ { 0, 2, 4, 5, 7, 9, 11 },
		{},
		/*D   */ { 0, 2, 3, 5, 7, 9, 10 },
		{},
		/*E      */ { 0, 1, 3, 5, 7, 8, 10 },
		/*F         */ { 0, 2, 4, 6, 7, 9, 11 },
		{},
		/*        D  E  F  G  A   B   C   D   E   F   G   A*/
		/*G            */ { 0, 2, 4, 5, 7, 9, 10 },
		{},
		/*A               */ { 0, 2, 3, 5, 7, 8, 10 },
		{},
		/*B                  */ { 0, 1, 3, 5, 6, 8, 10 },
		/*                 G  A   B   C   D   E   F   G   A*/
	};
	return clef_note + note_differences[clef_note % 12][note_pos - clef_position] - octaves * 12;
}

int
Clef::position_for_note (uint8_t note) const
{
	uint8_t base_note = note % 12;
	uint8_t base_octave = note / 12;
	uint8_t clef_octave = clef_note / 12;
	uint8_t clef_base_note = clef_note % 12;
	// octave is +/- 7 positions
	// note difference is trickier...
	if (base_note < clef_base_note) {
		base_note += 12;
		clef_octave++;
	}

	// now we have base_note (0..23) >= clef_base_note (0..11),
	// or base_note-clef_base_note(0..12), clef_base_note (0..11).
	// lookup table?
	/* TODO: somehow use this table rather than the bigger one?
	static uint8_t deltas[] = {
	    0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6
	};*/

	static const uint8_t position_difference[12][12] = {
		/*      C     D     E  F     G     A     B  C     D     E  F */
		/*C*/ { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 },
		{},
		/*D      */ { 0, 0, 1, 2, 2, 3, 3, 4, 4, 5, 6, 6 },
		{},
		/*E            */ { 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6 },
		/*F               */ { 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6 },
		{},
		/*            D     E  F     G     A     B  C     D     E  F     G     A */
		/*G                     */ { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6 },
		{},
		/*A                           */ { 0, 0, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6 },
		{},
		/*B                                 */ { 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6 },
		/*                     F     G     A     B  C     D     E  F     G     A */
	};
	int base_position = position_difference[clef_base_note][base_note - clef_base_note];
	return clef_position + base_position + 7 * (base_octave - clef_octave);
}

int
Clef::position_for_step (Step s, bool for_sharp) const
{
	int line_offset = static_cast<int> (s) - static_cast<int> (clef_pitch.step());
	int position = clef_position + line_offset - 7;
	int min_pos = for_sharp ? key_signature_sharp_offset : key_signature_flat_offset;
	while (position < min_pos) {
		position += 7;
	}
	return position;
}

int
Clef::position_for_pitch (const Pitch &p) const
{
	return (p - clef_pitch) + clef_position;
}

int
Clef::notes_on_bar (uint8_t note_min, uint8_t note_max) const
{
	// note_min below bar:
	int pos_min = position_for_note (note_min);
	int pos_max = position_for_note (note_max);
	if (pos_min > 8) {
		return 0;
	}
	if (pos_max < 0) {
		return 0;
	}
	return std::min (pos_max, 8) - std::max (pos_min, 0) + 1;
}

Clef Clef::treble_clef = Clef{ "Treble", Glyph::kGClef, 2, 67, Pitch (4, Step::G), 3, 1 };
Clef Clef::bass_clef = Clef{ "Bass", Glyph::kFClef, 6, 53, Pitch (3, Step::F), 1, 0 };

Clef *g_clefs[] = { &Clef::treble_clef, &Clef::bass_clef, nullptr };

} // namespace Engrave
