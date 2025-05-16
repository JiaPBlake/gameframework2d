#include <SDL.h>
#include <SDL_ttf.h>
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_audio.h"      //J TO BE ADDED
#include "gfc_config_def.h" //J TO BE ADDED

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"


														//note to self:  This list of includes I copy-pasted straight from game.c .  MOVE THINGS to the HEADER FILE whenever need be. I don't imagine the header file to be too expansive, since nothing should include it
//Start  //J ADDED:
	//Removed entity.h  on account of the fact that all the children of entity already include it
#include "player.h"
#include "monster.h"
#include "object.h"
#include "world.h"			//JMainMenu note (JMM note):  really think through what the World will be responsible for.  I anticipate making it my anchor for all entities  .. mm maybe not windows. Windows appear in every world
#include "camera.h"			
	//Removed spawn.   I haven't double checked where it's included,  but I'm90% sure I only included it in game when I was hard-code testing at the very beginning.  World should handle it by now
//#include "ui.h" //J TO BE ADDED
#include "window.h" //includes ui.h
#include "items.h" //J TESTING (should be good honestly
#include "text.h" //J TESTING.  Works as a basic thing. Gotta optimize and move things around         **ALSO INCLUDED IN WINDOW --> UI



#include "mainmenu.h"


//he has a main_menu.c function

//That sets up all the basic stuff of the world.  - the player, the level,  etc. ALL initialized and loaded here in the main menu, so that the player can start free of charge


void initialize_game_state();


//Sprite* mouse;
//GFC_Color mouseGFC_Color;
//int mx, my;
//float mf = 0;			//Honestly I might want to leave the mouse in Game.c  just to be consistent with keeping the Keys(board) there


extern Uint8 _DRAWBOUNDS;
extern Uint8 _INVENTORY_FLAG;
extern Uint8 _INBATTLE;
extern Uint32 _MOUSEBUTTON;
extern Uint8 _START_SCREEN;
extern Uint8 _PAUSED;
extern Uint8 _ANIMPLAYING;

extern Uint8 turn;

static GFC_Sound* test_sound;

static Entity* thePlayer;
static World* world;  // ....?? Where do I want to handle worlds...  Probably here, but think about it more

//I wanna move as much of the game setup code here  as I need (mouse capture, key capture etc)
 //since this funciton will be called even BEFORE the main game loop starts .
void initialize_game_state() {
	//mouseGFC_Color = gfc_color8(100, 100, 100, 200);
	

	//slog("Initializing Config File");  //Jlog
	//gfc_input_init("config/my_input.cfg");  //this is the funciton we use to initalize our inputs  a.k.a our keybinds!!

	gfc_audio_init(
		100,
		64,
		4,
		1,
		1,
		1);


	//What's really funny is I probably COULD move his Sprite_initializing function into this file.  But I'll leave it for now.. don't wanna break EVERYTHING
	entity_system_init(100);    //J ADDED - initalize our Entity system AFTER the sprite system.  Since it depends on the sprites


	text_init(50);		//Line 104		THIS HAS TO BE MADE  A LOOTTT MORE LMFAO

	ui_system_init(100);
	window_system_init(10);
	window_masterlist_initialize("def/ui_windows.def");
	configure_all_windows();
	
	items_initialize("def/items.def");
	move_masterlist_initialize("def/moveList.def");
	move_system_init(25);		//lmAOOO I hve no idea where I included  move.h  but hey! as long as it works.. I'll trace it later prob
	configure_all_moves();

	//dialogue_init(10);


	//mouse = gf2d_sprite_load_all("images/pointer.png", 32, 32, 16, 0);


	//J START:
	world = world_load("def/levels/testLevel.level"); /*world_test_new();*/
	world_set_active(world);
	thePlayer = player_get_the();

	_START_SCREEN = 1;				//SET THIS TO 1 ONCE YOU'VE DRAWN UP A MAIN MENU LMAOO
	_PAUSED = 0;
	_ANIMPLAYING = 0;  //these are for battle anim's specifically
	slog("GAME STATE INITIALIZED");

}

//In the event I call this OUTSIDE the main game loop:   (VERY unlikely I will use this approach)
void main_menu_concept_function_with_another_loop() {
	int begin = 0;
	const Uint8* keys;
	int mx, my;
	float mf = 0;
	Uint32 mouseButton;
	int mouseClickTimer = 0; //10 frames
	initialize_game_state();   //Initialize everything,  just like we used to do outside the Main Loop of game.c
	
/*	THEN !!  What happens is.  The Main menu would be a loop of it's own, technically ! no?
*   because it will appear on screen, infinitely  until the player clicks "Begin Game"
*  So that means, within this function would be a  while(!begin) loop   that would only set the variable 'begin'  once the Start Game button is interacted with or whatever.
* 
*/

	//I would need to create the  Start Screen (UI_Window structure) here.  So that I know all the buttons are in place


	//So then,  I would need these basic things to be in my loop  if I want to get any user input
	while (!begin) {
		gfc_input_update(); // update SDL's internal event structures   //J NOTE - must be called once a frame. If not called, nothing updates, then none of these conditions will ever set
		keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
		/*update things here*/
		mouseButton = SDL_GetMouseState(&mx, &my);
		mf += 0.1;
		if (mf >= 16.0)mf = 0;
		if (mouseClickTimer <= 0) { //We can click the mouse (leave it 1). And Reset the cooldown
			mouseClickTimer = 10;
		}
		else if (mouseClickTimer > 0) {  //On cooldown.  Do not register the click
			mouseButton = 0;
			mouseClickTimer--;
		}

			// HAVEN'T MADE THE WINDOW YET! LOL  do that.
		//  And then I'd handle mouse collision and click detection  ON top of a the Start Game Window button
			// like a Think() function of sorts



		//and drawing ofc just happens in its own loop:

		gf2d_graphics_clear_screen();// clears drawing buffers                //J NOTE: ALL YOUR DRAW CALLS must be within Clear and Next_Frame 
		// all drawing should happen betweem clear_screen and next_frame


		//So I'd draw the window (so I can SEE what I'm doing LMAOO) and cursor:

			//Draw Start Screen Window here

			//UI elements last
			/*gf2d_sprite_draw(
				mouse,
				gfc_vector2d(mx, my),
				NULL,
				NULL,
				NULL,
				NULL,
				&mouseGFC_Color,
				(int)mf);*/
		
		gf2d_graphics_next_frame();// render current draw frame and skip to the next frame 

	}//end of Main Menu loop
}


void game_think() {
//===========================	THINK
	entity_system_think_all();   //THINK FIRST   //J ADDED

}
void game_update() {

	cache_cleanup(); //to periodically clean up out fonts  //J ADDED (TEXT)


	entity_system_update_all();  //then update shit  //J ADDED
	camera_bounds_check(); //Feb 26: J Added
}

// all DRAWing functions  need to be in that specific section of the game.c main game loop
//I will use this function in the main game loop as a means of drawing everything in my game.  (except the cursor)
void game_draw() {
	GFC_Vector2D camera;
	GFC_Vector2D position = { 0 };
	if (world) {
		world = world_get_active();
		world_draw(world);
		if (_DRAWBOUNDS) world_draw_bounds(world); //J ADDED
	}
	entity_system_draw_all();

	if (world && world->foreground) {
		camera = camera_get_offset();  //AHA   since out offset is a negative value,  this changes where the Top Left corner of the world is DRAWNNNN  SO IF I'm in the middle of the world, the world ITSELF will be shifted in the negative direction-  sir DJ you are so smart and good at your job
		position = gfc_vector2d(0, 0);
		gfc_vector2d_add(position, position, camera);
		gf2d_sprite_draw_image(world->foreground, gfc_vector2d((position.x / 0.7), position.y));
	}

	//I will initialize the game state here    and then (after...?)  Draw the actual start screen
	UI_Window* window = {0};

	if(_START_SCREEN) {
		window = window_search_by_name("Start Screen");
	}
	else {
		window = window_get_active();	
	}

	if (_PAUSED) {
		window = window_search_by_name("Pause Screen");
	}

	window_draw(window);

	//I have to draw  Dialogue_window
		//and then draw dialogue on top of it... with dialogue thinking ??  up there ^^??

	if (_INVENTORY_FLAG) {
		player_show_inven(thePlayer);
		stats_draw("Fierce:", FS_medium, GFC_COLOR_RED, gfc_vector2d(50, 120), get_player_points(ENT_fierce)); //we'll just try it with Fierce first
		stats_draw("Docile:", FS_medium, GFC_COLOR_CYAN, gfc_vector2d(50, 185), get_player_points(ENT_docile)); //we'll just try it with Fierce first
		stats_draw("Cunning:", FS_medium, GFC_COLOR_GREEN, gfc_vector2d(50, 250), get_player_points(ENT_cunning)); //we'll just try it with Fierce first
	}



}
