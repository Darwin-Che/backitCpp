#include "client.h"

int main() {
	char depot_dir[fname_t::MAXLEN];
	find_depot(depot_dir);
	app_t app(depot_dir);
	app.add_track_file(0, "file 1");
	midx<mdt_t> dir = app.add_track_dir(0, "dir 1");
	app.add_track_file(dir, "file 2");
	print_mdt(0);
	return 0;
}