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

#ifdef __APPLE__
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <gtk/gtkrc.h>
#include "binary_location.h"
#endif

#include <iostream>

#include <glibmm/exception.h>

#include "Patchage.hpp"

#ifdef __APPLE__
void
set_bundle_environment()
{
	char* binloc_c = binary_location();
	std::string binloc(binloc_c);
	free(binloc_c);

	const std::string bundle_path = binloc.substr(0, binloc.find_last_of('/'));

	const std::string lib_path(bundle_path + "/lib");
	setenv("GTK_PATH", lib_path.c_str(), 1);
	setenv("DYLD_LIBRARY_PATH", lib_path.c_str(), 1);

	chdir(bundle_path.c_str());
	const std::string pangorc_path(bundle_path + "/Resources/pangorc");
	setenv("PANGO_RC_FILE", pangorc_path.c_str(), 1);

	const std::string fonts_conf_path(bundle_path + "/Resources/fonts.conf");
	setenv("FONTCONFIG_FILE", fonts_conf_path.c_str(), 1);

	const char* path_c = getenv("PATH");
	std::string path = "/opt/local/bin";
	if (path_c)
		path += std::string(":") + path_c;
	setenv("PATH", path.c_str(), 1);

	gtk_rc_parse((bundle_path + "/Resources/gtkrc").c_str());
}
#endif

int
main(int argc, char** argv)
{
#ifdef __APPLE__
	set_bundle_environment();
#endif

	try {

	Glib::thread_init();

	Gtk::Main app(argc, argv);

	Patchage patchage(argc, argv);
	app.run(*patchage.window());

	} catch (std::exception& e) {
		std::cerr << "patchage: error: " << e.what() << std::endl;
		return 1;
	} catch (Glib::Exception& e) {
		std::cerr << "patchage: error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

