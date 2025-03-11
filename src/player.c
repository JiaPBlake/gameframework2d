#include <SDL.h>

#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"

#include "player.h"
#include "collision.h"
#include "world.h"
#include "camera.h"
//#include "battle.h"
#include "window.h" //to set selected
#include "inventory.h"

//extern Entity *otherEnt;  //artifact of old thinking (me trying to implement collision before we went over it in class
Uint8 _INBATTLE = 0;
Uint8 _NEWENCOUNTER = 0;
extern Uint32 _MOUSEBUTTON;
Uint8 health_frame = 5;
int keySelectTimer = 0; //50 frames

//void player_think(Entity* self);
//void player_update(Entity* self);  //I decided to declare them in player.h instead  Haven't test ran this lmao but it should work fine
void player_damage(Entity* self, Entity* other, Entity* creit, float damage, Uint8 damageType);

//if you want to make the player a global entity for other files to access
Entity* thePlayer = NULL;

// "Rule of 3"s in programming: if ever we need a constant Constructor -  we'll need a Deconstructor  as well as a co-? Constructor

typedef struct {
	//float	speedMax; /**<Max speed of the player.So that when Player is in battle, we can change this value to 0 so that they don't move*/
	//int xp, neededxp;   //don't need these
	int		fierce_points;
	int		docile_points;
	int		cunning_points;															//WAIT I LIED I COULD TOTALLY MAKE PICK-UPABLE ITEMS ENTITIES !! THAT WOULD MAKE COLLISSION SO MUCH EASIER LMAOO
			//Something important to think about.  INVENTORY is something only a player will have.  Inventory has its OWN *itemsList.  and the World will have its own as well - for the purpose of drawing them for pickup
	Inventory inventory;  //   this.. this should be a pointer . 'cause our init function takes a pointer. (I mean yeah I could just pass &)  but like I feel like... a pointer here makes things easier..?
	/*int orb;
	int stone;		Don't need these here. each item in inventory->itemsList has its own count. and that's what the inventory_add_item() functions are for
	int trident;
	int drumstick;
	int artifact;*/
}PlayerEntityData;

Entity* player_get_the() {
	return thePlayer;
}

Entity *player_new_entity(GFC_Vector2D position, const char* defFile) //added def file const char *
{
	Entity *self;

	PlayerEntityData *data; //lmfao make sure this is a pointer

	if (thePlayer) {
		gfc_vector2d_copy(thePlayer->position, position);
		slog("The Player already exists");
		return thePlayer;
	}

	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	//now, we have an empty section of memory to work with, to fill in all the data members of our Entity class
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}
	//slog("Initializing Config File");  //Jlog
	gfc_input_init("config/my_input.cfg");  //this is the funciton we use to initalize our inputs  a.k.a our keybinds!!
	//That being said--  there's already a SAMPLE confic within the gfc folder: gameframework2d\gfc\sample_config

	entity_configure_from_file(self, defFile);  //Configure the entity (Loading the sprite and setting spawn position)
	if (position.x >= 0) { gfc_vector2d_copy(self->position, position); } //position override from the parameters
	//if position is a negative vector, don't override, just use the one from the def file

	self->layer = ECL_Entity;
	self->layer |= ECL_ALL; //The Player should collide with all possible things. Other Entities, the World, AND Items
	self->team = ETT_player;
	self->think = player_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = player_update;
	//self->damage = player_damage;

	//Feb 24:
	self->data_free = player_data_free;
	//Feb 24:   Polymorphism --  our Player IS an Entity, but we're adding more to it
//And I don't have to worry about trying to implement this into entity_configure,  because all this upcoming info is SPECIFIC to the player
	/*data = gfc_allocate_array(sizeof(PlayerEntityData), 1);
	if (data) {
		data->fierce_points = 0;
		data->docile_points = 0;
		data->cunning_points = 0;

	
		inventory_init(&data->inventory); //Player Entity Data's inventory

		self->data = data;   //"data" is the type-associated version.
	}*/
	

	//Set my global variable. So others can find me with the player_get_the() function
	thePlayer = self;

	return self;
}

void player_data_free(Entity* self) {
	slog("Made it to the player_data_free function even though it's a void pointer");
	if ((!self) || (!self->data)) return;
	
	PlayerEntityData* data;
	data = (PlayerEntityData *)self->data; //cast the Void pointer specifically to a PlayerEntityData* pointer
	
	//other cleanup goes here
	//gf2d_sprite_Free(data->profilePicture);  //for example.  if I had a sprite for profilePicture
	slog("In the Player data free function. About to free up the inventory");
	//inventory_cleanup(&data->inventory);


//free the data once we're done with it
	free(data);	///BECAUSEE   my Entity, self.  ONLY knows "data"  as a void pointer.  It does not know that it's a PlayerEntityData
	self->data = NULL; //could have just done   free(self->data),  but!  if we have other things to be cleaned up,  like a Sprite !!  making a PlayerEntityData pointer is (the only...?) best way to access it

}


void player_think(Entity* self) {
	
	if (!self) return;  //if I no am, then can not think!
	
	PlayerEntityData* data;


	gfc_input_update();
	/*GFC_Vector2D dir = {0};		Video code just to be sure this works upon compiling.  It does
	int mx = 0, my = 0;
	SDL_GetMouseState(&mx, &my);
	if (self->position.x < mx) dir.x = 1;
	if (self->position.y < my) dir.y = 1;
	if (self->position.x > mx) dir.x = -1;
	if (self->position.y > my) dir.y = -1;

	gfc_vector2d_normalize(&dir);
	gfc_vector2d_scale(self->velocity, dir, 2); */
	
	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		self->velocity.x = 1.0;
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  left
		self->velocity.x = -1.0;
	}
	else { self->velocity.x = 0; }

	if (gfc_input_command_down("down")) {  //if I'm pressin'  down
		self->velocity.y = 1.0;
	}
	else if (gfc_input_command_down("up")) {  //if I'm pressin'  up
		self->velocity.y = -1.0;
	}
	else { self->velocity.y = 0; }	//stop movement if we're not holding down a button
	

	/* This was for diagonal screen bouncing
	if (self->position.x <= 0) self->velocity.x = 1;
	if (self->position.y <= 0) self->velocity.y = 1;
	if (self->position.x > 800) self->velocity.x = -1;
	if (self->position.y > 650) self->velocity.y = -1;*/

	gfc_vector2d_normalize(&self->velocity);  //takes a pointer
	gfc_vector2d_scale(self->velocity, self->velocity, 3);  //Scale the velocity

	//slog("Printing out something about the Other Entity in the Player_new function. to make sure we can see it: Position.x = %f", otherEnt->position.x);
	/*//this is the collision function I tried to make myself, but uhhhh.   Yeah that's not fleshed out much at all lol
	if (collision_check(self, otherEnt)) { 
		slog("collision in PLAYER.C!!!");
		gfc_vector2d_scale(self->velocity, self->velocity, -2);
	}*/
	//Collission test !!   Single collision grabbing one other entity through a global variable
	/*if (entity_collision_check(self, otherEnt)) {
		//slog("Collision using the function defined in entity!"); //WORKS
		_INBATTLE = 1;
	}
	else { _INBATTLE = 0; }*/

	//Collision in class:    (Works !)
	Entity* other;
	GFC_List* others;
	int i, c; //i for iterating.   c for getting the  gfc_list_count(others);  so that we can iterate  i < c

	others = entity_collide_all(self);  //BE SURE TO DELETE THIS LIST ONCE WE'RE DONE WITH IT
	
	if (others) { //if I'm colliding with ANYTHING:
		
		other = gfc_list_get_nth(others, 0); //in my game, the player will really only be colliding with 1 thing at a time
		if (other->team & ETT_monsters) {
			if (!_INBATTLE) {
				if (health_frame == 0) { health_frame = 5; slog("Health reset"); }
				else health_frame--;
					
			}
			_INBATTLE = 1;  //I only want health to decrease ONCE PER collision.  not while I'm colliding
			//self->velocity.x = 0;
			self->think = player_think_battle;
			//Get the monster I'm colliding with
			//other = gfc_list_get_nth(others, 0);
			//battle_start(self, other);
		}
	
		if (other->team & ETT_cave) {
			if (gfc_input_command_down("proceed") && !_NEWENCOUNTER) { //no idea why the fuck gfc_input_command_pressed isn't working .
				if(other->name) slog("Entering cave: %s",other->name);
				_NEWENCOUNTER = 1;  //figure out when to set this to 0. probably in the World funciton ONCE the world is loaded
			}
		}
	}
	else {
		_INBATTLE = 0;
	}
	gfc_list_delete(others);
	//slog("collision list deleted");


	//Feb 24:
	if (self->data) {
		data = (PlayerEntityData*)self->data; 
		
		//Do stuff with data
		//Player tried to use an item !
	}

}

void player_think_battle(Entity* self) {

	//We should ONLY be in this function if the _INBATTLE flag is on.
	if (!_INBATTLE) {
		slog("Not in battle.  Exiting Battle Think function and setting player think back to normal");
		self->think = player_think;
	}

	//Select UI box
	/*
	I need to know what button I'm on.
	 
	
	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		//go right UI
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  left
		//go right UI
	}
	
	if (gfc_input_command_down("enter")) {   //if I press enter,  I want to perform the action of that button... WHICH IS GOING TO VARY PER BUTTON FUCK
		button.action();  //where action is a function pointer ??? BITCH!?!?
	}
	*/
	gfc_input_update();
	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		//self->velocity.x = 1.0;
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			//slog("Going right");
			inc_selected();
			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  left
		//self->velocity.x = -1.0;
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			dec_selected();
			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}
	else { self->velocity.x = 0; }
	gfc_vector2d_normalize(&self->velocity);  //takes a pointer
	gfc_vector2d_scale(self->velocity, self->velocity, 3);  //Scale the velocity

	//GFC_Vector2D move = gfc_vector2d(850, 100);
	//gfc_vector2d_sub(self->position, self->position, move);
	self->position = gfc_vector2d(400, 620);

	//Once collision is done  set _INBATTLE = 0,  but I need to change this for the actual battle function.
	/*
	Entity* other;
	GFC_List* others;
	int i, c; //i for iterating.   c for getting the  gfc_list_count(others);  so that we can iterate  i < c
	others = entity_collide_all(self);  //BE SURE TO DELETE THIS LIST ONCE WE'RE DONE WITH IT
	if (others) { //if I'm colliding with ANYTHING:
		_INBATTLE = 1;
	}
	else {
		_INBATTLE = 0;
		self->think = player_think;
	}
	gfc_list_delete(others);
	//slog("collision list deleted");
	*/


	if (gfc_input_command_down("down")) {  //if I'm pressin'  right
		//self->velocity.x = 1.0;
		reset_selected();
		_INBATTLE = 0;
		self->think = player_think;
	}

	if (gfc_input_command_down("enter")) {  //if I'm pressin'  right
		
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			
			slog("Performing action");
			button_perform_action(get_selected(), window_get_active());

			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}
	
	if (gfc_input_command_down("back")) {
		
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0

			slog("Trying to back out to last window");
			window_go_back();	

			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}

	//I  want toooooooo   call  battle_start()   in my Think
			//and battle_end  in THIS think
	

}

void player_update(Entity* self) {
	if (!self) return;
	//GFC_Vector2D screen; //instead of the screen = gf2d_graphics_get_resolution(); //should be 1200 by 720... I'm gonna make it the camera a.k.a world
	GFC_Rect screen;
	GFC_Rect rect = { 0 };
	GFC_Vector2D ground = { 0 };

	self->frame += 0.1;
	if (self->frame >= self->framesPerLine) self->frame = 0;

	//Update position using the velocity that was determined through Think()ing  'cause what is velocity?? METERS per unit second [frame] :D
	gfc_vector2d_add(self->position, self->position, self->velocity);
	
	screen = camera_get_bounds();
	//WORLD Bounds.  So that when I'm updating,  I don't update to a position where I'm offscreen. instead I will not cross that border
	if (self->position.x + self->bounds.x < 0) self->position.x = 0 - self->bounds.x;
	if (self->position.y + self->bounds.y < 0) self->position.y = 0 - self->bounds.y;
	if (self->position.x - self->bounds.x > screen.w) self->position.x = screen.w + self->bounds.x;
	if (self->position.y - self->bounds.y > screen.h) self->position.y = screen.h + self->bounds.y;


	//Testing World bounds:  WORKS!!
	gfc_rect_copy(rect, self->bounds); //copy our bounds rectangle into our new rect
	gfc_vector2d_add(rect, rect, self->position); //ALL OF MY ENTITIES.  have their position determined with the camera offset included, because that's how I'm. DRAWING them.
	
	if (world_test_shape(world_get_active(), gfc_shape_from_rect(rect))) { //my y position (taking into account my bounds) should be 614 compared to ground's 640 
		//slog("MY y position = %f",self->position.y); 
		ground = world_get_ground();
		//slog("Ground y position = %f", ground.y);
		self->position.y = ground.y + self->bounds.y;
	}

	//entity_update_position();  //he has this single function here.  I don't.. REALLY need to implement this since the dragons aren't doing any moving. 
	camera_center_on(self->position);

	//chat I forget basic math  idk how to do bounds LMAO
	//NVM WE FIGURED IT OUT !!  I could make a CheckBounds() function and plop it in entity..
}


//Using this to try and draw the sprite of any 1 item just to see what purpose I could give that
void player_show_inven(Entity* self) {
	PlayerEntityData* data;
	data = self->data;
	data->inventory;

	slog("Trying to display Resounding Artifact from the player inventory");
	display_item(&data->inventory, "item_resounding_artifact");


}
