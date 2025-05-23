#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"

#include "object.h"
#include "player.h"
#include "world.h"		//The only reason I need world is to do the World transitions .   but If I handle everything inside the World code,,,

/*extern Uint8 _NEWENCOUNTER;*/
static Entity* player;
World* active;
extern int keySelectTimer;

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
	Entity* other;
	GFC_List* others;
	others = entity_collide_all(self);  //BE SURE TO DELETE THIS LIST ONCE WE'RE DONE WITH IT	
	if (others) { //if I'm colliding with ANYTHING  [namely the player]:

		other = gfc_list_get_nth(others, 0); //in my game, the player will really only be colliding with 1 thing at a time
		if (other->team & ETT_player) {
			if (gfc_input_command_down("proceed") /*_NEWENCOUNTER*/) {  //IF I press Up
				if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0

					//_NEWENCOUNTER = 1;
					//World* new;	
					GFC_Vector2D pos = gfc_vector2d(60, 400);
					//slog("The type of Cave \"%s\" is: %i", self->name, self->type);
					slog("Are you sure?");
					//Test cave first
					if (self->type & ENT_test) {
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

					SDL_Delay(100);		//delay.  so,, effectively pause the game state  (?)	AND you can't register an "Enter" press for another 10 frames thanks to my keySelectTimer.
					//world_set_active(active);
					//_NEWENCOUNTER = 0;

					keySelectTimer = 10;
				}

			}
			if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
				keySelectTimer--;	//just realized this decrementation should be happening regardless of if I'm pressing a key lmfao.  Try this
			}
		}
	}

	gfc_list_delete(others);


		//also... think about when you can call exit_free()   .. might have to be at the end of the "Are you sure" confirmation when loading the world.   and!! before you call world_transition. bc that would free the Cave without freeing its exit data

}
