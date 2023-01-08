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

#ifndef __gtk_ardour_midi_score_bar_h__
#define __gtk_ardour_midi_score_bar_h__

#include "canvas/item.h"

class MidiScoreStreamView;

class MidiScoreBar : public ArdourCanvas::Item
{
public:
	MidiScoreBar (MidiScoreStreamView &view, ArdourCanvas::Item *parent, const ArdourCanvas::Duple &position,
	              double width);

	void line_distance_changed();
	void set_width (double w);

	void render (const ArdourCanvas::Rect &area, Cairo::RefPtr<Cairo::Context> cr) const override;
	void compute_bounding_box() const override;

private:
	MidiScoreStreamView &_view;
	double _width;
};

#endif // __gtk_ardour_midi_score_bar_h__
