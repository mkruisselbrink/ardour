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

#include "engrave/key_signature.h"

#include <gtest/gtest.h>

namespace Engrave {

namespace {
	namespace Notes {
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
		uint8_t Cb = 11;
	}
}

TEST (KeySignatureTest, baseNote)
{
	EXPECT_EQ (Notes::C, KeySignature (0, Scale::kMajor).base_note());
	EXPECT_EQ (Notes::A, KeySignature (0, Scale::kMinor).base_note());
	EXPECT_EQ (Notes::Cb, KeySignature (-7).base_note());
	EXPECT_EQ (Notes::Gb, KeySignature (-6).base_note());
	EXPECT_EQ (Notes::Db, KeySignature (-5).base_note());
	EXPECT_EQ (Notes::Ab, KeySignature (-4, Scale::kMajor).base_note());
	EXPECT_EQ (Notes::F, KeySignature (-4, Scale::kMinor).base_note());
	EXPECT_EQ (Notes::Eb, KeySignature (-3).base_note());
	EXPECT_EQ (Notes::Bb, KeySignature (-2).base_note());
	EXPECT_EQ (Notes::F, KeySignature (-1).base_note());
	EXPECT_EQ (Notes::G, KeySignature (1).base_note());
	EXPECT_EQ (Notes::D, KeySignature (2).base_note());
	EXPECT_EQ (Notes::A, KeySignature (3).base_note());
	EXPECT_EQ (Notes::E, KeySignature (4).base_note());
	EXPECT_EQ (Notes::B, KeySignature (5).base_note());
	EXPECT_EQ (Notes::Fs, KeySignature (6, Scale::kMajor).base_note());
	EXPECT_EQ (Notes::Ds, KeySignature (6, Scale::kMinor).base_note());
	EXPECT_EQ (Notes::Cs, KeySignature (7).base_note());
}

TEST (KeySignatureTest, sharpsOrFlats)
{
	KeySignature ks0 (0, Scale::kMajor);
	EXPECT_TRUE (ks0.sharps().empty());
	EXPECT_TRUE (ks0.flats().empty());

	KeySignature ks7b (-7, Scale::kMinor);
	EXPECT_TRUE (ks7b.sharps().empty());
	EXPECT_EQ (size_t{ 7 }, ks7b.flats().size());
	EXPECT_EQ (Notes::B, ks7b.flats()[0]);
	EXPECT_EQ (Notes::E, ks7b.flats()[1]);
	EXPECT_EQ (Notes::A, ks7b.flats()[2]);
	EXPECT_EQ (Notes::D, ks7b.flats()[3]);
	EXPECT_EQ (Notes::G, ks7b.flats()[4]);
	EXPECT_EQ (Notes::C, ks7b.flats()[5]);
	EXPECT_EQ (Notes::F, ks7b.flats()[6]);

	KeySignature ks7s (7, Scale::kMajor);
	EXPECT_TRUE (ks7s.flats().empty());
	EXPECT_EQ (size_t{ 7 }, ks7s.sharps().size());
	EXPECT_EQ (Notes::F, ks7s.sharps()[0]);
	EXPECT_EQ (Notes::C, ks7s.sharps()[1]);
	EXPECT_EQ (Notes::G, ks7s.sharps()[2]);
	EXPECT_EQ (Notes::D, ks7s.sharps()[3]);
	EXPECT_EQ (Notes::A, ks7s.sharps()[4]);
	EXPECT_EQ (Notes::E, ks7s.sharps()[5]);
	EXPECT_EQ (Notes::B, ks7s.sharps()[6]);
}

TEST (KeySignatureTest, noteAndAccidentalsToRender)
{
	constexpr uint8_t scale[7] = { 0, 2, 4, 5, 7, 9, 11 };
	// Notes in the scale should all render without accidentals
	for (int s_or_f = -7; s_or_f <= 7; s_or_f++) {
		KeySignature ks (s_or_f);
		for (int oct = 0; oct < 9; oct++) {
			for (uint8_t scale_note : scale) {
				uint8_t note = ks.base_note() + oct * 12 + scale_note;
				EXPECT_EQ (Accidental::kNone, ks.note_and_accidentals_to_render (note).accidental);
			}
		}
	}

	// Natural notes should always render without accidentals or with natural accidentals.
	// Depending on key signature it is possible for these to be rendered as a different note though (for
	// example an F in a key signature with an E-sharp).
	for (int s_or_f = -7; s_or_f <= 7; s_or_f++) {
		KeySignature ks (s_or_f);
		for (int oct = 0; oct < 10; oct++) {
			for (uint8_t scale_note : scale) {
				uint8_t note = oct * 12 + scale_note;
				auto a = ks.note_and_accidentals_to_render (note).accidental;
				EXPECT_TRUE (a == Accidental::kNone || a == Accidental::kNatural);
			}
		}
	}

	// If accidental is a sharp, note to render should be one lower than note, and vice versa for flats.
	for (int s_or_f = -7; s_or_f <= 7; s_or_f++) {
		KeySignature ks (s_or_f);
		for (uint8_t note = 0; note <= 120; ++note) {
			auto na = ks.note_and_accidentals_to_render (note);
			switch (na.accidental) {
			case Accidental::kNatural:
				EXPECT_EQ (note, na.note);
				break;
			case Accidental::kFlat:
				EXPECT_EQ (note + 1, int{ na.note });
				break;
			case Accidental::kSharp:
				EXPECT_EQ (note - 1, int{ na.note });
				break;
			case Accidental::kNone:
				// Could be same, higher, or lower. But always at most one
				// difference.
				EXPECT_LE (std::abs (na.note - note), 1);
				break;
			}
		}
	}

	// TODO: Check consistency with `sharps` and `flats`
}

} // namespace Engrave
