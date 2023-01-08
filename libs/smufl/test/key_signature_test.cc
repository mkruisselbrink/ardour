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

#include "smufl/key_signature.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace SMuFL
{
namespace
{
	namespace Notes
	{
		uint8_t C = 0;
		uint8_t Cs = 1;
		uint8_t Db = 1;
		uint8_t D = 2;
		uint8_t Ds = 3;
		uint8_t Eb = 3;
		uint8_t E = 4;
		uint8_t F = 5;
		uint8_t Fs = 6;
		uint8_t Gb = 6;
		uint8_t G = 7;
		uint8_t Gs = 8;
		uint8_t Ab = 8;
		uint8_t A = 9;
		uint8_t As = 10;
		uint8_t Bb = 10;
		uint8_t B = 11;
	}
}

class KeySignatureTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (KeySignatureTest);
	CPPUNIT_TEST (baseNote);
	CPPUNIT_TEST (sharpsOrFlats);
	CPPUNIT_TEST (noteAndAccidentalsToRender);
	CPPUNIT_TEST_SUITE_END();

public:
	void baseNote();
	void sharpsOrFlats();
	void noteAndAccidentalsToRender();
};

CPPUNIT_TEST_SUITE_REGISTRATION (KeySignatureTest);

void
KeySignatureTest::baseNote()
{
	CPPUNIT_ASSERT_EQUAL (Notes::C, KeySignature (0, KeySignature::Scale::kMajor).base_note());
	CPPUNIT_ASSERT_EQUAL (Notes::A, KeySignature (0, KeySignature::Scale::kMinor).base_note());
	CPPUNIT_ASSERT_EQUAL (Notes::Ab, KeySignature (-4, KeySignature::Scale::kMajor).base_note());
	CPPUNIT_ASSERT_EQUAL (Notes::F, KeySignature (-4, KeySignature::Scale::kMinor).base_note());
	CPPUNIT_ASSERT_EQUAL (Notes::Fs, KeySignature (6, KeySignature::Scale::kMajor).base_note());
	CPPUNIT_ASSERT_EQUAL (Notes::Ds, KeySignature (6, KeySignature::Scale::kMinor).base_note());
}

void
KeySignatureTest::sharpsOrFlats()
{
	KeySignature ks0 (0, KeySignature::Scale::kMajor);
	CPPUNIT_ASSERT (ks0.sharps().empty());
	CPPUNIT_ASSERT (ks0.flats().empty());

	KeySignature ks7b (-7, KeySignature::Scale::kMinor);
	CPPUNIT_ASSERT (ks7b.sharps().empty());
	CPPUNIT_ASSERT_EQUAL (size_t{ 7 }, ks7b.flats().size());
	CPPUNIT_ASSERT_EQUAL (Notes::B, ks7b.flats()[0]);
	CPPUNIT_ASSERT_EQUAL (Notes::E, ks7b.flats()[1]);
	CPPUNIT_ASSERT_EQUAL (Notes::A, ks7b.flats()[2]);
	CPPUNIT_ASSERT_EQUAL (Notes::D, ks7b.flats()[3]);
	CPPUNIT_ASSERT_EQUAL (Notes::G, ks7b.flats()[4]);
	CPPUNIT_ASSERT_EQUAL (Notes::C, ks7b.flats()[5]);
	CPPUNIT_ASSERT_EQUAL (Notes::F, ks7b.flats()[6]);

	KeySignature ks7s (7, KeySignature::Scale::kMajor);
	CPPUNIT_ASSERT (ks7s.flats().empty());
	CPPUNIT_ASSERT_EQUAL (size_t{ 7 }, ks7s.sharps().size());
	CPPUNIT_ASSERT_EQUAL (Notes::F, ks7s.sharps()[0]);
	CPPUNIT_ASSERT_EQUAL (Notes::C, ks7s.sharps()[1]);
	CPPUNIT_ASSERT_EQUAL (Notes::G, ks7s.sharps()[2]);
	CPPUNIT_ASSERT_EQUAL (Notes::D, ks7s.sharps()[3]);
	CPPUNIT_ASSERT_EQUAL (Notes::A, ks7s.sharps()[4]);
	CPPUNIT_ASSERT_EQUAL (Notes::E, ks7s.sharps()[5]);
	CPPUNIT_ASSERT_EQUAL (Notes::B, ks7s.sharps()[6]);
}

void
KeySignatureTest::noteAndAccidentalsToRender()
{
	constexpr uint8_t scale[7] = { 0, 2, 4, 5, 7, 9, 11 };
	// Notes in the scale should all render without accidentals
	for (int s_or_f = -7; s_or_f <= 7; s_or_f++) {
		KeySignature ks (s_or_f);
		for (int oct = 0; oct < 9; oct++) {
			for (uint8_t scale_note : scale) {
				uint8_t note = ks.base_note() + oct * 12 + scale_note;
				CPPUNIT_ASSERT_EQUAL (KeySignature::Accidental::kNone,
				                      ks.note_and_accidentals_to_render (note).accidental);
			}
		}
	}

	// Natural notes should always render without accidentals or with natural accidentals.
	// Depending on key signature it is possible for these to be rendered as a different note though (for example
	// an F in a key signature with an E-sharp).
	for (int s_or_f = -7; s_or_f <= 7; s_or_f++) {
		KeySignature ks (s_or_f);
		for (int oct = 0; oct < 10; oct++) {
			for (uint8_t scale_note : scale) {
				uint8_t note = oct * 12 + scale_note;
				auto a = ks.note_and_accidentals_to_render (note).accidental;
				CPPUNIT_ASSERT (a == KeySignature::Accidental::kNone
				                || a == KeySignature::Accidental::kNatural);
			}
		}
	}

	// If accidental is a sharp, note to render should be one lower than note, and vice versa for flats.
	for (int s_or_f = -7; s_or_f <= 7; s_or_f++) {
		KeySignature ks (s_or_f);
		for (uint8_t note = 0; note <= 120; ++note) {
			auto na = ks.note_and_accidentals_to_render (note);
			switch (na.accidental) {
			case KeySignature::Accidental::kNatural:
				CPPUNIT_ASSERT_EQUAL (note, na.note);
				break;
			case KeySignature::Accidental::kFlat:
				CPPUNIT_ASSERT_EQUAL (note + 1, int{ na.note });
				break;
			case KeySignature::Accidental::kSharp:
				CPPUNIT_ASSERT_EQUAL (note - 1, int{ na.note });
				break;
			case KeySignature::Accidental::kNone:
				// Could be same, higher, or lower. But always at most one difference.
				CPPUNIT_ASSERT_LESSEQUAL (1, std::abs (na.note - note));
				break;
			}
		}
	}

	// TODO: Check consistency with `sharps` and `flats`
}

} // namespace SMuFL
