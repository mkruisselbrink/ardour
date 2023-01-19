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

#ifndef ENGRAVE_TIME_SIGNATURE_ITEM_H_
#define ENGRAVE_TIME_SIGNATURE_ITEM_H_

#include "canvas/container.h"
#include "engrave/time_signature.h"

namespace Engrave {

class RenderContext;

class TimeSignatureItem : public ArdourCanvas::Container {
public:
	TimeSignatureItem (const RenderContext &context, ArdourCanvas::Item *parent,
	                   const TimeSignature &time_signature);

private:
	std::vector<std::unique_ptr<ArdourCanvas::Item> > create_items_for_number (int number, int line_offset);

	const RenderContext &_context;
	std::vector<std::unique_ptr<ArdourCanvas::Item> > _items;
};

} // namespace Engrave

#endif // ENGRAVE_TIME_SIGNATURE_ITEM_H_
