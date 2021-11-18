#include "include/tsee.h"

TSEE TSEECreate(int width, int height) {
	TSEE tsee;
	tsee.fonts = NULL;

	// Setup window + renderer
	tsee.window = malloc(sizeof(*tsee.window));
	tsee.window->width = width;
	tsee.window->height = height;
	tsee.window->running = true;
	tsee.window->fps = 60;

	// Setup world
	tsee.world = malloc(sizeof(*tsee.world));
	tsee.world->objects = TSEEArrayCreate();
	tsee.world->physics_objects = TSEEArrayCreate();
	tsee.world->text = TSEEArrayCreate();

	// Setup player
	tsee.player = malloc(sizeof(*tsee.player));
	tsee.player->physics_object = NULL;
	tsee.player->movement.up = false;
	tsee.player->movement.down = false;
	tsee.player->movement.left = false;
	tsee.player->movement.right = false;

	// Setup DT calculations
	tsee.dt = 0;
	tsee.last_time = 0;
	tsee.current_time = SDL_GetPerformanceCounter();
	return tsee;
}

bool TSEEClose(TSEE *tsee) {
	tsee->window->running = false;
	TSEEDestroyWindow(tsee->window);
	return true;
}

bool TSEECalculateDT(TSEE *tsee) {
	tsee->last_time = tsee->current_time;
	tsee->current_time = SDL_GetPerformanceCounter();
	tsee->dt = (double) ( (tsee->current_time - tsee->last_time) * 1000 / (double) SDL_GetPerformanceFrequency() );
	tsee->dt = tsee->dt * 0.001; // Convert from ms to s
	return true;
}

bool TSEESetWorldGravity(TSEE *tsee, double gravity) {
	tsee->world->gravity = gravity;
	return true;
}