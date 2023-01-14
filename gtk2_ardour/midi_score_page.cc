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

#include "gtkmm2ext/colors.h"

#include "score/clef.h"

#include "smufl/glyph.h"

#include "ui_config.h"

namespace ScoreCanvas {

class ScoreRendering {
public:
	static ScoreRendering &
	instance()
	{
		static ScoreRendering *r = new ScoreRendering();
		return *r;
	}

private:
	ScoreRendering() {}
};

class Glyph : public ArdourCanvas::Item {
public:
	Glyph (ArdourCanvas::Item *parent, SMuFL::Glyph g) : ArdourCanvas::Item (parent)
	{
		// TODO: no need to create a new Font instance for every glyph
		Pango::FontDescription font_desc = UIConfiguration::instance().get_ScoreFont();
		_font_face = Cairo::ToyFontFace::create (font_desc.get_family(), Cairo::FONT_SLANT_NORMAL,
		                                         Cairo::FONT_WEIGHT_NORMAL);
		_font
		    = Cairo::ScaledFont::create (_font_face, Cairo::scaling_matrix (32, 32), Cairo::identity_matrix());
		_text = SMuFL::GlyphAsUTF8 (g);
	}

	void
	render (ArdourCanvas::Rect const &area, Cairo::RefPtr<Cairo::Context> cr) const override
	{
		ArdourCanvas::Duple pos = item_to_window (ArdourCanvas::Duple (0, 0));
		cr->set_source_rgb (0, 0, 0);
		cr->set_scaled_font (_font);

		cr->move_to (pos.x, pos.y);
		cr->show_text (_text);
	}

	void
	compute_bounding_box() const override
	{
		Cairo::TextExtents extents;
		cairo_scaled_font_text_extents (const_cast<Cairo::ScaledFont::cobject *> (_font->cobj()),
		                                _text.c_str(), &extents);
		_bounding_box = { extents.x_bearing, extents.y_bearing, extents.x_bearing + extents.width,
			          extents.y_bearing + extents.height };
		set_bbox_clean();
	}

private:
	std::string _text;
	Cairo::RefPtr<Cairo::FontFace> _font_face;
	Cairo::RefPtr<Cairo::ScaledFont> _font;
};

class Clef : public Glyph {
public:
	Clef (ArdourCanvas::Item *parent, const Score::Clef &clef) : Glyph (parent, clef.glyph)
	{
		/*		set_font_description (UIConfiguration::instance().get_ScoreFont());
		                std::cerr << "Font size: " << UIConfiguration::instance().get_ScoreFont().get_size() <<
		   std::endl; set_adjust_for_baseline (true); set (SMuFL::GlyphAsUTF8 (clef.glyph));*/
	}

private:
};

} // namespace ScoreCanvas

MidiScorePage::MidiScorePage()
    : Tabbable (_content, _ ("Score"), X_ ("score")), _vertical_adjustment (0.0, 0.0, 1e16),
      _horizontal_adjustment (0.0, 0.0, 1e16)
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

	r = new ArdourCanvas::Rectangle (_hv_scroll_group.get(), ArdourCanvas::Rect (100.0, 0.0, 250.0, 250.0));
	CANVAS_DEBUG_NAME (r, "rect2");
	r->set_fill (true);
	r->set_outline (true);
	r->set_outline_color (Gtkmm2ext::rgba_to_color (0, 0, 0, 1.0));
	r->set_fill_color (Gtkmm2ext::rgba_to_color (0.0, 0.0, 1.0, 0.2));
	r->show();

	_staff_lines = new ArdourCanvas::LineSet (_v_scroll_group.get(), ArdourCanvas::LineSet::Horizontal);
	_staff_lines->set_extent (ArdourCanvas::COORD_MAX);
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

		auto *clef = new ScoreCanvas::Clef (_v_scroll_group.get(), Score::Clef::treble_clef);
		clef->set_position ({ 20, y - 8 });
		clef = new ScoreCanvas::Clef (_v_scroll_group.get(), Score::Clef::bass_clef);
		clef->set_position ({ 50, y - 24 });

		auto *g = new ScoreCanvas::Glyph (_v_scroll_group.get(), SMuFL::Glyph::kCClef);
		g->set_position ({ 80, y - 16 });

		_staff_lines->add_coord (y, 1.0, 0x000000ff);
		_staff_lines->add_coord (y - 8, 1.0, 0x000000ff);
		_staff_lines->add_coord (y - 16, 1.0, 0x000000ff);
		_staff_lines->add_coord (y - 24, 1.0, 0x000000ff);
		_staff_lines->add_coord (y - 32, 1.0, 0x000000ff);

		y += 100;
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
