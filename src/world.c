#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"

#include "entity.h"
#include "world.h"


World* world_load(const char* filename) {
	GFC_Vector2D frameSize = { 0 };
	Uint32 framesPerLine = 0;
	const char* string = NULL;
	int rowCount, columnCount;
	int i, j;
	SJson* column, * rows, * row;
	SJson* json;
	World* world;
	if (!filename) return NULL;
	json = sj_load(filename);
	if (!json) {
		slog("failed to load world %s", filename);
		return NULL;
	}
	world = world_new();
	if (!world) return NULL;

	rows = sj_object_get_value(json, "tileMap");
	rowCount = sj_array_get_count(rows);
	if (rowCount) {
		
		for(j = 0; j <rowCount; j++ ){
			//some kind of check  idk what
			row = sj_array_get_nth(rows, j);
			if (!row) {
				slog("World %s, tileMap missing rows", filename);
				sj_free(json);
				world_free(world);
				return NULL;
			}
			columnCount = sj_array_get_count(row);
			//After we have both columnCount and Rowcount,  set tileMapSize
			//world->tileMapSize.x = columnCount;
			//world->tileMapeSet.y = rowCount;
		}
		world->tileMap = gfc_allocate_array(sizeof(Uint8), rowCount * columnCount);
		if (!world->tileMap) {
			slog("Failed to allocate tilemap for world %s",filename);
			sj_free(json);
			world_free(world);
			return NULL;
		}

		for (j = 0; j < rowCount; j++) {
			row = sj_array_get_nth(rows, j);
			if (!row) continue;
//We have column count already by the time we get here
			for (i = 0; i < columnCount; i++)	//J  vertically.   i Horizontally
			{
				column = sj_array_get_nth(row, i);
				if (!column) continue;
				//sj_get_uint8_value(column, world->tileMap[] * columnCount + i); //When j is 0. j*0  then add i (which is the # of columns). ANd t does this for each row, j.
				//We need to give it the tileMap.  But which one ?.  Since we're saving the 2d array into a 1D array with gfc_allocate.. how do we determine stuff?

			}
		}
	}
	sj_free(json); //don't forget to clean the json object

	string = sj_object_get_string(json, "name");
	if (string) gfc_line_cpy(world->name,string);

	string = sj_object_get_string(json, "background");
	if (string) {
		world->background = gf2d_sprite_load_image(string);
	}
	//determine systematically  which I think is that giant block above name
	GFC_Vector2D	size;
	string = sj_object_get_string(json, "tileSet");
	if (string) {
		sj_object_get_vector2d(json, "tileSetSize", &frameSize);
		sj_object_get_uint32(json, "tileSetFPL", &framesPerLine);
		world->tileSet = gf2d_sprite_load_all(
			string,
			(Uint32)frameSize.x,
			(Uint32)frameSize.y,
			framesPerLine,
			0);
	}
	sj_free(json);
	return world;

	Uint8* tileMap;

	GFC_List* entity_list;

}

/*Uint8 world_get_rile_at(World* world, GFC_Vector2D position)
{
	if ((!world) || (world->tileMap)) return 0;
	return world->tileMap( (Uint32)position.y = (Uint32)world->tileMapSize.y + (Uint32)position.x);
}

void world_draw(World* world) {
	Uint8 tile;
	int i, j;
	if ((!world) || ) return;
	for(j = 0; j < world->tileMap; j++)
}


void world_free(World* world) {  //What you make you must destroy
	if (!world) return;

	gf2d_sprite_free(world->background);;
	//GFC_Vector2D	size;
	gf2d_sprite_free(world->tileSet);
	//missed smth here
	if (world->tileMap) free(world->tileMap);

	//clean up the entity list.  But if we remove the Player from the World's list,  then we don't have to worry about them being cleaned up
	int i, c;
	Entity* ent;
	if (world->entityList) {
		c = gfc_list_count(world->entityList);
		for (i = 0; i < c; i++) {
			ent = gfc_list_nth(world->entityList, i);
			if (!ent) continue;
			entity_free(ent);
		}
		gfc_list_delete(world->entityList);
	}

	free(world); //now that pointer is dead
}*/

World* world_new() {
	World* world;
	world = gfc_allocate_array(sizeof(World), 1);
	if (!world) {
		slog("ERROR: failed to allocate world");
		return NULL;
	}
	return world;
}

void world_free(World *world) {
	if (!world) return;
	free(world);
}

