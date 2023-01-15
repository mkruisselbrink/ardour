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

#include "engrave/staff_line_set.h"

#include "engrave/render_context.h"

namespace Engrave {

StaffLineSet::StaffLineSet (const RenderContext &context, ArdourCanvas::Item *parent)
    : ArdourCanvas::LineSet (parent, Horizontal), _context (context)
{
	set_extent (ArdourCanvas::COORD_MAX);
}

void
StaffLineSet::add_staff (ArdourCanvas::Coord bottom_line_pos, int line_count)
{
	const double line_thickness = _context.staff_line_thickness();
	for (int i = 0; i < line_count; ++i) {
		add_coord (bottom_line_pos - _context.line_distance() * i, line_thickness, 0x000000ff);
	}
}

} // namespace Engrave
