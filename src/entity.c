//J START - Entity c file  for the .h file
#include "simple_logger.h"
//#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"
 
#include "entity.h"
#include "camera.h"
#include "moves.h"			//So that I can call  ent_configure_moves  or whatever it's called.  DURING the entity_configure function while I have access to its def file (which will have the names of each move in a list)

extern Uint8 _DRAWBOUNDS;

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
		//If ALL Entities needs things to be initalized ahead of time,  HERE is where we should do that.  So things like Default Color and Scale

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
	if (!self || !self->_inuse) return;			//NEVER TRUST A POINTER.   Always check in case it doesn't exist.  and if it doesn't exist,  Stop. Return. END
	//So far, the entity class that I have ONLY has one dynamically allocated member, the sprite.
	if (self->sprite) { //if the entity has a sprite.  Free the sprite  -- function yoikned from Sprite.c  bc sprites are managed by the Sprite Manager.
		gf2d_sprite_free(self->sprite); //frees the spot IN the masterlist of Sprites.   gf2d_sprite.h takes care of actually deleting things.
	}
	slog("Freeing Entity %s",self->name);
	//  If I add anything else, I will need this funciton to free that specifically as well
	if (self->data_free) {
		slog("Calling Entity %s's data free function.",self->name);
		
		if (self->team == ETT_item) {
			//I will most likely NEVER use this branch .   but just in case
			slog("The entity I'm freeing is an item.  Casting self to be an item Pointer");
			self->data_free( (void*)self); //This SHOULD work.  I apparently don't even need to cast it back  ??
		}
		else {
			slog("About to enter Data Free function for Entity name: %s",self->name);
			self->data_free(self);		//if our Entity has data to free. Free that data
		}
	}  
	
	if (self->move_list) {
		gfc_list_delete(self->move_list);
	}
	memset(self, 0, sizeof(Entity));
	self->_inuse = 0; //Set its inuse flag to 0
}

void entity_draw(Entity* self) {
	if (!self) return;
	if (!self->sprite) return; //can't work without a sprite
	
	GFC_Vector2D camera, position;  //Everything I draw will now have to honor the Camera's position J CAMERA
	camera = camera_get_offset();
	gfc_vector2d_add(position, camera, self->position);

	//So now!  in the 2 places we have "position"  there used to be self->position

	GFC_Rect rect = { 0 };
	//now call the draw function for a sprite
	gf2d_sprite_draw(self->sprite,
		/*self->*/position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
		NULL,			//scale
		&self->center,		//center which is a 2D vector
		&self->rotation,	//rotation
		&self->flip,		//flip
		NULL,		//colorShift
		(Uint32) self->frame);
	//he's adding this for Bound-related things  Just as a means to give yourself some trace,  SEE the bounding box when you run the game
	if (_DRAWBOUNDS)
	{
		gfc_rect_copy(rect, self->bounds); //copy our bounds rectangle into our new rect
		gfc_vector2d_add(rect, rect, /*self->*/position); //This is for offset purposes. TO draw it AT the position of the player. Because his vector add is a macro. So long as the things have an x and y, they can be added ! so this works for rectangles
		gf2d_draw_rect(rect, GFC_COLOR_RED);
	}
}

void entity_system_draw_all() {  //this used to just be "draw"  ?? maybe I just forgot to add draw() 
	int  i;
	Entity* p = NULL;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse) continue; //if the entity is NOT in use,  don't draw it
		if (gfc_strlcmp(entity_system.entity_list[i].name, "player") == 0) {
			p = &entity_system.entity_list[i]; //save the player  and draw it last -- outside the for loop.
			continue;
		}
		entity_draw(&entity_system.entity_list[i]);
	}
	if ( p ) {
		entity_draw(p);
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
		//if (gfc_strlcmp(entity_system.entity_list[i].name, "cave_f") == 0) { slog("What the fuck: %i", i); }
		
		//This was for testing. I put the Player Think Battle function in a file other than player.c  Turns out the only problem was that I didn't include  gfc_input in the battle.c code, so it kept triggering the "key == down" branch
		//if (gfc_strlcmp(entity_system.entity_list[i].name, "player") == 0) { slog("Player's think function address is:  %p", entity_system.entity_list[i].think); }
		
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


int get_entity_type_by_name(const char *name) {
//***  strcmp   -- the LONGER word  must be 2nd.  to match substrings.  So for shit like items  whose names are ALL gonna be prefixed with 'item_..'. ANYWAY !!
		//lmfao  nvm.  Even if strcmp does it,  it returns a different value than expected.
		//instead I will be using the lovely strNcmp  which takes a # of characters to look through before it stops (as opposed to being NULL terminated)

	if (strncmp("fierce_", name, 7) == 0 || strncmp("cunning_", name, 8) == 0 || strncmp("docile_", name, 7) == 0 || strncmp("alpha_", name, 6) == 0 ) {
		//slog("When determining what Entity type, I found: Dragon!  of name %s",name);
		return ETT_monsters;
	}
	else if (strncmp("item_", name, 5) == 0) {
		//slog("When determining what Entity type, I found: Item!");
		return ETT_item;
	}
	else if (strncmp("cave_", name, 5) == 0) {
		//slog("When determining what Entity type, I found: Cave!");
		return ETT_cave;
	}
	else if (strncmp("npc_", name, 4) == 0) {
		//slog("When determining what Entity type, I found: NPC!");
		return ETT_NPC;
	}

	//slog("Compared all possible names and did not find a corresponding Entity Type");
	return -1;
}


//After we delved into the creation of Definition files:
void entity_configure(Entity* self, SJson* json) {
	if ((!self) || (!json)) return;

	//Let's get started .
	char *ent_name = sj_object_get_string(json, "name");
	if (ent_name) {
		gfc_line_cpy(self->name, ent_name);  //something something  Copy A into B  and make sure it's not longer than the length of a Line.
//slog("Name copied: %s",ent_name);
	}
	const char* filename = NULL;
	filename = sj_object_get_string(json, "sprite");
	if (filename) { //if it has a sprite
		GFC_Vector2D sp_sz = {0};
		sj_object_get_vector2d(json, "sprite_size", &sp_sz);  //which we grabbed from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %f", (Uint32)sp_sz.x);
		//For some reason this slog prints 0... but the saving from the sj obviously worked bc it loaded the sprite properly...
		Uint32 framesPerLine;
		sj_object_get_int32(json, "spriteFPL", &framesPerLine);
		self->sprite = gf2d_sprite_load_all(
			filename,
			(Uint32)sp_sz.x,
			(Uint32)sp_sz.y,
			framesPerLine,
			0
		);
		//if (!self->sprite->surface) { slog("Entity has no sprite surface"); }  //for Window comparison... these don't have surfaces
		if (!self->sprite) { slog("Sprite did not load for Entity %s",self->name); }
		self->sprite_size = sp_sz;
		self->center = gfc_vector2d(sp_sz.x*0.5, sp_sz.y*0.5);
		self->framesPerLine = framesPerLine;
		self->frame = 0; //Since frame 0 will be the default for every entity,  just et the first frame here in the configure function
		self->flip = gfc_vector2d(0, 0);

		GFC_Vector4D bounds; //Use this as a 4D vector to save the numbers from the json file. THEN uset he get Rect from 4D vector function to save it into the entity's bounds
		sj_object_get_vector4d(json, "bounds", &bounds);
		self->bounds = gfc_rect_from_vector4(bounds);

	}
	
	GFC_Vector2D pos = {0};
	sj_object_get_vector2d(json, "spawn_Position", &pos);
	self->position = pos;

	GFC_Vector2D vel = { 0 };
	sj_object_get_vector2d(json, "velocity", &vel);
	self->velocity = vel;
	gfc_vector2d_normalize(&self->velocity);

	GFC_Vector2D speedMax = { 0 };
	sj_object_get_vector2d(json, "speed_Max", &speedMax);
	if (speedMax.x) {
		self->speedMax = speedMax;
	}
	
	// Just.. don't normalize this vector. I don't want that, I'm not here for direction, I need magnitude
	GFC_Vector2D accel = { 0 };
	sj_object_get_vector2d(json, "acceleration", &accel);
	if (accel.y != 0) {
		//gfc_vector2d_copy(self->acceleration, (*accel));
		self->acceleration = accel;
		//slog("Acceleration in the y direction for this entity: %s is: %f",self->name, accel.y);
		//gfc_vector2d_normalize(&self->acceleration);
	}
	/*float accel;
	sj_object_get_float(json, "acceleration", &accel);
	if (accel) {
		self->acceleration.y = accel;
		slog("Acceleration in the y direction for this entity: %s is: %f", self->name, self->acceleration.y);
	}*/


	//Determine entity's type
	self->type = ENT_none;
	//slog("Entity %s Configured to have type none",self->name);
	const char* ent_type = sj_object_get_string(json, "type");
	if (ent_type) {
		if (gfc_strlcmp(ent_type, "fierce") == 0) {
			//slog("Fierce entity found: %s", self->name);
			self->type = ENT_fierce;
		}
		else if (gfc_strlcmp(ent_type, "docile") == 0) {
			//slog("Docile entity found: %s", self->name);
			self->type = ENT_docile;
			//slog("Entity %s's type is: %i", self->name, self->type);
			//if (self->type & ENT_docile) slog("Docile's y position is: %f. Center is: %f", self->position.y,self->center.y);
		}
		else if (gfc_strlcmp(ent_type, "cunning") == 0) {
			//slog("Cunning entity found");
			self->type = ENT_cunning;
		}
		else if (gfc_strlcmp(ent_type, "item") == 0) {
			//slog("Treasure entity found");
			self->type = ENT_treasure;
		}
		else if (gfc_strlcmp(ent_type, "npc") == 0) {
			//slog("Treasure entity found");
			self->type = ENT_npc;
		}
		else if (gfc_strlcmp(ent_type, "all") == 0) {
			slog("Alpha entity found");
			self->type = ENT_fierce | ENT_docile | ENT_cunning;

		}
		else if (gfc_strlcmp(ent_type, "test") == 0) {
			slog("Test cave found");
			self->type = ENT_test;

		}
		//gfc_line_cpy(self->name, ent_name);
	}

	int health;
	sj_object_get_int(json, "health", &health);
	self->health = health;
	if (self->team & ETT_monsters && self->health <= 0) {
		slog("We have a monster with no health.. that's a nono.");
	}
	self->firstCombat = 0;
	/*  Just realized I added a "domain" key in my cave def files.   I coulddd configure this,  but truth be told I'd much rather have the next domains hard-coded FOR NOW at least .  Once we get to procedural generation....... yeah .
	const char* domain = sj_object_get_string(json, "domain");
	if (domain) {

	}*/


//==============================	CONFIGURING MOVES

	//I want to make a list of the strings we extract from the def file...   OR
	self->move_list = gfc_list_new();
	GFC_List* moveNames;
	
	SJson* moveList, *movy;
	int i, c;


	moveNames = gfc_list_new();
	//Test this first,  but honestly I think I wanna use gfc_text_word for this..
	const char* move_name = NULL;

	//Extract the list of Move Names 
	moveList = sj_object_get_value(json, "moveList"); //This is now a list/array of N objects
	if (moveList) {
		c = sj_array_get_count(moveList);
		//slog("The number of entities that belong to this World's list is: %i",entCount);	//Jlog

		for (i = 0; i < c; i++) {

			movy = sj_array_get_nth(moveList, i); //Get the specific json object, which.. only contains the name for now
			
			//I  doo think I kinda wanna split it up into     key: AttackList    and key: ConverseList once I implement dialogue
			
			
			if (movy) {
				move_name = sj_object_get_string(movy, "name");    //Get the name of the entity/JSon object
				if (move_name) {  //append the string to the list
					//slog("On iteration: %i,  the name for the move is: %s", i, move_name);
					gfc_list_append(moveNames, move_name);
				}

			}
		}

		configure_moves_for_ent(moveNames, self->move_list);
	}
	else { slog("Entity name %s dose not have a moveList", self->name); }
	gfc_list_delete(moveNames);

}


//Also making this function to configure it from the filename, which is significantly easier.  This just
void entity_configure_from_file(Entity* self, const char* filename) {
	SJson* json;
	if (!self) return;
	json = sj_load(filename);
	if (!json) return;
	entity_configure(self, json);
	//close it
	sj_free(json);   //COMPLETELY forgot to close it for the first 4 weeks of having this funciton .
}


//He has an Update Position function here
/*void entity_update_position(Entity* self) {
	GFC_Vector2D screen;
	if (!self) return;

	gfc_vector2d_normalize(&self->velocity);	//I could've sworn we did this already somewhere else..  maybe player !
	gfc_vector2d_scale(self->velocity, self->velocity, self->speedMax);
	screen = gf2d_graphics_get_resolution();
	entity_move(self);
	//gfc_vector2d_add(self->position, self->position, self->velocity); I believe this function is instead in his entity_move() function

	//And then he has a section where he checks The Bounds of the Screen - so that we don't move offscreen

	if (self->position.x + self->bounds.x < 0) self

}*/

	//This might highkey be useful, depending on how I implement collision.  If I implement ALL collisions to prevent clipping, a.k.a I can stand on my enemies. This might come in handy for my skeleton
/*
int entity_layer_check(Entity* self, EntityCollisionLayers layer) {
	if (!self) return;

	return self->layer & layer; //will return True  IF and only IF the layer is true
}

void entity_set_collision_layer(Entity* self, EntityCollisionLayers layer) {
	if (!self) return;

	self->layer |= layer; //turn the Layer on.
}

void entity_remove_collision_layer(Entity* self, EntityCollisionLayers layer) {
	if (!self) return;

	self->layer &= ~layer; //turn the Layer['s SPECIFIC BIT] off.
}*/


int entity_collision_check(Entity* self, Entity* other) { //the way he's doing Entity Collision
	GFC_Rect bounds1, bounds2;
	if ((!self) || (!other))  return 0;

	
	//if (!entity_layer_check()) return 0;   //if there IS no layer, return 0.  So now we need to be sure that we set the mask to allow things to collide

	//Need to compare the bounds in WORLD space,   not in Entity space.

	gfc_rect_copy(bounds1, self->bounds);
	gfc_rect_copy(bounds2, other->bounds);

	//To move them to worldspace,  I now need to add these bounds  to our position...  which is what I did  :((
	gfc_vector2d_add(bounds1, bounds1, self->position);
	gfc_vector2d_add(bounds2, bounds2, other->position);

	return gfc_rect_overlap(bounds1, bounds2);

}

//ohh chat what is this,,	 WE GOIN' THROUGH THE WHOLE LIST  OH YEAH
GFC_List* entity_collide_all(Entity* self) {  //Works and up to date
	if (!self) return NULL;
	//ohhhhhh we goin through the whole list-
	
	GFC_List* entities;
	entities = gfc_list_new();

	int i;
	for (i = 0; i < entity_system.entity_max; i++) { //iterate through the list of all our entities and see which ones we're colliding with
		if (!entity_system.entity_list[i]._inuse) continue;  //if not in use. Skip
		if (self == &entity_system.entity_list[i]) continue;  //Don't wanna collide against myself

		if (entity_collision_check(self, &entity_system.entity_list[i])) {
			gfc_list_append(entities, &entity_system.entity_list[i] );
		}

	}

	if (!gfc_list_count(entities)) { //if, by the end of my for loop, I never collided with anything:
		gfc_list_delete(entities);
		return NULL;
	}
	return entities; //return the list. Please be sure to delete it after calling this function
}

entity_move(Entity* self) {
	//he has a Bounds rect
	GFC_Shape bounds;

	//and a position vector
	GFC_Vector2D position;

	gfc_vector2d_add(position, self->position, self->velocity);
	//and then again, updating SELF'S Velocity by adding Acceleration
	gfc_vector2d_add(self->velocity, self->velocity, self->acceleration);
	
	//check for movement collision:
	bounds = gfc_shape_from_rect(self->bounds);
	gfc_shape_move(&bounds, position);
	//if(self->layer)

	//if (I collide at new position) cancel the movement;
	//if my entity is ON the world layer:
	//if self->layer & ECL_World

}

/*eol@eof*/
