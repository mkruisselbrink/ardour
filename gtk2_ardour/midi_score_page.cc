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

#include "pbd/i18n.h"

#include "canvas/canvas.h"
#include "canvas/debug.h"
#include "canvas/rectangle.h"
#include "canvas/scroll_group.h"

#include "gtkmm2ext/colors.h"

MidiScorePage::MidiScorePage()
    : Tabbable (_content, _ ("Score"), X_ ("score")), _vertical_adjustment (0.0, 0.0, 1e16),
      _horizontal_adjustment (0.0, 0.0, 1e16)
{
	_canvas_viewport
	    = std::make_unique<ArdourCanvas::GtkCanvasViewport> (_horizontal_adjustment, _vertical_adjustment);
	_canvas = _canvas_viewport->canvas();

	_canvas->set_background_color (Gtkmm2ext::rgba_to_color (0.9, 0.9, 0.7, 1.0));
	_canvas->use_nsglview();

	_canvas->set_name ("ScoreMainCanvas");
	_canvas->add_events (Gdk::POINTER_MOTION_HINT_MASK | Gdk::SCROLL_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

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
	                                       ArdourCanvas::Rect (0.0, 0.0, 10.0, ArdourCanvas::COORD_MAX));
	CANVAS_DEBUG_NAME (r, "rect");
	r->set_fill (true);
	r->set_outline (true);
	r->set_outline_color (Gtkmm2ext::rgba_to_color (0, 0, 0, 1.0));
	r->set_fill_color (Gtkmm2ext::rgba_to_color (1.0, 0.0, 0.0, 1.0));
	r->show();

	r = new ArdourCanvas::Rectangle (_hv_scroll_group.get(), ArdourCanvas::Rect (50.0, 50.0, 250.0, 250.0));
	CANVAS_DEBUG_NAME (r, "rect2");
	r->set_fill (true);
	r->set_outline (true);
	r->set_outline_color (Gtkmm2ext::rgba_to_color (0, 0, 0, 1.0));
	r->set_fill_color (Gtkmm2ext::rgba_to_color (0.0, 0.0, 1.0, 1.0));
	r->show();
}

MidiScorePage::~MidiScorePage() {}

void
MidiScorePage::set_session (ARDOUR::Session *s)
{
	SessionHandlePtr::set_session (s);
}
