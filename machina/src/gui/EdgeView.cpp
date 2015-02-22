/*
  This file is part of Machina.
  Copyright 2007-2013 David Robillard <http://drobilla.net>

  Machina is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Machina is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Machina.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ganv/Canvas.hpp"

#include "machina/Controller.hpp"
#include "machina/types.hpp"

#include "EdgeView.hpp"
#include "MachinaCanvas.hpp"
#include "MachinaGUI.hpp"
#include "NodeView.hpp"

namespace machina {
namespace gui {

/* probability colour stuff */

#define RGB_TO_UINT(r, g, b) ((((guint)(r)) << 16) | (((guint)(g)) << 8) | ((guint)(b)))
#define RGB_TO_RGBA(x, a) (((x) << 8) | ((((guint)a) & 0xff)))
#define RGBA_TO_UINT(r, g, b, a) RGB_TO_RGBA(RGB_TO_UINT(r, g, b), a)

#define UINT_RGBA_R(x) (((uint32_t)(x)) >> 24)
#define UINT_RGBA_G(x) ((((uint32_t)(x)) >> 16) & 0xff)
#define UINT_RGBA_B(x) ((((uint32_t)(x)) >> 8) & 0xff)
#define UINT_RGBA_A(x) (((uint32_t)(x)) & 0xff)

#define MONO_INTERPOLATE(v1, v2, t) ((int)rint((v2) * (t) + (v1) * (1 - (t))))

#define UINT_INTERPOLATE(c1, c2, t) \
	RGBA_TO_UINT(MONO_INTERPOLATE(UINT_RGBA_R(c1), UINT_RGBA_R(c2), t), \
	             MONO_INTERPOLATE(UINT_RGBA_G(c1), UINT_RGBA_G(c2), t), \
	             MONO_INTERPOLATE(UINT_RGBA_B(c1), UINT_RGBA_B(c2), t), \
	             MONO_INTERPOLATE(UINT_RGBA_A(c1), UINT_RGBA_A(c2), t) )

inline static uint32_t edge_color(float prob)
{
	static const uint32_t min = 0xFF4444FF;
	static const uint32_t mid = 0xFFFF44FF;
	static const uint32_t max = 0x44FF44FF;

	if (prob <= 0.5) {
		return UINT_INTERPOLATE(min, mid, prob * 2.0);
	} else {
		return UINT_INTERPOLATE(mid, max, (prob - 0.5) * 2.0);
	}
}

/* end probability colour stuff */

using namespace Ganv;

EdgeView::EdgeView(Canvas&                             canvas,
                   NodeView*                           src,
                   NodeView*                           dst,
                   SPtr<machina::client::ClientObject> edge)
	: Ganv::Edge(canvas, src, dst, 0x9FA0A0FF, true, false)
	, _edge(edge)
{
	set_color(edge_color(probability()));

	edge->signal_property.connect(
		sigc::mem_fun(this, &EdgeView::on_property));

	signal_event().connect(
		sigc::mem_fun(this, &EdgeView::on_event));
}

EdgeView::~EdgeView()
{
	_edge->set_view(NULL);
}

float
EdgeView::probability() const
{
	return _edge->get(URIs::instance().machina_probability).get<float>();
}

double
EdgeView::length_hint() const
{
	NodeView* tail = dynamic_cast<NodeView*>(get_tail());
	return tail->node()->get(URIs::instance().machina_duration).get<float>()
		* 10.0;
}

void
EdgeView::show_label(bool show)
{
	set_color(edge_color(probability()));
}

bool
EdgeView::on_event(GdkEvent* ev)
{
	MachinaCanvas* canvas = dynamic_cast<MachinaCanvas*>(this->canvas());
	Forge&         forge  = canvas->app()->forge();

	if (ev->type == GDK_BUTTON_PRESS) {
		if (ev->button.state & GDK_CONTROL_MASK) {
			if (ev->button.button == 1) {
				canvas->app()->controller()->set_property(
					_edge->id(),
					URIs::instance().machina_probability,
					forge.make(float(probability() - 0.1f)));
				return true;
			} else if (ev->button.button == 3) {
				canvas->app()->controller()->set_property(
					_edge->id(),
					URIs::instance().machina_probability,
					forge.make(float(probability() + 0.1f)));
				return true;
			}
		}
	}
	return false;
}

void
EdgeView::on_property(machina::URIInt key, const Atom& value)
{
	if (key == URIs::instance().machina_probability) {
		set_color(edge_color(value.get<float>()));
	}
}

}  // namespace machina
}  // namespace gui
