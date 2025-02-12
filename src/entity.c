//J START - Entity c file  for the .h file
#include "simple_logger.h"
//#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "entity.h"

//Same design pattern as what's in gf2dSprite.c  -- just instead of managing Sprites,  we're managing entities
typedef struct {  //our singleton entity system/manager.  our big-ass list of entities
	Uint32		entity_max;
	Entity		*entity_list;		//this will be a POINTER  to a list of entities

}EntitySystem;

static EntitySystem entity_system = { 0 };   //Our GLOBAL variable, since it's a singleton.  But make it static so it's only local to this file

void entity_system_close() {  //not..? copied from entity.h  ?  apparently we don't need this Close function known in the Header file. Didn't catch Why he said.  but smth smth it runs directly after  ?
	if (entity_system.entity_list != NULL)  //if our system still has entities in it  (if the pointer does not point to NULL)
	{ //free all the entities
		entity_system_free_all(NULL); //J SPECIFIC. When I want to clear absolutely everything ('cause I'm shutting down the game), pass NULL to the ignore
		free(entity_system.entity_list); //entity_list was, ofc, dynamically allocated  so we'll use the standard library functino to free it once we're done with it
		slog("Entity List Freed");
	}
	entity_system.entity_list = NULL; //reset the entity list pointer
	memset(&entity_system, 0, sizeof(EntitySystem)); //completely unecessary.  This doesn't free nor clean up anything. Just sets the data there to be 0
	slog("entity system closed");
}

void entity_system_init(Uint32 maxEnts) {	//Caught up
	if (entity_system.entity_list) {
		slog("Cannot initialize more than one Entity System/List. One is already active");
		return;
	}
	if (!maxEnts) {
		slog("cannot initialize entity system for zero entities");
		return;
	}
	entity_system.entity_list = gfc_allocate_array( sizeof(Entity), maxEnts);	//highkey double check the video for this honestly  just 'cause I wanna be sure I'm giving it 
	if (!entity_system.entity_list) {
		slog("failed to access entity list");
	}
	entity_system.entity_max = maxEnts;		//I think?? This line is the line where you initialize the Entity System's maxEnts field ? That's what is done in Sprite, and it's the only thing that makes sense in this spot.
	atexit(entity_system_close);  //I'ma highkey need him to talk again about When the atExit function takes place. I remember his emphasis on order... . Oh wait is this just when the Game itself closes?
	slog("entity system initialized");
}

Entity* entity_new() { //since we initialized our entity list to 0, this function should return NULL automatically if we never init() the list.
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (entity_system.entity_list[i]._inuse)  //If this entity (from the list our pointer points to)  IS in use (_inuse flag is == 1)
			continue;
		memset(&entity_system.entity_list[i], 0, sizeof(Entity));   //Set the memory allocated at this spot to 0
		//memset requires the Memory address OF THAT specific, indexed entity.
		entity_system.entity_list[i]._inuse = 1;  //set the inuse flag to 1
		return &entity_system.entity_list[i]; //return the address of the thing we just allocated
		//If any Entity needs things to be initalized ahead of time,  HERE is where we should do that.  So things like Default Color and Scale

	}
	slog("failed to allocate new entity: list full");
	return NULL; //return NULL outside the for loop
}

void entity_system_free_all(Entity *ignore) {  //JUST iterates and calls  entity_free()
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (&entity_system.entity_list[i] != ignore && entity_system.entity_list[i]._inuse) {
			entity_free(&entity_system.entity_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}

void entity_free(Entity *self) {
	if (!self) return;			//NEVER TRUST A POINTER.   Always check in case it doesn't exist.  and if it doesn't exist,  Stop. Return. END
	//So far, the entity class that I have ONLY has one dynamically allocated member, the sprite.
	//  If I add anything else, I will need this funciton to free that specifically as well
	if (self->sprite) { //if the entity has a sprite.  Free the sprite  -- function yoikned from Sprite.c  bc sprites are managed by the Sprite Manager.
		gf2d_sprite_free(self->sprite); //frees the spot IN the masterlist of Sprites.   gf2d_sprite.h takes care of actually deleting things.
	}
	memset(self, 0, sizeof(Entity));
	self->_inuse = 0; //Set its inuse flag to 0
	//free();   //I think?????  Like this is all I need to free a piece of memory I use, right??
}

void entity_draw(Entity* self) {
	if (!self) return;
	if (!self->sprite) return; //can't work without a sprite
	//now call the draw function for a sprite
	gf2d_sprite_draw(self->sprite,
		self->position,
		NULL,
		NULL,
		&self->rotation,
		NULL,
		NULL,
		(Uint32) self->frame);
}

void entity_system_draw_all() {  //this used to just be "draw"  ?? maybe I just forgot to add draw() 
	int  i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse) continue; //if the entity is NOT in use,  don't draw it

		entity_draw(&entity_system.entity_list[i]);
	}
}

void entity_system_think_all() {  //up to date ??  Can't remember where the actual think function is like.. assigned or if we even did that yet
	int  i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse)
			continue; //if the entity is NOT in use,  don't think
		if (!entity_system.entity_list[i].think)   //if it DOES NOT have a Think function
			continue;
		//Skipped making a  think(Entity *self)  function in this file.  bc this is the only call we need
		entity_system.entity_list[i].think(&entity_system.entity_list[i]);  //call the think funcion with ourselves as the argument
	}
}

//to execute the decisions Think just made
void entity_system_update_all() {  //up to date ??
	int  i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse)
			continue; //if the entity is NOT in use,  don't update it
		if (!entity_system.entity_list[i].update)   //if it DOES NOT have a Update function
			continue;
		entity_system.entity_list[i].update(&entity_system.entity_list[i]);
	}
}

//After we delved into the creation of Definition files:

/*void entity_configure(Entity* self, SJson* json) {
	GFC_Vector4D bounds;
	const char* filename = NULL;
	if ((!self) || (!json)) return;

	filename = sj_object_get_string(json, "sprite");
	if (filename) {

		sj_object_get_vector2d();  //which we grabbed from gfc_config.h
		self->sprite = gf2d_sprite_load_all(
			filename,
			(Uint32)frameSize.x,
			(Uint32)frameSize.y,
			framesPerLine,
			0
		);
	}
	sj_object_get_float(json, "speedMax", &self->speedMax);
	sprite = sj_object_get_string(json, "name");
	if (sprite) gfc_line_cpy(self->name, sprite);  //something something  Copy A into B  and make sure it's not longer than the length of a Line.
	//He's talking about the Bounding boxes here I think

	sj_object_get_vector4d(json, "bounds", &bounds);
	//gfc_shape.h   has:
	self->bounds = gfc_rect_from_vector4(bounds);
}


//Also making an   \|/ to configure it from the filename, which is significantly easier.  This just
void entity_configure_from_file(Entity* self, const char* filename) {
	SJson* json;
	if (!self) return;
	json = sj_load(filename);
	if (!json) return;
	entity_configure(self, json);
	//close it

}

//He has an Update Position function here
void eneitty_update_position(Entity* self) {
	GFC_Vector2D screen;
	if (!self) return;

	gfc_vector2d_normalize(&self->velocity);	//I could've sworn we did this already somewhere else..  maybe player !
	gfc_vector2d_scale(self->velocity, self->velocity, self->speedMax);
	screen = gf2d_graphics_get_resolution();
	gfc_vector2d_add(self->position, self->position, self->velocity);
	if (self->position.x + self->bounds.x < 0) self

}
*/

/*eol@eof*/
