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

#include "smufl/clefs.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace SMuFL
{

class ClefTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (ClefTest);
	CPPUNIT_TEST (positionForNoteForPosition);
	CPPUNIT_TEST (lowestNoteOnBar);
	CPPUNIT_TEST (highestNoteOnBar);
	CPPUNIT_TEST_SUITE_END();

public:
	void positionForNoteForPosition();
	void lowestNoteOnBar();
	void highestNoteOnBar();
};

CPPUNIT_TEST_SUITE_REGISTRATION (ClefTest);

void
ClefTest::positionForNoteForPosition()
{
	Clef treble = { "Treble", Glyph::kGClef, 2, 67 };
	uint8_t whole_notes[] = { 0, 2, 4, 5, 7, 9, 11 };
	// Middle C (note 60) starts as position -2
	// Note 0 is 5 octaves or 7*5=35 lines lower
	// So the last note before that is one position lower still.
	int last_pos = -2 - 7 * 5 - 1;
	for (int octave = 0; octave < 10; octave++) {
		for (uint8_t n : whole_notes) {
			uint8_t note = octave * 12 + n;
			int pos = treble.position_for_note (note);
			CPPUNIT_ASSERT_EQUAL (last_pos + 1, pos);
			CPPUNIT_ASSERT_EQUAL (note, treble.note_for_position (pos));
			last_pos = pos;
		}
	}
}

void
ClefTest::lowestNoteOnBar()
{
    for (int i = -2; i < 10; ++i) {
        std::cerr << "Pos: " << i << ", note: " << int{Clef::treble_clef.note_for_position(i)} << std::endl;
    }
	CPPUNIT_ASSERT_EQUAL (uint8_t{ 64 }, Clef::treble_clef.lowest_note_on_bar());
	CPPUNIT_ASSERT_EQUAL (uint8_t{ 43 }, Clef::bass_clef.lowest_note_on_bar());
}

void
ClefTest::highestNoteOnBar()
{
	CPPUNIT_ASSERT_EQUAL (uint8_t{ 77 }, Clef::treble_clef.highest_note_on_bar());
	CPPUNIT_ASSERT_EQUAL (uint8_t{ 57 }, Clef::bass_clef.highest_note_on_bar());
}

} // namespace SMuFL
