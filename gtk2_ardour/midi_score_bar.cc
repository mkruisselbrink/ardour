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

#include "midi_score_bar.h"

#include "smufl/glyph.h"

#include "midi_score_streamview.h"

MidiScoreBar::MidiScoreBar (MidiScoreStreamView &view, ArdourCanvas::Item *parent, const ArdourCanvas::Duple &position,
                            double width)
    : ArdourCanvas::Item (parent, position), _view (view), _width (width)
{
}

void
MidiScoreBar::line_distance_changed()
{
	// TODO: bounding box should be full height, so this shouldn't effect bounding box
	begin_change();
	compute_bounding_box();
	end_change();
}

void
MidiScoreBar::set_width (double w)
{
	if (w == _width)
		return;
	begin_change();
	_width = w;
	compute_bounding_box();
	end_change();
}

void
MidiScoreBar::render (const ArdourCanvas::Rect &area, Cairo::RefPtr<Cairo::Context> cr) const
{
	ArdourCanvas::Rect self = item_to_window (ArdourCanvas::Rect (0, -_view.line_distance() * 4, _width, 0));
	ArdourCanvas::Rect isect = self.intersection (area);
	if (!isect)
		return;

	cr->set_source_rgb (0, 0, 0);
	cr->set_line_width (1);
	cr->move_to (self.x1 - 0.5, self.y0);
	cr->line_to (self.x1 - 0.5, self.y1);
	cr->stroke();

	cr->select_font_face ("Leland", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
	cr->set_font_size (_view.line_distance() * 4);

	{
		Cairo::TextExtents extents;
		std::string s = SMuFL::GlyphAsUTF8 (SMuFL::Glyph::kRestWhole);
		cr->get_text_extents (s, extents);
		cr->move_to ((self.x1 - self.x0) / 2 + self.x0 - extents.width / 2,
		             self.y1 - _view.line_distance() * 3);
		cr->show_text (s);
	}
}

void
MidiScoreBar::compute_bounding_box() const
{
	_bounding_box = { 0, -_view.line_distance() * 4, _width, 0 };
	set_bbox_clean();
}
