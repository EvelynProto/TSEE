#include "../tsee.h"

/**
 * @brief Creates a TSEE object
 *
 * @param width Window width
 * @param height Window height
 * @return TSEE*
 */
TSEE *TSEE_Create(int width, int height) {
	TSEE_Log("Initialising TSEE Engine...\n");
	TSEE *tsee = xmalloc(sizeof(*tsee));
	if (!tsee) {
		TSEE_Error("Failed to malloc for TSEE.\n");
		return NULL;
	}
	tsee->resources = NULL;

	// Setup window + renderer
	tsee->window = xmalloc(sizeof(*tsee->window));
	if (!tsee->window) {
		TSEE_Error("Failed to malloc for window.\n");
		xfree(tsee);
		return NULL;
	}
	if (!SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1")) {
		TSEE_Warn(
			"Render batching could not be enabled, performance may suffer.\n");
	}
	tsee->window->width = width;
	tsee->window->height = height;
	tsee->window->running = true;
	tsee->window->fps = 60;
	tsee->window->last_render = 0;

	// Setup world + textures
	tsee->world = xmalloc(sizeof(*tsee->world));
	if (!tsee->world) {
		TSEE_Error("Failed to malloc for world.\n");
		xfree(tsee->window);
		xfree(tsee);
		return NULL;
	}
	tsee->world->objects = TSEE_Array_Create();
	tsee->world->parallax_objects = TSEE_Array_Create();
	tsee->world->scroll_x = 0;
	tsee->world->scroll_y = 0;
	tsee->world->max_scroll_x = 0;
	tsee->world->gravity = (TSEE_Vec2){0, 0};

	// Setup player
	tsee->player = xmalloc(sizeof(*tsee->player));
	if (!tsee->player) {
		TSEE_Error("Failed to malloc for player.\n");
		TSEE_Array_Destroy(tsee->world->objects);
		TSEE_Array_Destroy(tsee->world->parallax_objects);
		xfree(tsee->world);
		xfree(tsee->window);
		xfree(tsee);
		return NULL;
	}
	tsee->player->object = NULL;
	tsee->player->movement.up = false;
	tsee->player->movement.down = false;
	tsee->player->movement.left = false;
	tsee->player->movement.right = false;
	tsee->player->grounded = false;
	tsee->player->held_up = 0;
	tsee->player->jump_force = 1;
	tsee->player->speed = 1;
	tsee->player->step_size = 5;

	// Setup DT calculations
	tsee->dt = 0;
	tsee->last_time = 0;
	tsee->current_time = SDL_GetPerformanceCounter();

	// Setup UI
	tsee->ui = xmalloc(sizeof(*tsee->ui));
	if (!tsee->ui) {
		TSEE_Error("Failed to malloc for UI.\n");
		xfree(tsee->player);
		TSEE_Array_Destroy(tsee->world->objects);
		TSEE_Array_Destroy(tsee->world->parallax_objects);
		xfree(tsee->world);
		xfree(tsee->window);
		xfree(tsee);
		return NULL;
	}

	// Setup init stuff
	tsee->init = xmalloc(sizeof(*tsee->init));
	if (!tsee->init) {
		TSEE_Error("Failed to malloc for tsee->init");
		xfree(tsee->ui);
		xfree(tsee->player);
		TSEE_Array_Destroy(tsee->world->objects);
		TSEE_Array_Destroy(tsee->world->parallax_objects);
		xfree(tsee->world);
		xfree(tsee->window);
		xfree(tsee);
		return NULL;
	}
	tsee->init->text = false;
	tsee->init->ui = false;
	tsee->init->rendering = false;
	tsee->init->events = false;
	tsee->init->input = false;

	// Setup Debugging Counters
	tsee->debug = xmalloc(sizeof(*tsee->debug));
	if (!tsee->debug) {
		TSEE_Error("Failed to malloc for tsee->debug.\n");
		xfree(tsee->init);
		xfree(tsee->ui);
		xfree(tsee->player);
		TSEE_Array_Destroy(tsee->world->objects);
		TSEE_Array_Destroy(tsee->world->parallax_objects);
		xfree(tsee->world);
		xfree(tsee->window);
		xfree(tsee);
		return NULL;
	}
	tsee->debug->event_time = 0;
	tsee->debug->physics_time = 0;
	tsee->debug->render_time = 0;
	tsee->debug->render_times.object_time = 0;
	tsee->debug->render_times.parallax_time = 0;
	tsee->debug->render_times.ui_time = 0;
	tsee->debug->frame_time = 0;
	tsee->debug->framerate = 0;
	tsee->debug->active = false;

	// Load basic settings
	TSEE_Settings_Load(tsee);
	TSEE_Log("TSEE Engine initialized.\n");
	return tsee;
}

/**
 * @brief Initialises all subsystems for a TSEE.
 *
 * @return success status
 */
bool TSEE_InitAll(TSEE *tsee) {
	TSEE_Log("Initialising TSEE modules...\n");
	if (!TSEE_Rendering_Init(tsee)) {
		TSEE_Critical("Failed to initialize the TSEE Rendering Module.\n");
		TSEE_Close(tsee);
		return false;
	}
	TSEE_Log("Initialized TSEE Rendering.\n");
	if (!TSEE_Resource_Init(tsee)) {
		TSEE_Critical("Failed to initialise the TSEE Resource Manager.\n");
		TSEE_Close(tsee);
		return false;
	}
	TSEE_Log("Initialised TSEE Resource Manager.\n");
	if (!TSEE_Text_Init(tsee, true)) {
		TSEE_Critical("Failed to initialize the TSEE Text Module.\n");
		TSEE_Close(tsee);
		return false;
	}
	TSEE_Log("Initialized TSEE Text.\n");
	if (!TSEE_Events_Init(tsee)) {
		TSEE_Critical("Failed to initialize the TSEE Events Module.\n");
		TSEE_Close(tsee);
		return false;
	}
	TSEE_Log("Initialized TSEE Events.\n");
	if (!TSEE_Input_Init(tsee)) {
		TSEE_Critical("Failed to initialize the TSEE Input Module.\n");
		TSEE_Close(tsee);
		return false;
	}
	TSEE_Log("Initialized TSEE Input.\n");
	if (!TSEE_UI_Init(tsee)) {
		TSEE_Critical("Failed to initialize the TSEE UI Module.\n");
		TSEE_Close(tsee);
		return false;
	}
	TSEE_Log("Initialized TSEE UI.\n");
	if (!TSEE_Animation_Init(tsee)) {
		TSEE_Critical("Failed to initialize the TSEE Animation Module.\n");
		TSEE_Close(tsee);
		return false;
	}
	TSEE_Log("All TSEE modules initialized.\n");
	return true;
}

/**
 * @brief Closes a TSEE object, freeing all memory used.
 *
 * @return success status
 */
bool TSEE_Close(TSEE *tsee) {
	if (tsee->window)
		tsee->window->running = false;

	if (tsee->world->objects) {
		for (size_t i = 0; i < tsee->world->objects->size; i++) {
			TSEE_Object *obj = TSEE_Array_Get(tsee->world->objects, i);
			TSEE_Object_Destroy(tsee, obj, false);
		}
		TSEE_Array_Destroy(tsee->world->objects);
	}
	if (tsee->world->parallax_objects) {
		for (size_t i = 0; i < tsee->world->parallax_objects->size; i++) {
			TSEE_Object *para =
				TSEE_Array_Get(tsee->world->parallax_objects, i);
			TSEE_Object_Destroy(tsee, para, false);
		}
		TSEE_Array_Destroy(tsee->world->parallax_objects);
	}
	TSEE_Resource_Unload(tsee);

	if (tsee->player)
		xfree(tsee->player);
	if (tsee->world)
		xfree(tsee->world);

	if (tsee->init->events) {
		xfree(tsee->events->event);
		xfree(tsee->events);
	}

	if (tsee->init->ui) {
		if (tsee->ui->toolbar) {
			if (tsee->ui->toolbar->data) {
				for (size_t i = 0; i < tsee->ui->toolbar->size; i++) {
					TSEE_Toolbar_Object *obj =
						TSEE_Array_Get(tsee->ui->toolbar, i);
					TSEE_Text_Destroy(tsee, obj->text, false);
					for (size_t j = 0; j < obj->buttons->size; j++) {
						TSEE_Toolbar_Child *child =
							TSEE_Array_Get(obj->buttons, j);
						TSEE_Text_Destroy(tsee, child->text, false);
						xfree(child);
					}
					TSEE_Array_Destroy(obj->buttons);
					xfree(obj);
				}
			}
			TSEE_Array_Destroy(tsee->ui->toolbar);
		}
		xfree(tsee->ui);
	}

	TSEE_Window_Destroy(tsee->window);

	if (tsee->init->resources) {
		TSEE_Resource_Close(tsee);
	}

	if (tsee->init->text) {
		TTF_Quit();
	}
	if (tsee->init->rendering) {
		IMG_Quit();
		SDL_Quit();
	}
	xfree(tsee->init);
	xfree(tsee->debug);
	xfree(tsee);
	return true;
}

/**
 * @brief Calculates delta-time for a TSEE object.
 *
 * @return success status
 */
bool TSEE_CalculateDT(TSEE *tsee) {
	tsee->last_time = tsee->current_time;
	tsee->current_time = SDL_GetPerformanceCounter();
	tsee->dt = (double)((tsee->current_time - tsee->last_time) /
						(double)SDL_GetPerformanceFrequency());
	return true;
}

/**
 * @brief Sets the gravity strength for a TSEE object
 *
 * @param gravity New gravity value.
 * @return success status
 */
bool TSEE_World_SetGravity(TSEE *tsee, TSEE_Vec2 gravity) {
	tsee->world->gravity = gravity;
	return true;
}

/**
 * @brief Centers the camera around an object.
 *
 * @param obj Object to center around
 */
void TSEE_World_ScrollToObject(TSEE *tsee, TSEE_Object *obj) {
	SDL_Rect pos = TSEE_Object_GetRect(obj);
	double win_width = tsee->window->width;
	double win_height = tsee->window->height;

	double mid_x = pos.x + pos.w / 2;
	double mid_y = pos.y + pos.h / 2;

	double half_win_width = win_width / 2;
	double half_win_height = win_height / 2;

	if (mid_x < half_win_width && tsee->world->scroll_x > 0) {
		double diff = half_win_width - mid_x;
		tsee->world->scroll_x -= diff;
		obj->texture->rect.x = half_win_width - pos.w / 2;
	} else if (mid_x > half_win_width &&
			   tsee->world->scroll_x < tsee->world->max_scroll_x) {
		double diff = mid_x - half_win_width;
		tsee->world->scroll_x += diff;
		obj->texture->rect.x = half_win_width - pos.w / 2;
	}

	if (mid_y > half_win_height * 0.75f) {
		double diff = mid_y - half_win_height * 0.75f;
		tsee->world->scroll_y -= diff;
		obj->texture->rect.y = half_win_height * 0.75f - pos.h / 2;
	} else if (mid_y < half_win_height * 0.25f && tsee->world->scroll_y < 0) {
		double diff = half_win_height * 0.25f - mid_y;
		tsee->world->scroll_y += diff;
		obj->texture->rect.y = half_win_height * 0.25f - pos.h / 2;
	}

	if (tsee->world->scroll_x < 0) {
		tsee->world->scroll_x = 0;
	} else if (tsee->world->scroll_x > tsee->world->max_scroll_x) {
		tsee->world->scroll_x = tsee->world->max_scroll_x;
	}

	if (tsee->world->scroll_y < 0) {
		tsee->world->scroll_y = 0;
	}

	for (size_t i = 0; i < tsee->world->objects->size; i++) {
		TSEE_Object *object = TSEE_Array_Get(tsee->world->objects, i);
		if (TSEE_Object_CheckAttribute(object, TSEE_ATTRIB_UI) ||
			TSEE_Object_CheckAttribute(object, TSEE_ATTRIB_PLAYER))
			continue;
		if (!TSEE_Object_CheckAttribute(object, TSEE_ATTRIB_PARALLAX)) {
			object->texture->rect.x =
				object->position.x - tsee->world->scroll_x;
		}
		object->texture->rect.y = object->position.y * -1 +
								  tsee->window->height - tsee->world->scroll_y;
	}
}

/**
 * @brief Runs the main game loop.
 */
void TSEE_MainLoop(TSEE *tsee) {
	while (tsee->window->running) {
		TSEE_CalculateDT(tsee);
		TSEE_Events_Handle(tsee);
		TSEE_Player_HandleInput(tsee);
		TSEE_Physics_PerformStep(tsee);
		TSEE_Animation_RunStep(tsee);
		TSEE_RenderAll(tsee);
	}
}