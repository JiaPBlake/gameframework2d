#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"

#include "object.h"
#include "player.h"
#include "world.h"

extern Uint8 _NEWENCOUNTER;
static Entity* player;
World* active;

Entity* object_new_entity(GFC_Vector2D position, const char* defFile) {
	Entity* self;
	self = entity_new();  //create the new entity [spot in our total Entity List]. Where-in its _inuse flag will be set.
	if (!self) {
		slog("failed to spawn a new object entity");
		return NULL;
	}

	self->layer = ECL_Entity;
	self->layer |= ECL_World;
	self->team = ETT_cave;
	self->think = object_think;  //Set my entity's Think function   TO the function we make in this file!
	self->update = NULL/*object_update*/;

	//Def file section
	entity_configure_from_file(self, defFile);

	//position override from the parameters:
	if (position.x >= 0) { gfc_vector2d_copy(self->position, position); } //if position is a negative vector, don't override, just use the one from the def file
	//ohhhh fuck me I literally forgot to RETURN the entities here... no wonder only the caves would mess up with printing names and being freed .

	//I tried to get the player but completely forgot the player is spawned in LAST because of Entity draw orderrrr raghh
	player = player_get_the(); //get the player so we know where it is  and DON'T have to re-get it in the thinking for loop
	//if(player) slog("Player name is: %s", player->name);
	active = world_get_active();
	//if (active) slog("We've got an active world. And its name is: %s", active->name);
	return self;
}

void object_think(Entity* self) {  //Because my point of exit will be the cave,  check if the player is colliding with me AND pressing W ofc
	if (!self) return;
	
	//check if player is colliding with me,  then transition to the next level  And then each entity should have its respective filename to load up the proper world !!!!!
	//If I didn't make each individual cave check to MAKE SURE the player is currently colliding with IT.  then my global flag used to transition worls would cause the FIRST cave in my entity list to carry out its transition. And not the specific cave I clicked on
	Entity* other;
	GFC_List* others;
	others = entity_collide_all(self);  //BE SURE TO DELETE THIS LIST ONCE WE'RE DONE WITH IT	
	if (others) { //if I'm colliding with ANYTHING  [namely the player]:

		other = gfc_list_get_nth(others, 0); //in my game, the player will really only be colliding with 1 thing at a time
		if (other->team & ETT_player) {
			if (_NEWENCOUNTER) {  //IF I press Up
				//World* new;	
				GFC_Vector2D pos = gfc_vector2d(60, 400);
				//slog("The type of Cave \"%s\" is: %i", self->name, self->type);
				slog("Are you sure?");
				//Test cave first
				if (self->type & ENT_MAX) { 
					slog("The type of %s is: %i", self->name, self->type);
					world_transition(world_get_active(), "def/levels/testLevel.level", pos);
				}
				else if (self->type & ENT_fierce) {   //... :|  for some reason this always equates to 1..  can't remember why but I think it was smth smth the order. Fierce came first.  Maybe I never properly declared types..? don't remember actually
					slog("The type of %s is: %i", self->name, self->type);
					world_transition(world_get_active(), "def/levels/fierce_domain.level", pos);
				}
				
				else if (self->type & ENT_docile) {   
					slog("The type of %s is: %i", self->name, self->type);
					world_transition(world_get_active(), "def/levels/docile_domain.level", pos);
				}
				
				else if (self->type & ENT_cunning) {   
					slog("The type of %s is: %i", self->name, self->type);
					world_transition(world_get_active(), "def/levels/cunning_domain.level", pos);
				}
				else if (self->type & ENT_treasure) {   
					slog("The type of %s is: %i", self->name, self->type);
					world_transition(world_get_active(), "def/levels/treasure_trove.level", pos);
				}

				
				//slog("Active world's name SHOULD BE LEVELS and it is: %s",new->name);

				SDL_Delay(1000);
				//world_set_active(active);
				_NEWENCOUNTER = 0;
			}
		}
	}

	gfc_list_delete(others);


	//Once I get world_transition working  just stick it in here  and worry about making each cave do different things AND FOR FUCK'S SAKE JUST HARD CODE IT LMAOO
		//also... think about when you can call exit_free()   .. might have to be at the end of the "Are you sure" confirmation when loading the world.   and!! before you call world_transition. bc that would free the Cave without freeing its exit data

}
