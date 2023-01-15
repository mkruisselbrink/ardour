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

#include "midi_score_page.h"

#include <gtkmm/action.h>

#include <cairomm/fontface.h>

#include "pbd/i18n.h"

#include "ardour/midi_track.h"
#include "ardour/session.h"
#include "ardour/types.h"

#include "canvas/canvas.h"
#include "canvas/debug.h"
#include "canvas/line_set.h"
#include "canvas/rectangle.h"
#include "canvas/scroll_group.h"
#include "canvas/text.h"
#include "engrave/clef.h"
#include "engrave/clef_item.h"
#include "engrave/glyph.h"
#include "engrave/glyph_item.h"
#include "engrave/staff_line_set.h"
#include "gtkmm2ext/colors.h"

#include "ui_config.h"

MidiScorePage::MidiScorePage()
    : Tabbable (_content, _ ("Score"), X_ ("score")), _vertical_adjustment (0.0, 0.0, 1e16),
      _horizontal_adjustment (0.0, 0.0, 1e16),
      _render_context (UIConfiguration::instance().get_ScoreFont().get_family(), 10)
{
	_canvas_viewport
	    = std::make_unique<ArdourCanvas::GtkCanvasViewport> (_horizontal_adjustment, _vertical_adjustment);
	_canvas = _canvas_viewport->canvas();

	_canvas->set_background_color (Gtkmm2ext::rgba_to_color (0.9, 0.9, 0.85, 1.0));
	_canvas->use_nsglview();

	_canvas->set_name ("ScoreMainCanvas");
	_canvas->set_debug_render (true);
	_canvas->add_events (Gdk::POINTER_MOTION_HINT_MASK | Gdk::SCROLL_MASK | Gdk::KEY_PRESS_MASK
	                     | Gdk::KEY_RELEASE_MASK);

	_canvas->signal_scroll_event().connect (
	    sigc::bind (sigc::mem_fun (*this, &MidiScorePage::canvas_scroll_event), true));

	_content.pack_start (*_canvas_viewport, /*expand=*/true, /*fill=*/true, /*padding=*/0);

	_hv_scroll_group = std::make_unique<ArdourCanvas::ScrollGroup> (
	    _canvas->root(),
	    static_cast<ArdourCanvas::ScrollGroup::ScrollSensitivity> (
		ArdourCanvas::ScrollGroup::ScrollsHorizontally | ArdourCanvas::ScrollGroup::ScrollsVertically));
	CANVAS_DEBUG_NAME (_hv_scroll_group.get(), "canvas hv scroll");
	_canvas->add_scroller (*_hv_scroll_group);

	_v_scroll_group = std::make_unique<ArdourCanvas::ScrollGroup> (_canvas->root(),
	                                                               ArdourCanvas::ScrollGroup::ScrollsVertically);
	CANVAS_DEBUG_NAME (_v_scroll_group.get(), "canvas v scroll");
	_canvas->add_scroller (*_v_scroll_group);

	auto *r = new ArdourCanvas::Rectangle (_v_scroll_group.get(),
	                                       ArdourCanvas::Rect (0.0, 0.0, 100.0, ArdourCanvas::COORD_MAX));
	CANVAS_DEBUG_NAME (r, "rect");
	r->set_fill (true);
	r->set_outline (true);
	r->set_outline_color (Gtkmm2ext::rgba_to_color (0, 0, 0, 1.0));
	r->set_fill_color (Gtkmm2ext::rgba_to_color (1.0, 0.0, 0.0, 0.2));
	r->show();

	r = new ArdourCanvas::Rectangle (_hv_scroll_group.get(),
	                                 ArdourCanvas::Rect (100.0, 0.0, 200, ArdourCanvas::COORD_MAX));
	CANVAS_DEBUG_NAME (r, "rect2");
	r->set_fill (true);
	// r->set_outline (true);
	r->set_outline_color (Gtkmm2ext::rgba_to_color (0, 0, 0, 1.0));
	r->set_fill_color (Gtkmm2ext::rgba_to_color (0.0, 0.0, 1.0, 0.1));
	r->show();

	_staff_lines = std::make_unique<Engrave::StaffLineSet> (_render_context, _v_scroll_group.get());
	_staff_lines->set_x_position (20);
}

MidiScorePage::~MidiScorePage() {}

void
MidiScorePage::set_session (ARDOUR::Session *s)
{
	std::cerr << "Setting session" << std::endl;
	SessionHandlePtr::set_session (s);

	if (!s) {
		return;
	}

	double y = 90;

	boost::shared_ptr<ARDOUR::RouteList> tracks = s->get_tracks();
	for (const auto &t : *tracks) {
		auto midi_track = boost::dynamic_pointer_cast<ARDOUR::MidiTrack> (t);
		if (!midi_track) {
			std::cerr << "Not a midi track: " << t->name() << std::endl;
			continue;
		}

		std::cerr << "Name: " << midi_track->name() << std::endl;
		auto *txt = new ArdourCanvas::Text (_v_scroll_group.get());
		txt->set (midi_track->name());
		txt->set_position ({ 0, y });
		txt->show();

		auto *clef
		    = new Engrave::ClefItem (_render_context, _v_scroll_group.get(), Engrave::Clef::treble_clef);
		clef->set_position ({ 20, y });
		clef = new Engrave::ClefItem (_render_context, _v_scroll_group.get(), Engrave::Clef::bass_clef);
		clef->set_position ({ 50, y });

		auto *g = new Engrave::GlyphItem (_render_context, _v_scroll_group.get(), Engrave::Glyph::kCClef);
		g->set_position ({ 80, y - _render_context.line_distance() * 2 });

		_staff_lines->add_staff (y);

		y += 120;
	}
}

bool
MidiScorePage::canvas_scroll_event (GdkEventScroll *ev, bool from_canvas)
{
	switch (ev->direction) {
	case GDK_SCROLL_UP:
		_vertical_adjustment.set_value (_vertical_adjustment.get_value() - 32);
		return true;
	case GDK_SCROLL_DOWN:
		_vertical_adjustment.set_value (_vertical_adjustment.get_value() + 32);
		return true;
	case GDK_SCROLL_LEFT:
		_horizontal_adjustment.set_value (_horizontal_adjustment.get_value() - 32);
		return true;
	case GDK_SCROLL_RIGHT:
		_horizontal_adjustment.set_value (_horizontal_adjustment.get_value() + 32);
		return true;
	}
	return false;
}
