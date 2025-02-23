#include <SDL.h>

#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"

#include "player.h"
#include "collision.h"

extern Entity *otherEnt;

//void player_think(Entity* self);
//void player_update(Entity* self);  //I decided to declare them in player.h instead  Haven't test ran this lmao but it should work fine
void player_damage(Entity* self, Entity* other, Entity* creit, float damage, Uint8 damageType);

Entity *player_new_entity(GFC_Vector2D position) //added def file const char *
{
	Entity *self;
	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	//now, we have an empty section of memory to work with, to fill in all the data members of our Entity class
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}
	gfc_input_init("config/my_input.cfg");  //he added this  AHHH this is the funciton we use to initalize our inputs  a.k.a our keybinds!!
	//That being said--  there's already a SAMPLE confic within the gfc folder: gameframework2d\gfc\sample_config

	self->think = player_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = player_update;

	entity_configure_from_file(self, "def/player.def");  //Configure the entity (Loading the sprite and setting spawn position)

	//self->velocity = gfc_vector2d(1, 1);
	//gfc_vector2d_normalize(&self->velocity);  //Commenting this out because I decided to initialize it in the Configure function
	//gfc_vector2d_scale(self->velocity, self->velocity, 0.5);
	if(position.x >= 0 ) { gfc_vector2d_copy(self->position, position); } //position override from the parameters
	//if position is a negative vector, don't override, just use the one from the def file


	return self;
}

void player_think(Entity* self) {
	
	if (!self) return;  //if I no am, then can not think!
	
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

	if ( gfc_input_command_down("down") ) {  //if I'm pressin'  down
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
	//gfc_vector2d_scale(self->velocity, self->velocity, 5);  //him scaling it to 5

	//slog("Printing out something about the Other Entity in the Player_new function. to make sure we can see it: Position.x = %f", otherEnt->position.x);

	/*if (collision_check(self, otherEnt)) { //this is the collision function I tried to make myself, but uhhhh
		slog("collision in PLAYER.C!!!");
		gfc_vector2d_scale(self->velocity, self->velocity, -2);
	}*/

	//Collission test !!
	if (entity_collision_check(self, otherEnt)) {
		slog("Collision using the function defined in entity!");
	}

	//Collision in class :
	Entity* other;
	GFC_List* others;
	int i, c; //i for iterating.   c for getting the  gfc_list_count(others);  so that we can iterate  i < c

	other = entity_collide_all(self);
	/*if (others) {
		//iterate through the list, others

		//handle entity collision
	}*/


	//screen = gf2d_graphics_get_resolution();
	gfc_vector2d_add(self->position, self->position, self->velocity);

}

void player_update(Entity* self) {
	if (!self) return;

	self->frame += 0.1;
	if (self->frame >= self->framesPerLine) self->frame = 0;

	//Update position using the velocity that was determined through Think()ing  'cause what is velocity?? METERS per unit second [frame] :D
	gfc_vector2d_add(self->position, self->position, self->velocity);
	//Bounds.  So that when I'm updating,  I don't update to a position where I'm offscreen. instead I will not cross that border
	if (self->position.x + self->bounds.x < 0) self->position.x = 0 - self->bounds.x;
	if (self->position.y + self->bounds.y < 0) self->position.y = 0 - self->bounds.y;
	if (self->position.x - self->bounds.x > 1200) self->position.x = 1200 + self->bounds.x;
	if (self->position.y - self->bounds.y > 700) self->position.y = 700 + self->bounds.y;

	//chat I forget basic math  idk how to do bounds LMAO
	//NVM WE FIGURED IT OUT !!  I could make a CheckBounds() function and plop it in entity..
}
