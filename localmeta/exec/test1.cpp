#include "client.h"

int main() {
	init_create();
	app_t app;
	app.add_track_unit(0, "f1");
	return 0;
}

// init shall set mdt[0] to be the root