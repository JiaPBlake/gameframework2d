//an example of how Transitioning between worlds could be done:
#include "simple_logger.h"
#include "simple_json.h"

#include "entity.h"
#include "player.h"
#include "gfc_vector.h"
//I should have exit include player,   since player shouldn't have anything to do with exit


extern Uint8 _NEWENCOUNTER;
//Entity* player = player_get_the();

typedef struct {
	GFC_TextLine world;				/**<The next world to load*/
	GFC_TextLine target;			/**<the exit in the new world to put the player at*/
	GFC_Vector2D placeAt;			/**<any player entering by this exit will start here*/
}ExitInfo;


void exit_think(Entity* self) {  //Because my point of exit will be the cave,  check if the player is colliding with me AND pressing W ofc
	if (!self) return;
	//check if player is colliding with me,  then transition to the next level  And then each entity should have its respective filename to load up the proper world !!!!!



	if (_NEWENCOUNTER) {
		slog("Are you sure?");
	}


//Once I get world_transition working  just stick it in here  and worry about making each cave do different things AND FOR FUCK'S SAKE JUST HARD CODE IT LMAOO
	//also... think about when you can call exit_free()   .. might have to be at the end of the "Are you sure" confirmation when loading the world.   and!! before you call world_transition. bc that would free the Cave without freeing its exit data

}

void exit_free(Entity* self) {
	if ((!self) || (!self->data)) return;
	free(self->data);  //data stands for all the ExitInfo  of the exit Entity.  And none of those 3 things are pointers or anything, so I can free the dynamically allocated pointer "data"
}

//CREATE an exit entity.
Entity* exit_new_entity(GFC_Vector2D position, const char* defFile) {
	Entity *self;
	ExitInfo *data;

	self = entity_new();
	if(!self) return NULL;
	self->think = exit_think;

	gfc_vector2d_copy(self->position, position);
	entity_configure_from_file(self, defFile);  //oh.   wait wait no I could just have Caves here
	data = gfc_allocate_array(sizeof(ExitInfo), 1);



	//*** Think about moving the caves from Object.c  to Exit.c  because all my caves will be entrances/exits to new maps
}
