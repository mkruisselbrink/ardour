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

#ifndef ENGRAVE_KEY_SIGNATURE_H_
#define ENGRAVE_KEY_SIGNATURE_H_

#include <cstdint>
#include <vector>

#include "engrave/types.h"
#include "engrave/pitch.h"

namespace Engrave {

// Representation of a Key Signature. For now only supports "simple" major and
// minor scales with at most 7 sharps or flat.
class KeySignature
{
public:
	// flats_or_sharps should be in -7 - 7, negative numbers for flats, positive for sharps
	KeySignature (int flats_or_sharps = 0, Scale scale = Scale::kMajor);

	// Base-octave midi note for the root of this key signature.
	uint8_t base_note() const;

	// Number of sharps or flats as passed in.
	int flats_or_sharps_count() const;

	const std::vector<Step>& sharps() const {
		return _sharps;
	}
	
	const std::vector<Step>& flats() const {
		return _flats;
	}

	Pitch pitch_from_midi_note (uint8_t midi_note) const;

	int alter_for_step(Step s) const;

	Accidental accidental_from_pitch (const Pitch& p) const;

private:
	int _flats_or_sharps;
	Scale _scale;
	std::vector<Step> _sharps;
	std::vector<Step> _flats;
	Pitch _pitches[12];
};

} // namespace Engrave

#endif // ENGRAVE_KEY_SIGNATURE_H_
