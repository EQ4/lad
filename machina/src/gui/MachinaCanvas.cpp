/*
  This file is part of Machina.
  Copyright 2007-2014 David Robillard <http://drobilla.net>

  Machina is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Machina is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Machina.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <map>

#include "raul/TimeStamp.hpp"

#include "client/ClientModel.hpp"
#include "client/ClientObject.hpp"
#include "machina/Controller.hpp"
#include "machina/Engine.hpp"
#include "machina/types.hpp"

#include "EdgeView.hpp"
#include "MachinaCanvas.hpp"
#include "MachinaGUI.hpp"
#include "NodeView.hpp"

using namespace Raul;
using namespace Ganv;

namespace machina {
namespace gui {

MachinaCanvas::MachinaCanvas(MachinaGUI* app, int width, int height)
	: Canvas(width, height)
	, _app(app)
	, _connect_node(NULL)
	, _did_connect(false)
{
	widget().grab_focus();

	signal_event.connect(sigc::mem_fun(this, &MachinaCanvas::on_event));
}

void
MachinaCanvas::connect_nodes(GanvNode* node, void* data)
{
	MachinaCanvas* canvas = (MachinaCanvas*)data;
	NodeView*      view   = dynamic_cast<NodeView*>(Glib::wrap(node));
	if (!view || view == canvas->_connect_node) {
		return;
	}
	if (canvas->get_edge(view, canvas->_connect_node)) {
		canvas->action_disconnect(view, canvas->_connect_node);
		canvas->_did_connect = true;
	} else if (!canvas->get_edge(canvas->_connect_node, view)) {
		canvas->action_connect(view, canvas->_connect_node);
		canvas->_did_connect = true;
	}
}

bool
MachinaCanvas::node_clicked(NodeView* node, GdkEventButton* event)
{
	if (event->state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK)) {
		return false;

	} else if (event->button == 2) {
		// Middle click: learn
		_app->controller()->learn(_app->maid(), node->node()->id());
		return false;

	} else if (event->button == 1) {
		// Left click: connect/disconnect
		_connect_node = node;
		for_each_selected_node(connect_nodes, this);
		const bool handled = _did_connect;
		_connect_node = NULL;
		_did_connect  = false;
		if (_app->chain_mode()) {
			return false;  // Cause Ganv to select as usual
		} else {
			return handled;  // If we did something, stop event here
		}
	}

	return false;
}

bool
MachinaCanvas::on_event(GdkEvent* event)
{
	if (event->type == GDK_BUTTON_RELEASE
	    && event->button.button == 3
	    && !(event->button.state & (GDK_CONTROL_MASK))) {

		action_create_node(event->button.x, event->button.y);
		return true;
	}
	return false;
}

void
MachinaCanvas::on_new_object(SPtr<client::ClientObject> object)
{
	const machina::URIs& uris = URIs::instance();
	const Atom&          type = object->get(uris.rdf_type);
	if (!type.is_valid()) {
		return;
	}

	if (type.get<URIInt>() == uris.machina_Node) {
		const Atom& node_x = object->get(uris.machina_canvas_x);
		const Atom& node_y = object->get(uris.machina_canvas_y);
		float x, y;
		if (node_x.type() == _app->forge().Float &&
		    node_y.type() == _app->forge().Float) {
			x = node_x.get<float>();
			y = node_y.get<float>();
		} else {
			int scroll_x, scroll_y;
			get_scroll_offsets(scroll_x, scroll_y);
			x = scroll_x + 128.0;
			y = scroll_y + 128.0;
		}

		NodeView* view = new NodeView(_app->window(), *this, object, x, y);

		//if ( ! node->enter_action() && ! node->exit_action() )
		//	view->set_base_color(0x101010FF);

		view->signal_clicked().connect(
			sigc::bind<0>(sigc::mem_fun(this, &MachinaCanvas::node_clicked),
			              view));

		object->set_view(view);

	} else if (type.get<URIInt>() == uris.machina_Edge) {
		SPtr<machina::client::ClientObject> tail = _app->client_model()->find(
			object->get(uris.machina_tail_id).get<int32_t>());
		SPtr<machina::client::ClientObject> head = _app->client_model()->find(
			object->get(uris.machina_head_id).get<int32_t>());

		if (!tail || !head) {
			std::cerr << "Invalid arc "
			          << object->get(uris.machina_tail_id).get<int32_t>()
			          << " => "
			          << object->get(uris.machina_head_id).get<int32_t>()
			          << std::endl;
			return;
		}

		NodeView* tail_view = dynamic_cast<NodeView*>(tail->view());
		NodeView* head_view = dynamic_cast<NodeView*>(head->view());

		object->set_view(new EdgeView(*this, tail_view, head_view, object));

	} else {
		std::cerr << "Unknown object type " << type.get<URIInt>() << std::endl;
	}
}

void
MachinaCanvas::on_erase_object(SPtr<client::ClientObject> object)
{
	const Atom& type = object->get(URIs::instance().rdf_type);
	if (type.get<URIInt>() == URIs::instance().machina_Node) {
		delete object->view();
		object->set_view(NULL);
	} else if (type.get<URIInt>() == URIs::instance().machina_Edge) {
		remove_edge(dynamic_cast<Ganv::Edge*>(object->view()));
		object->set_view(NULL);
	} else {
		std::cerr << "Unknown object type" << std::endl;
	}
}

void
MachinaCanvas::action_create_node(double x, double y)
{
	const Properties props = {
		{ URIs::instance().rdf_type,
		  _app->forge().make_urid(URIs::instance().machina_Node) },
		{ URIs::instance().machina_canvas_x,
		  _app->forge().make((float)x) },
		{ URIs::instance().machina_canvas_y,
		  _app->forge().make((float)y) },
		{ URIs::instance().machina_duration,
		  _app->forge().make((float)_app->default_length()) } };

	_app->controller()->create(props);
}

void
MachinaCanvas::action_connect(NodeView* tail, NodeView* head)
{
	_app->controller()->connect(tail->node()->id(), head->node()->id());
}

void
MachinaCanvas::action_disconnect(NodeView* tail, NodeView* head)
{
	_app->controller()->disconnect(tail->node()->id(), head->node()->id());
}

}  // namespace machina
}  // namespace gui
