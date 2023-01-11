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

#include "gtkmm2ext/colors.h"

MidiScorePage::MidiScorePage()
    : Tabbable (_content, _ ("Score"), X_ ("score")), _vertical_adjustment (0.0, 0.0, 1e16),
      _horizontal_adjustment (0.0, 0.0, 1e16)
{
	_canvas_viewport
	    = std::make_unique<ArdourCanvas::GtkCanvasViewport> (_horizontal_adjustment, _vertical_adjustment);
    _canvas = _canvas_viewport->canvas();

    _canvas->set_background_color(Gtkmm2ext::rgba_to_color(0.9, 0.9, 0.9, 1.0));
    _canvas->use_nsglview();

	_content.pack_start (*_canvas_viewport, /*expand=*/true, /*fill=*/true, /*padding=*/0);
}

MidiScorePage::~MidiScorePage() {}

void
MidiScorePage::set_session (ARDOUR::Session *s)
{
	SessionHandlePtr::set_session (s);
}
