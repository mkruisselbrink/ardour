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

#ifndef ENGRAVE_TYPES_H_
#define ENGRAVE_TYPES_H_

#include <cassert>
#include <cstdint>
#include <vector>

namespace Engrave {

enum class Accidental { kNone, kNatural, kSharp, kFlat };

enum class Scale { kMajor, kMinor };

enum class Step {
	C = 0,
	D = 1,
	E = 2,
	F = 3,
	G = 4,
	A = 5,
	B = 6,
};

inline int
alter_for_accidental (Accidental a)
{
	switch (a) {
	case Accidental::kNone:
	case Accidental::kNatural:
		return 0;
	case Accidental::kSharp:
		return 1;
	case Accidental::kFlat:
		return -1;
	}
	assert (false);
	return 0;
}

inline Accidental
accidental_from_alter (int alter, Accidental treat_zero_as = Accidental::kNatural)
{
	switch (alter) {
	case -1:
		return Accidental::kFlat;
	case 0:
		return treat_zero_as;
	case 1:
		return Accidental::kSharp;
	}
	assert (false);
	return Accidental::kNone;
}

} // namespace Engrave

#endif // ENGRAVE_TYPES_H_
