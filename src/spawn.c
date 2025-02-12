//spawn.c
#include "simple_logger.h"
#include "gfc_list.h"
#include "monster.h"

typedef struct {
	const char		*name;
	const char		*defFile;

	Entity(*spawn)(GFC_Vector2D position, const char* defFile);
}SpawnPair;

static SpawnPair spawn_list[] =
{
	/*{
		"player_start" //namae of function we made in the level file
	},*/

	{
		"monster", //name
		"defs/monster.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{0}
};

/*Entity* spawn_entity_from_json(SJson* item) {
	const char* name;
	GFC_Vector2D position = { 0 };
	
	if (!item) return NULL;
	name = sj_object_Get_string(item, "name");
	if (!name) return NULL;
	//then get the position  sj_object_get_vector2d   from item, save into "position", into &position
	return spawn_entity(name, position);
	//SOMEONE ELSE IT GOING TO HAVE TO FREE  !!! ?
}*/

GFC_List* spawn_entity_list(SJson *entityList) {
	int i, c;
	SJson* item;
	GFC_List* list;
	if (!entityList) return NULL;
	list = gfc_list_new();
	if (!list) return NULL;
	/*c = arraucount;
		for (i = 0; i, c) {
			item = sj_array_nth()
				if (!item) continue;
			enter = spawn_entity_from_json(item);

			if (ent)  gfc_list_append(list, ent);

	}*/

}


//iterate through the bigass list up ^ there
/*Entity *spawn_entity(const char* name, GFC_Vector2D position) {
	//string compar functions don't check for NULL

	if (!name) { slog("Spawn failed. No name provided"); return NULL; }
	
	int i;
	for (i = 0; spawn_list[i].name != NULL; i++) { //the 0 kinda stands for the name.  that name == NULL
		if (gfc_strlcmp(spawn_list[i].name, name) == 0) {   //his string compare funciton comes with a little side effect.  Will match the first Substring. But also matches the length, if the length of the strings are not the same, it will not return as matching
			if (spawn_list[i].spawn) {
				return spawn_list[i].spawn(position, spawn_list[i].defFile);  //calling the function AT the memory address of our (function) pointer :D
			}
		}

	}
	return NULL;  //default case.
}*/

//end offile