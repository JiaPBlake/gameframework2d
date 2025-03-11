#include "simple_logger.h"

#include "gfc_config.h"
#include "gf2d_graphics.h"

#include "window.h"
#include "battle.h" //to end battle upon clicking Flee

static Uint8 selected_button_index = 0; //global variable that I can Set() and Get()  - primarily is use through the Player_think_battle function
static UI_Window* activeWindow = NULL;
static UI_Window* previousWindow = NULL;     //I can keep this global for now... because hopefully I can get away with JUST 2 windows.. and not have to worry about the Text-box / Dialogue based windows I will inevitably need..
static UI_Window* nextWindow = NULL;		//'cause ideally  EACH window would have a pointer to a previous window.

//Same design pattern as what's in gf2dSprite.c  and entity.c

typedef struct {
	Uint32		button_max;
	Button*		button_master_list;
}ButtonSystem;  //... unbelievable that I have to do this twice. LOL

typedef struct {  //our singleton entity system/manager.  our big-ass list of entities
	Uint32		window_max;
	UI_Window*	window_list;		//this will be a POINTER  to a list of entities

}UI_WindowSystem;

static ButtonSystem button_system = { 0 };
static UI_WindowSystem window_system = { 0 };   //Our GLOBAL variable, since it's a singleton.  But make it static so it's only local to this file



//Buttons done.
void button_system_close() {
	//slog("in Button system close");
	if (button_system.button_master_list != NULL)
	{ //free all the buttons
		button_system_free_all();
		free(button_system.button_master_list); //button_list itself is dynamically allocated  so we'll use the standard library functino to free it once we're done with it
		slog("Button List Freed");
	}
	button_system.button_master_list = NULL; //reset the button_list pointer
	memset(&button_system, 0, sizeof(ButtonSystem)); //completely unecessary.  This doesn't free nor clean up anything. Just sets the data there to be 0
	slog("Button system closed");
}

//Window done.
void window_system_close() {  
	//slog("in Window system close");
	if (window_system.window_list != NULL)
	{ //free all the windows
		window_system_free_all();
		free(window_system.window_list); //window_list itself is dynamically allocated  so we'll use the standard library functino to free it once we're done with it
		slog("Window List Freed");
	}
	window_system.window_list = NULL; //reset the window_list pointer
	memset(&window_system, 0, sizeof(UI_WindowSystem)); //completely unecessary.  This doesn't free nor clean up anything. Just sets the data there to be 0
	slog("UI Window system closed");
}

//Buttons done.
void button_system_init(Uint32 maxButtons) {
	if (button_system.button_master_list) {
		slog("Cannot initialize more than one Button System/Master List. One is already active");
		return;
	}
	if (!maxButtons) {
		slog("cannot initialize ui button system for zero ui buttons");
		return;
	}
	button_system.button_master_list = gfc_allocate_array(sizeof(Button), maxButtons);	//freed by button_system_close()
	if (!button_system.button_master_list) {
		slog("failed to access button list");
	}
	button_system.button_max = maxButtons;
	atexit(button_system_close);
	slog("button system initialized");
}

//Window done
void window_system_init(Uint32 maxWindows) {
	if (window_system.window_list) {
		slog("Cannot initialize more than one Window System/List. One is already active");
		return;
	}
	if (!maxWindows) {
		slog("cannot initialize ui window system for zero ui windows");
		return;
	}
	window_system.window_list = gfc_allocate_array(sizeof(UI_Window), maxWindows);
	if (!window_system.window_list) {
		slog("failed to access window list");
	}
	window_system.window_max = maxWindows;
	atexit(window_system_close);
	slog("Window system initialized");
}

//Gives you a pointer to a free space in our pre-allocated masterlist of Buttons
Button* button_new() {
	int i;		//Find a space in the masterlist to place a Button
	for (i = 0; i < button_system.button_max; i++) {
		if (button_system.button_master_list[i]._inuse)  //If in use
			continue;
		memset(&button_system.button_master_list[i], 0, sizeof(Button));   //Set the memory allocated at this spot to 0 to clean up garbage data

		button_system.button_master_list[i]._inuse = 1;  //set the inuse flag [of this palce in the list] to 1
		return &button_system.button_master_list[i]; //return the address of the thing we just allocated
	}
	slog("failed to allocate new Button: list full");
	return NULL; //return NULL outside the for loop
}

//Window done
UI_Window* window_new() {
	int i;
	for (i = 0; i < window_system.window_max; i++) {
		if (window_system.window_list[i]._inuse)  //If in use
			continue;
		memset(&window_system.window_list[i], 0, sizeof(UI_Window));   //Set the memory allocated at this spot to 0 to clean up garbage data
		
		window_system.window_list[i]._inuse = 1;  //set the inuse flag to 1
		return &window_system.window_list[i]; //return the address of the thing we just allocated
	}
	slog("failed to allocate new Window: list full");
	return NULL; //return NULL outside the for loop
}

//both should be done
void button_system_free_all() { //For the Button Resource manager.  In theory every button should be able to be taken care of by the window it belongs to, but.. just in case
	int i;
	for (i = 0; i < button_system.button_max; i++) {
		if (button_system.button_master_list[i]._inuse) {
			slog("Freeing a button using the button manager");
			button_free(&button_system.button_master_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}
void window_system_free_all() {  //JUST iterates and calls  window_free()
	int i;
	for (i = 0; i < window_system.window_max; i++) {
		if (window_system.window_list[i]._inuse) {
			window_free(&window_system.window_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}


void button_free(Button* self) {
	if (!self) return;

	//So far, Buttons only have Sprites that have been dynamically allocated
	//free the Button's sprite
	if (self->sprite) {
		//slog("Freeing button AT X POS: %f's Sprite", self->position.x);		//Jlog
		gf2d_sprite_free(self->sprite); //frees the spot IN the masterlist of Sprites.   gf2d_sprite.h takes care of actually deleting things.
	}
	slog("Button sprite freed");
	//free(self);				DO NOT !!!!!!   FREE()  ANYTHING THAT HAS NOT BEEN DYNAMICALLY  gfc_allocate_array'EDDDD!!
	
	self->_inuse = 0; //Set its inuse flag to 0
	memset(self, 0, sizeof(Button));
	//slog("Button freed successfully");
}

//Done.  Sprite, Buttons, Layer
void window_free(UI_Window* self) {
	if (!self) return;
	
	//free the Window's sprite
	if (self->sprite) {
		gf2d_sprite_free(self->sprite); //frees the spot IN the masterlist of Sprites.   gf2d_sprite.h takes care of actually deleting things.
	}
	slog("Window's Sprite freed");
	//Free the layer made from the Sprite and all the buttons
	if (self->layer) gf2d_sprite_free(self->layer);
	slog("Window's Layer freed");

	//somehow.  the Button list paired with this Window is fucked..  I think. honestly.  NO.  NO!! THAT DOESN'T MAKE FUCKING SENSEE


//BUTTON FREE-ing
	int i, c;
	Button* button;
	if (self->button_list) {
		slog("Freeing Window '%s's Button List",self->name);
		c = gfc_list_count(self->button_list);
		//slog("The number of buttons in the UI Window about to be freed is: %i",c);	//Jlog  

		//Button* but2 = gfc_list_get_nth(self->button_list, 2);		//for testing purposes
		//slog("Button 2's position: %f", but2->position.x);  //Jlog

		for (i = 0; i < c; i++) {
			button = NULL;
			//but2 = gfc_list_get_nth(self->button_list, 1);			//for testing purposes
			//slog("Button %i's position: %f", 1, but2->position.x);
			button = gfc_list_get_nth(self->button_list, i);
			if ( (!button) || (!button->_inuse)) continue;
			//slog("Printing something about Button #%i BEFORE Button_Free to be sure I'm not FUCKING tripping: %f", i, button->position.x);
			button_free(button);

			//somehow freeing the button fucks up the list

		}
		//Once all the individual buttons have been freed,  I can free the GFC_List I used to hold them
		gfc_list_delete(self->button_list); //delete the GFC_List itself
	}

	//free(self); //technically.. this shouldn't be here  because the Window pointer was never gfc_allocate_array'ed
	slog("Window successfully freed");
	memset(self, 0, sizeof(UI_Window));
	self->_inuse = 0; //Set its inuse flag to 0
}



//--------------------------------------------------------------------------------


int button_get_type(const char* action ) {
	if (!action) { slog("No button action String provided"); return 0; }
	if (gfc_strlcmp(action, "newWindow") == 0 ) {
		//slog("Button should open a new window");
		return BT_NewWindow;
	}
	if (gfc_strlcmp(action, "exitWindow") == 0) {
		//slog("Button should close the window");
		return BT_CloseWindow;
	}
	slog("Neither IF statement occurred,  returning ButtonType 0");
	return 0;
}

//following the same format as a player_new_enty function.  Call new() to get a space in the list.  then call configure on that space separately
Button* button_create(SJson* json) {
	Button* button = {0};

	button = button_new();  //create the new button [spot in our master Button List]. Where-in its _inuse flag will be set.
	//now, we have an empty section of memory to work with, to fill in all the data members of our Button structure
	if (!button) {
		slog("failed to create a new button");
		return NULL;
	}
	
	//slog("Configuring");
	button_configure(button, json);  //Configure the entity (Loading the sprite and setting spawn position)
	
	return button;
}

void button_configure(Button *self, SJson* json) {
	
	if ( (!self) || (!json) ) { slog("No JSON object (OR BUTTON) provided to create the button"); return;}

	char* button_action_json = sj_object_get_string(json, "action");
	
	self->actionType = button_get_type(button_action_json);

	if (self->actionType == BT_NewWindow) {
		self->action = window_transition;
	}
	if (self->actionType == BT_CloseWindow) {
		//slog("This button would Close a window!");
		self->action = battle_end;
	}

	//Button Sprite section
	const char* button_sprite = NULL;
	button_sprite = sj_object_get_string(json, "sprite");
	if (button_sprite) { //if the Window itself has a sprite
		//slog("This button's Sprite is '%s'", button_sprite);
		GFC_Vector2D b_sp_sz = { 0 };
		sj_object_get_vector2d(json, "sprite_size", &b_sp_sz);  //function from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %i", (Uint32)b_sp_sz.x);

		//load the sprite into the object's sprite data member
		self->sprite = gf2d_sprite_load_all(
			button_sprite,
			(Uint32)b_sp_sz.x,
			(Uint32)b_sp_sz.y,
			2,
			1);
		if (!self->sprite->surface) { slog("Button does not have a sprite SURFACE"); }
		//slog("Button Sprite's Frame Height is: %i", self->sprite->frame_h);
		self->sprite_size = b_sp_sz;
		self->center = gfc_vector2d(b_sp_sz.x * 0.5, b_sp_sz.y * 0.5);
		self->framesPerLine = 2;	//yeah 'cause if I want the buttons to be highlighted upon Selection-  wait . FUCK
		self->frame = 0;

		GFC_Rect bounds = gfc_rect(0,0, (Uint32)b_sp_sz.x, (Uint32)b_sp_sz.y);
		self->bounds = bounds;

		GFC_Vector2D button_pos = { 0 };
		sj_object_get_vector2d(json, "button_position", &button_pos);
		self->position = button_pos;
		//slog("This button's position is: %f, %f", self->position.x, self->position.y);
	}
	else { slog("Button Sprite couldn't be found"); }

	const char* button_new_window = NULL;
	
	//Save the next window it's meant to create (IF it needs to)
	if (self->actionType & BT_NewWindow) {
		if (self->actionType & BT_CloseWindow) slog("Just testing the other actionType to make sure it DOESN'T proc");

		button_new_window = sj_object_get_string(json, "nextWindow");
		gfc_line_cpy(self->windowToOpen, button_new_window);
		slog("Button's next window to open should be: %s", self->windowToOpen);
	}

}

void window_layer_build(UI_Window* window) {
	if (!window) return;

	if (window->layer) gf2d_sprite_free(window->layer); //in case it already had one, clean it up
	int i, c;
	GFC_Vector2D position = { 0 };
	Button* button;

	//We're going to make a Surface for our Sprite. So we can convert it to a Texture to use (such that we can draw it all at once)
		// "We're going to be using gf2d_sprite-draw_to_surface()  to draw TO the layer (the SDL_Surface, I think)  all of our tile-stuff (all our Tile sprite related stuff)

	window->layer = gf2d_sprite_new();	//Create a Sprite
	if (!window->layer) slog("No window layer!? :(((");

	//slog("The x dimension for the create surface command is: %i", (int)window->tileMapSize.x * window->tileSet->frame_w);
		//Create the Surface OF the Sprite
	
	
	window->layer->surface = gf2d_graphics_create_surface(1200, 720); //just the size of the screen for the UI

	if (!window->layer->surface) {
		slog("Failed to create layer surface");
		return;
	}
	window->layer->frame_w = 1200; //set the width and height of my Layer, which would be the whole Map size
	window->layer->frame_h = 720;
	//slog("In WINDOW LAYER BUILD");

	//GFC_Vector2D pos = gfc_vector2d(0, 0);	
	gf2d_sprite_draw_to_surface(
		window->sprite,	
		window->position,
		NULL,
		NULL,
		0,
		window->layer->surface);
	
	//now... there's honestly a good chance that I want to take out the buttons from this layer,  considering I'd like them to update frame in real time :melting:

	if (window->button_list) {
		c = gfc_list_count(window->button_list);
		//slog("The number of butotns in the UI Window about to be freed is: %i",c);	//Jlog
		for (i = 0; i < c; i++) {
			button = gfc_list_nth(window->button_list, i);
			//if(ent->name) slog("ent grabbed: %s",ent->name); //Jlog
			if (!button) continue;
			slog("Trying to draw button index %i to the surface",i);

			gf2d_sprite_draw_to_surface(
				button->sprite,
				button->position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
				NULL,			//scale
				NULL,		//center which is a 2D vector
				0,
				window->layer->surface);
			slog("X position of the #%i button is: %f", i, button->position.x);
		}
	}
	window->layer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), window->layer->surface);
	if (!window->layer->texture) {
		slog("Failed to convert window layer to a texture");
		return;
	}
	slog("Layer created");
}

void window_configure(UI_Window* self, SJson* json) {
	if (!json) {
		slog("No JSON object provided to configure Window. Exiting");
		return;
	}

	//Let's get started .
	const char* win_name = { 0 };
	win_name = sj_object_get_string(json, "name");
	if (win_name) {
		gfc_line_cpy(self->name, win_name);
//slog("Name copied: %s",ent_name);
	}
	else { slog("Window did not have a name"); }

	const char* win_sprite = NULL;
	win_sprite = sj_object_get_string(json, "window_sprite");
	if (win_sprite) { //if the Window itself has a sprite
		GFC_Vector2D sp_sz = {0};
		sj_object_get_vector2d(json, "window_sprite_size", &sp_sz);  //function from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %i", (Uint32)sp_sz.x);
		
		//load the sprite into the object's sprite data member
		self->sprite = gf2d_sprite_load_all(
			win_sprite,
			(Uint32)sp_sz.x,
			(Uint32)sp_sz.y,
			1,
			1);
		self->sprite_size = sp_sz;
		self->center = gfc_vector2d(sp_sz.x*0.5, sp_sz.y*0.5);
		self->framesPerLine = 1;
		self->frame = 0; //Since frame 0 will be the default for every entity,  just et the first frame here in the configure function

		GFC_Vector4D bounds;
		sj_object_get_vector4d(json, "window_bounds", &bounds);
		self->bounds = gfc_rect_from_vector4(bounds);

		GFC_Vector2D win_pos = { 0 };
		sj_object_get_vector2d(json, "window_position", &win_pos);
		self->position = win_pos;

	}

	else { slog("Window did not have a sprite"); }
	//Still need to do the Text with  "window_Text"



	//-----------------------------------------------
	//Buttons Section
	SJson* buttonList, * button;
	const char* ent_name = NULL;
	int i, c, buttonCount;
	Button* butto;


	self->button_list = gfc_list_new();  //allocate the list here
	buttonList = sj_object_get_value(json, "buttons"); //This is now a list/array of N objects
	buttonCount = sj_array_get_count(buttonList);
	slog("The number of buttons in this Window is: %i", buttonCount);	//Jlog

	for (i = 0; i < buttonCount; i++) {	

		button = sj_array_get_nth(buttonList, i); //buttons is now the JSon object that contains all the info about a given button
		//Configure the button
		if (button) {
			butto = button_create(button); //Configure for buttons is like Spawn for entities;  creates a new one an configures it using hte JSon object
			if (butto) {
				//slog("Configured Button #%i and adding it to the list", i); //Jlog
				gfc_list_append(self->button_list, butto); //append the button to our Window's list.  THIS will be the primary point of contact for them
			}
			else { slog("Button configuration failed. On iteration: %i", i); }
		}

		else { slog("Button on index %i could not be retrieved from 'buttons' list", i); }
	}

	c = gfc_list_count(self->button_list);
	slog("All buttons should be configured.  Length of the Window's Button List is: %i", c);

	//Build the layer so that I can draw the whole Window  (window's sprite AND alll button sprites)  in one fell swoop:
	//window_layer_build(self);

	activeWindow = self;
	self->prev = previousWindow;
	//end of Configure function
}

void window_configure_from_file(UI_Window* self, const char* filename) {
	SJson* json;
	if (!self) return;
	json = sj_load(filename);
	if (!json) return;
	window_configure(self, json);
	//close it
	sj_free(json);
}

UI_Window* window_get_prev() {
	return previousWindow;
}
UI_Window* window_get_next() {
	return nextWindow;
}

UI_Window* window_get_active() {
	return activeWindow;
}
void window_set_active(UI_Window* windowToSet) {
	activeWindow = windowToSet;
}

void set_selected(int index) {
	selected_button_index = index;
}
void inc_selected() {
	if (selected_button_index < 3) {
		selected_button_index++;
	}
}
void dec_selected() {
	if (selected_button_index > 0) {
		selected_button_index--;
	}
}
Uint8 get_selected() {
	return selected_button_index;
}

void reset_selected() {
	selected_button_index = 0;
}

void window_draw(UI_Window* window) {
	GFC_Vector2D pos = gfc_vector2d(0, 0);
	//gf2d_sprite_draw_image(window->layer, pos);
	
	//DRAWING EVERYTHING MANUALLY:   WHICH WORKS !!!    assuming you configured the sprites to Keep Surface 0

	
	gf2d_sprite_draw(window->sprite,
		pos,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
		NULL,			//scale
		NULL,		//center which is a 2D vector
		NULL,	//rotation
		NULL,		//flip
		NULL,		//colorShift
		0);

	//Draw buttons:
	int i, c;
	Button* button;
	int frame;
	if (window->button_list) {
		c = gfc_list_count(window->button_list);
		for (i = 0; i < c; i++) {
			button = gfc_list_nth(window->button_list, i);
			//if(ent->name) slog("ent grabbed: %s",ent->name); //Jlog
			if (!button) continue;
			frame = 0;

			//when drawing the buttons, I want to know WHICH one is selected. So that I can make THIS specific iteration's frame = 1
			if (i == selected_button_index) { frame = 1; }  //and first frame selected SHOULD be 0 so !  this works actually

			gf2d_sprite_draw(button->sprite,
				button->position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
				NULL,			//scale
				NULL,		//center which is a 2D vector
				NULL,	//rotation
				NULL,		//flip
				NULL,		//colorShift
				frame);
		}
	}
	
}

//
void window_transition(Button* self) {
	if (!nextWindow) {
		slog("in Window transition function");
		previousWindow = activeWindow;
		UI_Window* win = window_new();
		window_configure_from_file(win, self->windowToOpen);

		activeWindow = win;
		slog("New Window loaded. Name of previous: %s", previousWindow->name);
	}
	else {
		slog("Next window has already been loaded. No need to configure it again.");
		previousWindow = activeWindow; //To go back to Attack screen from Main.  Save Main into prev.  SHOULD already be there.. but. y'know just incase
		activeWindow = nextWindow;
	}
}
/*					//get_active()
void world_transition(World* old, const char* newWorld, GFC_Vector2D targetPlayerSpawn) {  //here we go
	Entity* player;
	World* world = NULL;
	player = player_get_the();
	if (!newWorld) {
		slog("No newWorld name provided");
		return;
	}

	if (old) {
		//slog("Checking if player");
		if (player) {
			//slog("Removing player");
			world_remove_entity(old->entity_list, player);  //remove the Player from the Old world's Ent List so that the player is not freed
		}
		//slog("Freeing world now");
		world_free(old);
	}

	slog("Loading new world");
	world = world_load(newWorld);  //AND  so that when we laod up the new world, it will just return the current active Player and add it to ITS list
	if (!world) {   //uh oh
		slog("Uh oh. newWorld failed to load");
		return;
	}
	else { slog("New world LOADED: %s", world->name); }
	
	activeWorld = world;
	//and move the player's position to the target spawn point for the new world
	gfc_vector2d_copy(player->position, targetPlayerSpawn);
	//return world;
	
}
*/

void window_go_back() {
	if (!previousWindow) { slog("No previous window. Cannot back out"); return; }
	UI_Window* temp;
	temp = activeWindow;
	slog("The CURRENT Window's name is: %s",temp->name);
	activeWindow = previousWindow;
	previousWindow = NULL; //so that I can't try and go back more than once.  Otherwise, that messes things up
	slog("Window sawpped. Now, Window's name is: %s", activeWindow->name);
	nextWindow = temp;
}

void button_perform_action(int selected, UI_Window* win) {
	//this is gonna have to load windows in the same way Worlds do
	if (!win) { slog("Window not provided. Not performing any button actions"); return; }
	int i, c;
	GFC_Vector2D position = { 0 };
	Button* button;
	slog("Selected's current value is: %i",selected);

	if (win->button_list) {
		button = gfc_list_nth(win->button_list, selected);
		if (!button) return;
		if (!button->action) { slog("Selected button does not have an action function assigned"); return; }

		slog("Calling Selected Button's action function");
		//Everything up to this point works perfectly fine
		
		button->action(button);
		//... fuck idk how to do this with a function pointer...  truth be told to you
		
	}
}

