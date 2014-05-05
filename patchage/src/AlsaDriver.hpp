/* This file is part of Patchage.
 * Copyright 2007-2014 David Robillard <http://drobilla.net>
 *
 * Patchage is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Patchage is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Patchage.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PATCHAGE_ALSADRIVER_HPP
#define PATCHAGE_ALSADRIVER_HPP

#include <queue>
#include <set>
#include <string>
#include <map>

#include <alsa/asoundlib.h>
#include <pthread.h>

#include "Driver.hpp"
#include "PatchageModule.hpp"

class Patchage;
class PatchagePort;

/** Handles all externally driven functionality, registering ports etc.
 */
class AlsaDriver : public Driver
{
public:
	explicit AlsaDriver(Patchage* app);
	~AlsaDriver();

	void attach(bool launch_daemon = false);
	void detach();

	bool is_attached() const { return (_seq != NULL); }

	void refresh();
	void destroy_all();

	PatchagePort* create_port_view(
			Patchage*     patchage,
			const PortID& id);

	bool connect(PatchagePort* src_port,
	             PatchagePort* dst_port);

	bool disconnect(PatchagePort* src_port,
	                PatchagePort* dst_port);

	void print_addr(snd_seq_addr_t addr);

	void process_events(Patchage* app);

private:
	bool         create_refresh_port();
	static void* refresh_main(void* me);
	void         _refresh_main();

	PatchageModule* find_module(uint8_t client_id, ModuleType type);

	PatchageModule*
	find_or_create_module(
			Patchage*          patchage,
			uint8_t            client_id,
			const std::string& client_name,
			ModuleType         type);

	void
	create_port_view_internal(
			Patchage*        patchage,
			snd_seq_addr_t   addr,
			PatchageModule*& parent,
			PatchagePort*&   port);

	PatchagePort* create_port(
		PatchageModule&    parent,
		const std::string& name,
		bool               is_input,
		snd_seq_addr_t     addr);

	Patchage*  _app;
	snd_seq_t* _seq;
	pthread_t  _refresh_thread;

	Glib::Mutex               _events_mutex;
	std::queue<PatchageEvent> _events;

	struct SeqAddrComparator {
		bool operator() (const snd_seq_addr_t& a, const snd_seq_addr_t& b) const {
			return ((a.client < b.client) || ((a.client == b.client) && a.port < b.port));
		}
	};

	typedef std::set<snd_seq_addr_t, SeqAddrComparator> Ignored;
	Ignored _ignored;

	typedef std::multimap<uint8_t, PatchageModule*> Modules;
	Modules _modules;

	typedef std::map<PatchagePort*, PortID> PortAddrs;
	PortAddrs _port_addrs;

	bool ignore(const snd_seq_addr_t& addr, bool add=true);
};

#endif // PATCHAGE_ALSADRIVER_HPP
