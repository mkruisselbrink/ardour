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

TEST (KeySignatureTest, sharps)
{
	KeySignature ks0 (0);
	EXPECT_TRUE (ks0.sharps().empty());

	KeySignature ks3 (3, Scale::kMinor);
	ASSERT_EQ (3, ks3.sharps().size());
	EXPECT_EQ (Step::F, ks3.sharps()[0]);
	EXPECT_EQ (Step::C, ks3.sharps()[1]);
	EXPECT_EQ (Step::G, ks3.sharps()[2]);

	KeySignature ks7 (7);
	ASSERT_EQ (7, ks7.sharps().size());
	EXPECT_EQ (Step::F, ks7.sharps()[0]);
	EXPECT_EQ (Step::C, ks7.sharps()[1]);
	EXPECT_EQ (Step::G, ks7.sharps()[2]);
	EXPECT_EQ (Step::D, ks7.sharps()[3]);
	EXPECT_EQ (Step::A, ks7.sharps()[4]);
	EXPECT_EQ (Step::E, ks7.sharps()[5]);
	EXPECT_EQ (Step::B, ks7.sharps()[6]);

	KeySignature ks2f (-2);
	EXPECT_TRUE (ks2f.sharps().empty());
}

TEST (KeySignatureTest, flats)
{
	KeySignature ks0 (0);
	EXPECT_TRUE (ks0.flats().empty());

	KeySignature ks4 (-4);
	ASSERT_EQ (4, ks4.flats().size());
	EXPECT_EQ (Step::B, ks4.flats()[0]);
	EXPECT_EQ (Step::E, ks4.flats()[1]);
	EXPECT_EQ (Step::A, ks4.flats()[2]);
	EXPECT_EQ (Step::D, ks4.flats()[3]);

	KeySignature ks7 (-7, Scale::kMinor);
	ASSERT_EQ (7, ks7.flats().size());
	EXPECT_EQ (Step::B, ks7.flats()[0]);
	EXPECT_EQ (Step::E, ks7.flats()[1]);
	EXPECT_EQ (Step::A, ks7.flats()[2]);
	EXPECT_EQ (Step::D, ks7.flats()[3]);
	EXPECT_EQ (Step::G, ks7.flats()[4]);
	EXPECT_EQ (Step::C, ks7.flats()[5]);
	EXPECT_EQ (Step::F, ks7.flats()[6]);

	KeySignature ks2s (2);
	EXPECT_TRUE (ks2s.flats().empty());
}

TEST (KeySignatureTest, pitch_correctOctaveAndMidiNote)
{
	for (int s_or_f = -7; s_or_f <= 7; s_or_f++) {
		SCOPED_TRACE (s_or_f);
		KeySignature ks (s_or_f);
		for (uint8_t note = 0; note <= 120; ++note) {
			SCOPED_TRACE (int{ note });
			Pitch p = ks.pitch_from_midi_note (note);
			int expected_octave = note / 12 - 1;
			if (p.step() == Step::C && p.alter() == -1) {
				expected_octave++;
			}
			if (p.step() == Step::B && p.alter() == 1) {
				expected_octave--;
			}
			EXPECT_EQ (expected_octave, p.octave());
			EXPECT_EQ (note, p.midi_note());
		}
	}
}

TEST (KeySignatureTest, pitch_consistentWithSharps)
{
	for (int s = 1; s <= 7; ++s) {
		SCOPED_TRACE (s);
		KeySignature ks (s);
		for (uint8_t note = 0; note <= 120; ++note) {
			Pitch p = ks.pitch_from_midi_note (note);
			EXPECT_TRUE (p.alter() == 0 || p.alter() == 1);
		}
	}
}

TEST (KeySignatureTest, pitch_consistentWithFlats)
{
	for (int s = 1; s <= 7; ++s) {
		SCOPED_TRACE (s);
		KeySignature ks (-s);
		for (uint8_t note = 0; note <= 120; ++note) {
			Pitch p = ks.pitch_from_midi_note (note);
			EXPECT_TRUE (p.alter() == 0 || p.alter() == -1);
		}
	}
}

TEST (KeySignatureTest, alterForStep)
{
	for (int s_or_f = -7; s_or_f <= 7; ++s_or_f) {
		SCOPED_TRACE (s_or_f);
		KeySignature ks (s_or_f);
		std::vector<Step> sharps = ks.sharps();
		std::vector<Step> flats = ks.flats();
		for (int istep = 0; istep <= 7; ++istep) {
			Step step = static_cast<Step> (istep);
			int alter = ks.alter_for_step (step);
			if (std::find (sharps.begin(), sharps.end(), step) != sharps.end()) {
				EXPECT_EQ (alter, 1);
			} else if (std::find (flats.begin(), flats.end(), step) != flats.end()) {
				EXPECT_EQ (alter, -1);
			} else {
				EXPECT_EQ (alter, 0);
			}
		}
	}

	KeySignature ks (3);
	EXPECT_EQ (ks.alter_for_step (Step::C), 1);
	EXPECT_EQ (ks.alter_for_step (Step::D), 0);
	EXPECT_EQ (ks.alter_for_step (Step::E), 0);
	EXPECT_EQ (ks.alter_for_step (Step::F), 1);
	EXPECT_EQ (ks.alter_for_step (Step::G), 1);
	EXPECT_EQ (ks.alter_for_step (Step::A), 0);
	EXPECT_EQ (ks.alter_for_step (Step::B), 0);
}

TEST (KeySignatureTest, accidentalFromPitch)
{
	KeySignature ks (-2);
	EXPECT_EQ (ks.accidental_from_pitch (Pitch (3, Step::C)), Accidental::kNone);
	EXPECT_EQ (ks.accidental_from_pitch (Pitch (4, Step::C, 1)), Accidental::kSharp);
	EXPECT_EQ (ks.accidental_from_pitch (Pitch (4, Step::D)), Accidental::kNone);
	EXPECT_EQ (ks.accidental_from_pitch (Pitch (4, Step::D, -1)), Accidental::kFlat);
	EXPECT_EQ (ks.accidental_from_pitch (Pitch (2, Step::E)), Accidental::kNatural);
	EXPECT_EQ (ks.accidental_from_pitch (Pitch (2, Step::E, -1)), Accidental::kNone);
}

} // namespace Engrave
