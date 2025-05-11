#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "npc.h"
#include "world.h"


typedef enum {
	aba,
	abb,
	abc
}idkIfIllNeedOneOfTheselmao; //dk if they should be bit flags tho

typedef struct {
	GFC_List		*pathToPlayer; //list of world coordinates to the player  :0 woah
	//MonsterType		monsterType;



}NPCEntityData;


Entity* npc_new_entity(GFC_Vector2D position, const char* defFile) //Now that I'm configuring the spawn.h file, I'm going to iterate through my list of Spawnable entities
{															// and call this function providing the position to spawn it AND the DefFile to be used in Configuring this entity
	Entity* self;
	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	if (!self) {
		slog("failed to spawn a new NPC entity");
		return NULL;
	}


	//Feb 24:   Then,  you would need to allocate the NPCEntityData here   ANDDD CLEAN IT UP!!   Especially that GFC_List
	NPCEntityData* data;
	
	self->layer = ECL_Entity;
	self->team = ETT_NPC;
	self->think = npc_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = npc_update;

//Def file section
	entity_configure_from_file(self, defFile);  //INSTEAD OF this Sprite loading bock underneath
	//position override from the parameters:
	if (position.x >= 0) { slog("Position override for NPC %s",self->name); gfc_vector2d_copy(self->position, position); } //if position is a negative vector, don't override, just use the one from the def file
/*Calling Sprite hard-coded
	self->velocity = gfc_vector2d(1, 0);
	self->sprite = gf2d_sprite_load_all(
		"images/fierce.png",  //change this picture to smth silly
		214,
		200,
		1,		//this works :((  why no configure work
		0
	); //all the sprite info here
	
	self->sprite = gf2d_sprite_load_all(
		"images/enemy.png",  //change this picture to smth silly
		300,
		300,
		1,
		0
	); //all the sprite info here
	self->frame = 0;*/


	self->firstCombat = 1;
	//Me trying to implement data with monsters...  (from all the stuff that was first introduced on Feb 24:
	self->data_free = npc_data_free;   //set the data free function   (y'know.. once I make one)

	/*data = gfc_allocate_array(sizeof(NPCEntityData), 1);
	if (data) {
		data->monsterType = 1000;
	}

	self->data = data;*/



	return self;
}

void npc_data_free(Entity* self) {
	if ((!self) || (!self->data)) return;

	NPCEntityData* data; //create a pointer
	data = self->data;   //make it point.  Especially because "data", in THIS file  is specific to NPCs.  whereas self->data is declared to be a Void pointer

	//other cleanup goes here
	//gf2d_sprite_Free(data->profilePicture);  //for example.  if I had a sprite for profilePicture

	free(data);	//remember: don't wanna just pass in self->data here.  Because that'd be a void pointer. and that no worky
	self->data = NULL;						// ^^^ technically I COULD have...  but!  if we have other things to be cleaned up,  like a Sprite !!  making a PlayerEntityData pointer is the best (only...?) way to access it

}

void npc_think(Entity *self) {
	//GFC_Vector2D screen;
	if (!self) return;			//if I no am, then can not think!
	
	//if (self->position.x + self->bounds.x <= 0) self->velocity.x = 1;		//this was for when I had it bouncing back and forth on the screen
	//if (self->position.x - self->bounds.x >= 1200) self->velocity.x = -1;
	gfc_vector2d_normalize(&self->velocity);
	gfc_vector2d_scale(self->velocity, self->velocity, 1);


	//screen = gf2d_graphics_get_resolution();

}

void npc_update(Entity* self) {

}

/*end of file*/