#include "client.h"

int main() {
	init_create();
	app_t app;
	app.add_track_unit(0, "f1");
	unload();
	return 0;
}