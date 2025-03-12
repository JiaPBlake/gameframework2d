//spawn.c
#include "simple_logger.h"
#include "gfc_list.h"
#include "player.h"
#include "monster.h"
#include "object.h"
#include "items.h"

typedef struct {
	const char		*name;
	const char		*defFile;

	Entity*			(*spawn)(GFC_Vector2D position, const char* defFile);
}SpawnPair;

//*Every 'name' provided to the spawn function should match the 'name' in this list, WHICH SHOULD ALSO match the name (of the entity) in each's deffile
static SpawnPair spawn_list[] =
{
	/*{
		"player_start" //name of function we made in the level file  raaaahh idr what this was supposed to be. I think 
	},*/

	{
		"player", //name
		"def/player.def", //name of the def file
		player_new_entity //assign the spawn function
	},
	{
		"fierce_wyvern", //name
		"def/fierce1.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"fierce_wyvern2", //name
		"def/fierce2.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"cunning_drake", //name
		"def/cunning1.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"cunning_wyvern", //name
		"def/cunning2.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"docile_dragon", //name
		"def/docile.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"docile_amphi", //name
		"def/docile_amphi.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"docile_wyv", //name
		"def/docile_wyvern.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"alpha_dragon", //name
		"def/alpha.def", //name of the def file
		monster_new_entity //assign the spawn function
	},
	{
		"cave_f", //name
		"def/cave_f.def", //name of the def file
		object_new_entity //assign the spawn function
	},
	{
		"cave_d", //name
		"def/cave_d.def", //name of the def file
		object_new_entity //assign the spawn function
	},
	{
		"cave_c", //name
		"def/cave_c.def", //name of the def file
		object_new_entity //assign the spawn function
	},
	{
		"cave_t", //name
		"def/cave_t.def", //name of the def file
		object_new_entity //assign the spawn function
	},
	{
		"item_resounding_artifact", //name  (Name as listed in the .Level file for the World.  Since the WORLD is the one spawning us
		"item_resounding_artifact", //name of the item again. To be used in the item search function
		item_new_entity //assign the spawn function	
	},
	{
		"item_drumstick", //name  (Name as listed in the .Level file for the World.  Since the WORLD is the one spawning us
		"item_drumstick", //name of the item again. To be used in the item search function
		item_new_entity //assign the spawn function	
	},
	{
		"item_black_orb", //name  (Name as listed in the .Level file for the World.  Since the WORLD is the one spawning us
		"item_black_orb", //name of the item again. To be used in the item search function
		item_new_entity //assign the spawn function	
	},
	{
		"item_tectonic_stone", //name  (Name as listed in the .Level file for the World.  Since the WORLD is the one spawning us
		"item_tectonic_stone", //name of the item again. To be used in the item search function
		item_new_entity //assign the spawn function	
	},
	{
		"item_trident", //name  (Name as listed in the .Level file for the World.  Since the WORLD is the one spawning us
		"item_trident", //name of the item again. To be used in the item search function
		item_new_entity //assign the spawn function	
	},
	{
		"cave_test", //name
		"def/cave_test.def", //name of the def file
		object_new_entity //assign the spawn function
	},
	{0}
};
//WAIT WAIT OHH NO  hold on.  THIS JSon file is NOT my def file.  It's ANOTHER JSON file that's essentially a LIST of things to spawn!!!!!! ohhh
/*Entity* spawn_entity_from_json(SJson* item) {
	const char* name;
	GFC_Vector2D position = { 0 };
	
	if (!item) return NULL;
	name = sj_object_Get_string(item, "name");
	if (!name) return NULL;
	sj_object_get_vector2d(item, "spawn_Position", &position);
	return spawn_entity(name, position);
	//SOMEONE ELSE IT GOING TO HAVE TO FREE  !!! ?
}*/

/*idr what we used this for in class;   but like I'M going to use it to spawn all
GFC_List* spawn_entity_list(SJson* entityList) {
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
			ent = spawn_entity_from_json(item);

			if (ent)  gfc_list_append(list, ent);

		}

}*/


//iterate through the bigass list up ^ there
Entity* spawn_entity(const char* name, GFC_Vector2D position) {  //what the..  I mean it works now but I genuinely couldn't tell you what problem it had before when th
	//string compar functions don't check for NULL
	if (!name) { slog("Spawn failed. No name provided"); return NULL; }
	
	int i;
	for (i = 0; spawn_list[i].name != NULL; i++) { //the 0 @ the end of our list kinda stands for the name.  that name == NULL
		if (gfc_strlcmp(spawn_list[i].name, name) == 0) {   //his string compare funciton comes with a little side effect.  Will match the first Substring. But also matches the length, if the length of the strings are not the same, it will not return as matching
			if (spawn_list[i].spawn) {
				//slog("Spawning entity %s", name);   //Jlog
				return spawn_list[i].spawn(position, spawn_list[i].defFile);  //calling the function AT the memory address of our (function) pointer :D
			}
		}
		
	}
	slog("Entity '%s' not found in entity list", name);
	return NULL;  //default case.
}

//end offile