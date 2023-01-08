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

#ifndef __gtk_ardour_midi_score_header_h__
#define __gtk_ardour_midi_score_header_h__

#include <gtkmm/drawingarea.h>

#include "temporal/tempo.h"

class MidiScoreStreamView;

class MidiScoreHeader : public Gtk::DrawingArea
{
public:
	MidiScoreHeader (MidiScoreStreamView &);

	bool on_expose_event (GdkEventExpose *) override;
	void on_size_request (Gtk::Requisition *) override;
	void on_size_allocate (Gtk::Allocation &a) override;

	void note_range_changed();
	void set_meter (const Temporal::Meter &meter);

private:
	MidiScoreStreamView &_view;
	Temporal::Meter _meter;
};

#endif // __gtk_ardour_midi_score_header_h__
