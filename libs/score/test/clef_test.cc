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

#include "score/clef.h"

#include <gtest/gtest.h>

namespace Score {

TEST (ClefTest, positionForNoteForPosition)
{
	auto &treble = Clef::treble_clef;
	uint8_t whole_notes[] = { 0, 2, 4, 5, 7, 9, 11 };
	// Middle C (note 60) starts as position -2
	// Note 0 is 5 octaves or 7*5=35 lines lower
	// So the last note before that is one position lower still.
	int last_pos = -2 - 7 * 5 - 1;
	for (int octave = 0; octave < 10; octave++) {
		for (uint8_t n : whole_notes) {
			uint8_t note = octave * 12 + n;
			int pos = treble.position_for_note (note);
			EXPECT_EQ (last_pos + 1, pos);
			EXPECT_EQ (note, treble.note_for_position (pos));
			last_pos = pos;
		}
	}
}

TEST (ClefTest, lowestNoteOnBar)
{
	EXPECT_EQ (uint8_t{ 64 }, Clef::treble_clef.lowest_note_on_bar());
	EXPECT_EQ (uint8_t{ 43 }, Clef::bass_clef.lowest_note_on_bar());
}

TEST (ClefTest, highestNoteOnBar)
{
	EXPECT_EQ (uint8_t{ 77 }, Clef::treble_clef.highest_note_on_bar());
	EXPECT_EQ (uint8_t{ 57 }, Clef::bass_clef.highest_note_on_bar());
}

} // namespace Score
