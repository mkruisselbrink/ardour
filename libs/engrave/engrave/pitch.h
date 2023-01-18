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

#ifndef ENGRAVE_PITCH_H_
#define ENGRAVE_PITCH_H_

#include "engrave/types.h"

namespace Engrave {

// Octave 4 starts with middle C (i.e. midi note 60 would be octave=4,step=0,alter=0).
class Pitch {
public:
	constexpr Pitch() : Pitch (4, Step::C, 0) {}
	constexpr Pitch (int octave, Step step, int alter = 0) : _octave (octave), _step (step), _alter (alter) {}

	int octave() const { return _octave; }
	Step step() const { return _step; }
	int alter() const { return _alter; }

	uint8_t midi_note() const;

	Pitch add_octaves (int octave_delta) const { return Pitch (_octave + octave_delta, _step, _alter); }

private:
	int _octave;
	Step _step;
	int _alter;
};

inline int
operator- (const Pitch &a, const Pitch &b)
{
	return (static_cast<int> (a.step()) + a.octave() * 7) - (static_cast<int> (b.step()) + b.octave() * 7);
}

} // namespace Engrave

#endif // ENGRAVE_PITCH_H_
