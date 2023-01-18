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

#include <algorithm>
#include <cassert>
#include <cstdlib>

namespace Engrave {

namespace {
	constexpr uint8_t kFirstSharp = 5; // F
	constexpr uint8_t kSharpOffset = 7;
	constexpr uint8_t kFirstFlat = 11; // B
	constexpr uint8_t kFlatOffset = 5;
}

KeySignature::KeySignature (int flats_or_sharps, Scale scale) : _flats_or_sharps (flats_or_sharps), _scale (scale)
{
	assert (-7 <= flats_or_sharps && flats_or_sharps <= 7);

	static constexpr uint8_t major_scale[7] = { 0, 2, 4, 5, 7, 9, 11 };
	for (int i = 0; i < 7; ++i) {
		_pitches[major_scale[i]] = Pitch (4, static_cast<Step> (i), 0);
	}
	static constexpr Step kOrderedSharps[7] = { Step::F, Step::C, Step::G, Step::D, Step::A, Step::E, Step::B };
	static constexpr Step kOrderedFlats[7] = { Step::B, Step::E, Step::A, Step::D, Step::G, Step::C, Step::F };

	_sharps.assign (kOrderedSharps, kOrderedSharps + std::max (0, _flats_or_sharps));
	_flats.assign (kOrderedFlats, kOrderedFlats - std::min (0, _flats_or_sharps));

	const Step *src = _flats_or_sharps < 0 ? kOrderedFlats : kOrderedSharps;
	int alter = _flats_or_sharps < 0 ? -1 : 1;
	// TODO: 5/6/7 flats/sharps if flats or sharps, 3 of both if C-major
	for (int i = 0; i < std::max (5, std::abs (_flats_or_sharps)); ++i) {
		int octave = 4;
		int n = major_scale[static_cast<int> (src[i])] + alter;
		if (n < 0) {
			n += 12;
			octave++;
		}
		if (n > 11) {
			n -= 12;
			octave--;
		}
		_pitches[n] = Pitch (octave, src[i], alter);
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

Pitch
KeySignature::pitch_from_midi_note (uint8_t midi_note) const
{
	int octave = (midi_note / 12) - 5;
	return _pitches[midi_note % 12].add_octaves (octave);
}

int
KeySignature::alter_for_step (Step s) const
{
	if (std::find (_sharps.begin(), _sharps.end(), s) != _sharps.end()) {
		return 1;
	}
	if (std::find (_flats.begin(), _flats.end(), s) != _flats.end()) {
		return -1;
	}
	return 0;
}

Accidental
KeySignature::accidental_from_pitch (const Pitch &p) const
{
	if (alter_for_step (p.step()) == p.alter()) {
		return Accidental::kNone;
	}
	return accidental_from_alter(p.alter());
}

} // namespace Engrave
