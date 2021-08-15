/*
 * Copyright (C) 2021 Marijn Kruisselbrink <mek@google.com>
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

#include "midi_score_time_axis.h"

#include "pbd/i18n.h"

#include "ardour/stripable.h"

#include "canvas/container.h"

#include "widgets/tooltips.h"

#include "midi_score_streamview.h"
#include "utils.h"

MidiScoreTimeAxisView::MidiScoreTimeAxisView (
	ARDOUR::Session *s,
	boost::shared_ptr<ARDOUR::Stripable> strip,
	PublicEditor &e,
	TimeAxisView &parent,
	ArdourCanvas::Canvas &canvas)
	: TimeAxisView (s, e, &parent, canvas), _stripable (strip)
{
	_view = new MidiScoreStreamView(*this);
	_view->attach ();

	set_height(preset_height(HeightNormal));

	// name label isn't editable on a midi score track; remove the tooltip
	ArdourWidgets::set_tooltip(name_label, X_(""));
	name_label.set_text("Score");
	name_label.show();
}

MidiScoreTimeAxisView::~MidiScoreTimeAxisView () 
{
	delete _view;
}

void
MidiScoreTimeAxisView::set_height (uint32_t h, TrackHeightMode m)
{
	bool const changed = (height != (uint32_t) h) || _first_call_to_set_height;
	TimeAxisView::set_height (h, m);

	_view->set_height(h);
	_view->update_contents_height();

	_first_call_to_set_height = false;
	if (changed) {
		if (_canvas_display->visible() && _stripable) {
			/* only emit the signal if the height really changed and we were visible */
			_stripable->gui_changed ("visible_tracks", (void *) 0); /* EMIT_SIGNAL */
		}
	}
}

Gdk::Color MidiScoreTimeAxisView::color () const
{
	return ARDOUR_UI_UTILS::gdk_color_from_rgb (_stripable->presentation_info().color());
}

boost::shared_ptr<ARDOUR::Stripable> MidiScoreTimeAxisView::stripable () const
{
	return _stripable;
}

std::string MidiScoreTimeAxisView::state_id () const
{
	return "midi score";
}
