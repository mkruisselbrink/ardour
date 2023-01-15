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

#ifndef ENGRAVE_STAFF_LINE_SET_H
#define ENGRAVE_STAFF_LINE_SET_H

#include "canvas/line_set.h"

namespace Engrave {

class RenderContext;

class StaffLineSet : public ArdourCanvas::LineSet {
public:
	StaffLineSet (const RenderContext &context, ArdourCanvas::Item *parent);

	void add_staff (ArdourCanvas::Coord bottom_line_pos, int line_count = 5);

private:
	const RenderContext &_context;
};

} // namespace Engrave

#endif // ENGRAVE_STAFF_LINE_SET_H
