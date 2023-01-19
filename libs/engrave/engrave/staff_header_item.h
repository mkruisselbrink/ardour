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

#ifndef ENGRAVE_STAFF_HEADER_ITEM_H_
#define ENGRAVE_STAFF_HEADER_ITEM_H_

#include "canvas/container.h"
#include "engrave/clef.h"
#include "engrave/key_signature.h"
#include "engrave/time_signature.h"

namespace Engrave {

class RenderContext;

class StaffHeaderItem : public ArdourCanvas::Container {
public:
	StaffHeaderItem (const RenderContext &context, ArdourCanvas::Item *parent);

	void set_clef (const Clef &clef);
	void set_key_signature (const KeySignature &key_signature);
    void set_time_signature(const TimeSignature& time_signature);

private:
	const RenderContext &_context;
	std::unique_ptr<Clef> _clef;
	std::unique_ptr<KeySignature> _key_signature;
    std::unique_ptr<TimeSignature> _time_signature;

	std::unique_ptr<ArdourCanvas::Item> _clef_item;
	std::vector<std::unique_ptr<ArdourCanvas::Item> > _key_signature_items;
	std::unique_ptr<ArdourCanvas::Item> _time_signature_item;
};

} // namespace Engrave

#endif // ENGRAVE_STAFF_HEADER_ITEM_H_
