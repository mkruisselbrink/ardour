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

#ifndef ENGRAVE_TIME_SIGNATURE_H_
#define ENGRAVE_TIME_SIGNATURE_H_

#include "temporal/tempo.h"

namespace Engrave {

class TimeSignature {
public:
	TimeSignature (const Temporal::Meter &meter) : TimeSignature (meter.divisions_per_bar(), meter.note_value()) {}
	TimeSignature (int8_t divisions_per_bar, int8_t note_value);

    int divisions_per_bar() const { return _divisions_per_bar; }
    int note_value() const { return _note_value; }

private:
	int8_t _divisions_per_bar;
	int8_t _note_value;
};

} // namespace Engrave

#endif // ENGRAVE_TIME_SIGNATURE_H_
