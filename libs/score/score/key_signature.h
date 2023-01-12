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

#ifndef SCORE_KEY_SIGNATURE_H_
#define SCORE_KEY_SIGNATURE_H_

#include <cstdint>
#include <vector>

namespace Score {

enum class Accidental
{
	kNone,
	kNatural,
	kSharp,
	kFlat
};

// Representation of a Key Signature. For now only supports "simple" major and
// minor scales with at most 7 sharps or flat.
class KeySignature
{
public:
    using Accidental = Score::Accidental;
	enum class Scale
	{
		kMajor,
		kMinor
	};

	// flats_or_sharps should be in -7 - 7, negative numbers for flats, positive for sharps
	KeySignature (int flats_or_sharps, Scale scale = Scale::kMajor);

	// Base-octave midi note for the root of this key signature.
	uint8_t base_note() const;

	// Number of sharps or flats as passed in.
	int flats_or_sharps_count() const;

	// Base-octave midi notes for all the notes with sharps in this key signature. For example if F# is one, this
	// would return F.
	std::vector<uint8_t> sharps() const;
	// Same but for flats.
	std::vector<uint8_t> flats() const;

	// Given a midi note `note`, returns the natural note to render as well as any accidentals to render in this
	// key signature.
	struct NoteAndAccidentals
	{
		uint8_t note = 0;
		Accidental accidental = Accidental::kNone;
	};
	NoteAndAccidentals note_and_accidentals_to_render (uint8_t note) const;

private:
	int _flats_or_sharps;
	Scale _scale;
	NoteAndAccidentals _notes[12];
};

} // namespace Score

#endif // SCORE_KEY_SIGNATURE_H_
