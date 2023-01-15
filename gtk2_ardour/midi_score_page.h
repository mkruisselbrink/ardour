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

#ifndef __gtk_ardour_midi_score_page_h__
#define __gtk_ardour_midi_score_page_h__

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>

#include "pbd/signals.h"

#include "ardour/session_handle.h"

#include "widgets/tabbable.h"
#include "engrave/render_context.h"

namespace ArdourCanvas {
class GtkCanvas;
class GtkCanvasViewport;
class LineSet;
class ScrollGroup;
} // namespace ArdourCanvas

namespace Engrave {
class StaffLineSet;
} // namespace Engrave

class MidiScorePage : public ArdourWidgets::Tabbable,
		      public ARDOUR::SessionHandlePtr,
		      public PBD::ScopedConnectionList {
public:
	MidiScorePage();
	~MidiScorePage() override;

	void set_session (ARDOUR::Session *) override;

	bool canvas_scroll_event (GdkEventScroll *event, bool from_canvas);

private:
	Gtk::HBox _content;
	Gtk::Adjustment _vertical_adjustment;
	Gtk::Adjustment _horizontal_adjustment;
	std::unique_ptr<ArdourCanvas::GtkCanvasViewport> _canvas_viewport;
	ArdourCanvas::GtkCanvas *_canvas = nullptr;

	std::unique_ptr<ArdourCanvas::ScrollGroup> _hv_scroll_group;
	std::unique_ptr<ArdourCanvas::ScrollGroup> _v_scroll_group;

    Engrave::RenderContext _render_context;

	std::unique_ptr<Engrave::StaffLineSet> _staff_lines;

};

#endif // __gtk_ardour_midi_score_page_h__
