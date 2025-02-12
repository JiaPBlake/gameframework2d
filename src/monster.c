#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "monster.h"

//Everywhere you see Player,  replace with Monster

Entity* monster_new_entity(GFC_Vector2D position)
{
	Entity* self;
	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	if (!self) {
		slog("failed to spawn a new monster entity");
		return NULL;
	}
	//Bounding box here
	//self->bounds = (25,25); //specific dimensions of your sprite's pixels.
	self->think = monster_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = monster_update;
	self->velocity = gfc_vector2d(1, 0);
	gfc_vector2d_copy(self->position, position);

	//entity_configure_from_file(self, "defs/player.def");  //INSTEAD OF this Sprite loading bock underneath
	//nor  do we have to set  self->bounds  to anything
	self->sprite = gf2d_sprite_load_all(
		"images/enemy.png",  //change this picture to smth silly
		300,
		300,
		1,
		0
	); //all the sprite info here
	self->frame = 0;

	return self;
}


void monster_think(Entity *self) {
	//GFC_Vector2D screen;
	if (!self) return;			//if I no am, then can not think!

	if (self->position.x <= 0) self->velocity.x = 1;
	if (self->position.x >= 850) self->velocity.x = -1;
	//fuck..  just get it to spawn on screen first and _then_ you can worry about forgoing this dir vector and using its velocity
	
	gfc_vector2d_normalize(&self->velocity);  //I'll just normalize this dir vector first,  since I'm using it anyway
	gfc_vector2d_scale(self->velocity, self->velocity, 2);
	/*
	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		self->velocity.x = 1.0;
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  right
		self->velocity.x = -1.0;
	}
	else self->velocity.x = 0;							//something  seomething

	if (gfc_input_command_down("down")) {  //if I'm pressin'  right
		self->velocity.y = 1.0;
	}
	else if (gfc_input_command_down("up")) {  //if I'm pressin'  right
		self->velocity.y = -1.0;
	}
	else self->velocity.y = 0;		//stop movement if we're not holding down a button
	
	*/

	//screen = gf2d_graphics_get_resolution();

}

void monster_update(Entity* self) {
	gfc_vector2d_add(self->position, self->position, self->velocity);
	if (self->position.x < 0) self->position.x = 0;
	if (self->position.y < 0) self->position.y = 0;
}

/*end of file*/