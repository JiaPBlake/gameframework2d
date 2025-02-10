#include <SDL.h>

#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "player.h"

//void player_think(Entity* self);
//void player_update(Entity* self);  //I decided to declare them in player.h instead  Haven't test ran this lmao but it should work fine

Entity *player_new_entity(GFC_Vector2D position)
{
	Entity *self;
	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}
	//Bounding box here
	//self->bounds = (25,25); //specific dimensions of your sprite's pixels.
	self->think = player_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = player_update;
	self->velocity = gfc_vector2d(1, 1);
	gfc_vector2d_normalize(&self->velocity);
	gfc_vector2d_scale(self->velocity, self->velocity, 0.5);
	gfc_vector2d_copy(self->position, position); 

	//entity_configure_from_file(self, "defs/player.def");  //INSTEAD OF this Sprite loading bock underneath
	//nor  do we have to set  self->bounds  to anything
	self->sprite = gf2d_sprite_load_all(
		"images/ed210.png",
		128,
		128,
		16,
		0
	); //all the sprite info here
	self->frame = 0;

	return self;
}

void player_think(Entity* self) {
	
	if (!self) return;  //if I no am, then can not think!
	
	/*GFC_Vector2D dir = {0};		Video code just to be sure this works upon compiling.  It does
	int mx = 0, my = 0;
	SDL_GetMouseState(&mx, &my);
	if (self->position.x < mx) dir.x = 1;
	if (self->position.y < my) dir.y = 1;
	if (self->position.x > mx) dir.x = -1;
	if (self->position.y > my) dir.y = -1;

	gfc_vector2d_normalize(&dir);
	gfc_vector2d_scale(self->velocity, dir, 2); */
	

	//Swear to you there's something wrong with the gfc_input_update() function in gfc_input.c
	/*
	//GFC_Vector2D screen;		
	//The problem is not with these functions.  We PROPERLY get to the gfc_input_command_down() function. AND the Input object has our list of keys...
	//it just.. WON'T read the keys
	//actually to be so deadass with you it might have smth to do with the way it's not searching through the Input *in object's ->keyCodes list... also dk what this is: gfc_input_data.input_list
	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		self->velocity.x = 1.0;
		slog("Right key pressed");
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  left
		self->velocity.x = -1.0;
	}
	else { self->velocity.x = 0; }							//something  seomething

	if ( gfc_input_command_down("down") ) {  //if I'm pressin'  down
		self->velocity.y = 1.0;
	}
	else if (gfc_input_command_down("up")) {  //if I'm pressin'  up
		self->velocity.y = -1.0;
	}
	else { self->velocity.y = 0; }	//stop movement if we're not holding down a button
	
	//^Doesn't work.  Make me bounce around diagonally instead :) */

	if (self->position.x <= 0) self->velocity.x = 1;
	if (self->position.y <= 0) self->velocity.y = 1;
	if (self->position.x > 800) self->velocity.x = -1;
	if (self->position.y > 650) self->velocity.y = -1;

	gfc_vector2d_normalize(&self->velocity);  //takes a pointer
	//gfc_vector2d_scale(self->velocity, self->velocity, 5);  //him scaling it to 5

	//screen = gf2d_graphics_get_resolution();
	gfc_vector2d_add(self->position, self->position, self->velocity);

}
void player_update(Entity* self) {
	if (!self) return;

	self->frame += 0.1;
	if (self->frame >= 16) self->frame = 0;

	//Update position using the velocity that was determined through Think()ing  'cause what is velocity?? METERS per unit second [frame] :D
	gfc_vector2d_add(self->position, self->position, self->velocity);
	//Bounds.  So that when I'm updating,  I don't update to a position where I'm offscreen. instead I will not cross that border
	if (self->position.x < 0) self->position.x = 0;
	if (self->position.y < 0) self->position.y = 0;
	if (self->position.x > 12000) self->position.x = 12000;
	if (self->position.y > 700) self->position.y = 700;

}
