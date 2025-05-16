#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "monster.h"
#include "world.h"

/*typedef enum {
	MS_Idle,
	MS_Patrol,
	MS_Hunt,
	MS_Attack,
	MS_MAX
}MonsterStates; //In class demo of Monster States. Instead of States, I'll be using this to implement my Types*/

typedef enum {
	MT_Fierce,
	MT_Docile,
	MT_Cunning,
	MT_MAX
}MonsterType; //dk if they should be bit flags tho

typedef struct {
	GFC_List		*pathToPlayer; //list of world coordinates to the player  :0 woah
	MonsterType		monsterType;   //specific to the monster.  and NO ONE outside the Monster world needs to know what these are

	//List of Attacking Moves		should be configured with configure_attack
	//List of possible dialogue options


}MonsterEntityData;


extern Uint8 turn;


Entity* monster_new_entity(GFC_Vector2D position, const char* defFile) //Now that I'm configuring the spawn.h file, I'm going to iterate through my list of Spawnable entities
{															// and call this function providing the position to spawn it AND the DefFile to be used in Configuring this entity
	Entity* self;
	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	if (!self) {
		slog("failed to spawn a new monster entity");
		return NULL;
	}


	//Feb 24:   Then,  you would need to allocate the MonsterEntityData here   ANDDD CLEAN IT UP!!   Especially that GFC_List
	MonsterEntityData* data;

	self->layer = ECL_Entity;
	self->team = ETT_monsters;
	self->think = monster_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = monster_update;

//Def file section
	entity_configure_from_file(self, defFile);  //INSTEAD OF this Sprite loading bock underneath
	//position override from the parameters:
	if (position.x >= 0) { //if position is a negative vector, don't override, just use the one from the def file
		//slog("Position override for Monster %s.  Should be at X position: %f",self->name, position.x);
		gfc_vector2d_copy(self->position, position);
	}
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
	self->data_free = monster_data_free;   //set the data free function   (y'know.. once I make one)

	/*data = gfc_allocate_array(sizeof(MonsterEntityData), 1);
	if (data) {
		data->monsterType = 1000;
	}

	self->data = data;*/



	return self;
}

void monster_data_free(Entity* self) {
	if ((!self) || (!self->data)) return;

	MonsterEntityData* data; //create a pointer
	data = self->data;   //make it point.  Especially because "data", in THIS file  is specific to Monsters.  whereas self->data is declared to be a Void pointer

	//other cleanup goes here
	//gf2d_sprite_Free(data->profilePicture);  //for example.  if I had a sprite for profilePicture

	free(data);	//remember: don't wanna just pass in self->data here.  Because that'd be a void pointer. and that no worky
	self->data = NULL;						// ^^^ technically I COULD have...  but!  if we have other things to be cleaned up,  like a Sprite !!  making a PlayerEntityData pointer is the best (only...?) way to access it

}

void monster_think(Entity *self) {
	//GFC_Vector2D screen;
	if (!self) return;			//if I no am, then can not think!
	
	//if (self->position.x + self->bounds.x <= 0) self->velocity.x = 1;		//this was for when I had it bouncing back and forth on the screen
	//if (self->position.x - self->bounds.x >= 1200) self->velocity.x = -1;
	gfc_vector2d_normalize(&self->velocity);
	gfc_vector2d_scale(self->velocity, self->velocity, 1);



	//screen = gf2d_graphics_get_resolution();

}

void monster_update(Entity* self) {
	/*gfc_vector2d_add(self->position, self->position, self->velocity);
	if (self->position.x + self->bounds.x < 0) self->position.x = 0 - self->bounds.x;
	if (self->position.y + self->bounds.y < 0) self->position.y = 0 - self->bounds.y;
	if (self->position.x - self->bounds.x > 1200) self->position.x = 1200 + self->bounds.x;
	if (self->position.y - self->bounds.y > 700) self->position.y = 700 + self->bounds.y;*/

	//I was going to implement a special bounds check, so that each monster could be like.. 20 pixels IN the ground, but... nah it's way easier to just do it based on def file
	//since each monster (1 dragon per domain btw)  will be especially placed into the map
	/*GFC_Rect rect = { 0 };
	GFC_Vector2D ground = { 0 };
	gfc_rect_copy(rect, self->bounds); //copy our bounds rectangle into our new rect
	gfc_vector2d_add(rect, rect, self->position); //ALL OF MY ENTITIES.  have their position determined with the camera offset included, because that's how I'm. DRAWING them.

	if (world_test_shape(world_get_active(), gfc_shape_from_rect(rect))) { //my y position (taking into account my bounds) should be 614 compared to ground's 640 
		//slog("MY y position = %f",self->position.y); 
		ground = world_get_ground();
		//slog("Ground y position = %f", ground.y);
		self->position.y = ground.y + self->bounds.y;
	}
	*/
}

/*end of file*/