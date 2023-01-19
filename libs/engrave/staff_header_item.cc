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

#include "engrave/staff_header_item.h"

#include "engrave/clef_item.h"
#include "engrave/glyph_item.h"
#include "engrave/render_context.h"
#include "engrave/time_signature_item.h"

namespace Engrave {

StaffHeaderItem::StaffHeaderItem (const RenderContext &context, ArdourCanvas::Item *parent)
    : ArdourCanvas::Container (parent), _context (context)
{
}

void
StaffHeaderItem::set_clef (const Clef &clef)
{
	_clef = std::make_unique<Clef> (clef);
	_clef_item = std::make_unique<ClefItem> (_context, this, *_clef);
	_clef_item->set_position ({ 0, 0 });
}

void
StaffHeaderItem::set_key_signature (const KeySignature &key_signature)
{
	_key_signature = std::make_unique<KeySignature> (key_signature);
	_key_signature_items.clear();

	double x = _clef_item->bounding_box().x1 + _context.line_distance();
	std::vector<Step> sharps = _key_signature->sharps();
	for (Step sharp : sharps) {
		std::cerr << "Sharp at " << sharp << std::endl;
		int pos = _clef->position_for_step (sharp, /*for_sharp=*/true);
		auto item = std::make_unique<GlyphItem> (_context, this, Glyph::kAccidentalSharp);
		item->set_position ({ x, -_context.line_distance() / 2 * pos });
		x += item->width();
		_key_signature_items.push_back (std::move (item));
	}
	std::vector<Step> flats = _key_signature->flats();
	for (Step flat : flats) {
		std::cerr << "Flat at " << flat << std::endl;
		int pos = _clef->position_for_step (flat, /*for_sharp=*/false);
		auto item = std::make_unique<GlyphItem> (_context, this, Glyph::kAccidentalFlat);
		item->set_position ({ x, -_context.line_distance() / 2 * pos });
		x += item->width();
		_key_signature_items.push_back (std::move (item));
	}
}

void
StaffHeaderItem::set_time_signature (const TimeSignature &time_signature)
{
	_time_signature_item = std::make_unique<TimeSignatureItem> (_context, this, time_signature);
	_time_signature_item->set_x_position (_key_signature_items.back()->bounding_box().x1
	                                      + _key_signature_items.back()->position().x + _context.line_distance());
}

} // namespace Engrave
