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

#include "score/key_signature.h"

#include <cassert>
#include <cstdlib>

namespace Score {

namespace {
	constexpr uint8_t kFirstSharp = 5; // F
	constexpr uint8_t kSharpOffset = 7;
	constexpr uint8_t kFirstFlat = 11; // B
	constexpr uint8_t kFlatOffset = 5;

	bool
	is_natural (uint8_t note)
	{
		note = note % 12;
		return note == 0 || note == 2 || note == 4 || note == 5 || note == 7 || note == 9 || note == 11;
	}

	namespace N {
		using Ac = KeySignature::Accidental;
		using NAA = KeySignature::NoteAndAccidentals;
		constexpr NAA C{ 0, Ac::kNone };
		constexpr NAA Cn{ 0, Ac::kNatural };
		constexpr NAA Cs{ 0, Ac::kSharp };
		constexpr NAA Db{ 2, Ac::kFlat };
		constexpr NAA D{ 2, Ac::kNone };
		constexpr NAA Dn{ 2, Ac::kNatural };
		constexpr NAA Ds{ 2, Ac::kSharp };
		constexpr NAA Eb{ 4, Ac::kFlat };
		constexpr NAA E{ 4, Ac::kNone };
		constexpr NAA En{ 4, Ac::kNatural };
		constexpr NAA F{ 5, Ac::kNone };
		constexpr NAA Fn{ 5, Ac::kNatural };
		constexpr NAA Fs{ 5, Ac::kSharp };
		constexpr NAA Gb{ 7, Ac::kFlat };
		constexpr NAA G{ 7, Ac::kNone };
		constexpr NAA Gn{ 7, Ac::kNatural };
		constexpr NAA Gs{ 7, Ac::kSharp };
		constexpr NAA Ab{ 9, Ac::kFlat };
		constexpr NAA A{ 9, Ac::kNone };
		constexpr NAA An{ 9, Ac::kNatural };
		constexpr NAA As{ 9, Ac::kSharp };
		constexpr NAA Bb{ 11, Ac::kFlat };
		constexpr NAA B{ 11, Ac::kNone };
		constexpr NAA Bn{ 11, Ac::kNatural };
	}
	using namespace N;
	// TODO: this doesn't need to be hardcoded
	constexpr KeySignature::NoteAndAccidentals kScales[15][12] = {
		/*Cb*/ { Cn, D, Dn, E, F, Fn, G, Gn, A, An, B, C }, // Fb
		/*Gb*/ { Cn, D, Dn, E, En, F, G, Gn, A, An, B, C }, // Cb
		/*Db*/ { C, D, Dn, E, En, F, G, Gn, A, An, B, Bn }, // Gb
		/*Ab*/ { C, D, Dn, E, En, F, Gb, G, A, An, B, Bn }, // Db
		/*Eb*/ { C, Db, D, E, En, F, Gb, G, A, An, B, Bn }, // Ab
		/*Bb*/ { C, Db, D, E, En, F, Gb, G, Ab, A, B, Bn }, // Eb
		/*F */ { C, Db, D, Eb, E, F, Gb, G, Ab, A, B, Bn }, // Bb
		/*C */ { C, Cs, D, Eb, E, F, Fs, G, Ab, A, Bb, B },
		/*G */ { C, Cs, D, Ds, E, Fn, F, G, Gs, A, As, B }, // F#
		/*D */ { Cn, C, D, Ds, E, Fn, F, G, Gs, A, As, B }, // C#
		/*A */ { Cn, C, D, Ds, E, Fn, F, Gn, G, A, As, B }, // G#
		/*E */ { Cn, C, Dn, D, E, Fn, F, Gn, G, A, As, B }, // D#
		/*B */ { Cn, C, Dn, D, E, Fn, F, Gn, G, An, A, B }, // A#
		/*F#*/ { Cn, C, Dn, D, En, E, F, Gn, G, An, A, B }, // E#
		/*C#*/ { B, C, Dn, D, En, E, F, Gn, G, An, A, Bn }, // B#
	};
}

KeySignature::KeySignature (int flats_or_sharps, Scale scale) : _flats_or_sharps (flats_or_sharps), _scale (scale)
{
	assert (-7 <= flats_or_sharps && flats_or_sharps <= 7);
	for (int i = 0; i != 12; ++i)
		{
			_notes[i] = kScales[flats_or_sharps + 7][i];
		}
}

uint8_t
KeySignature::base_note() const
{
	return (12 * 7 + (_scale == Scale::kMinor ? 9 : 0) + _flats_or_sharps * kSharpOffset) % 12;
}

int
KeySignature::flats_or_sharps_count() const
{
	return _flats_or_sharps;
}

std::vector<uint8_t>
KeySignature::sharps() const
{
	if (_flats_or_sharps <= 0)
		{
			return {};
		}
	std::vector<uint8_t> result;
	uint8_t note = kFirstSharp;
	for (int i = 0; i < _flats_or_sharps; ++i)
		{
			result.push_back (note);
			note = (note + kSharpOffset) % 12;
		}
	return result;
}

std::vector<uint8_t>
KeySignature::flats() const
{
	if (_flats_or_sharps >= 0)
		{
			return {};
		}
	std::vector<uint8_t> result;
	uint8_t note = kFirstFlat;
	for (int i = 0; i < -_flats_or_sharps; ++i)
		{
			result.push_back (note);
			note = (note + kFlatOffset) % 12;
		}
	return result;
}

KeySignature::NoteAndAccidentals
KeySignature::note_and_accidentals_to_render (uint8_t note) const
{
	NoteAndAccidentals result = _notes[note % 12];
	// Make sure we're int he right octave.
	result.note += (note / 12) * 12;
	// Wrapping around could get us off-by-one, so correct for that.
	if (result.note > note + 1)
		{
			if (result.note > 12)
				{
					result.note -= 12;
				}
			else
				{
					// TODO: maybe not used unsigned 8-bit ints for notes
					// This must be note 0 trying to be rendered as note -1. Instead render as a C
					// with a natural accidental.
					result.note = 0;
					result.accidental = Accidental::kNatural;
				}
		}
	if (result.note < note - 1)
		{
			result.note += 12;
		}
	return result;
}

} // namespace Score
