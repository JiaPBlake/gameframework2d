#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"

#include "entity.h"
#include "world.h"

/* ONLY used this to help me fix the bugs I had before.  of which, the bug was that I forgot the ==0 for the continue statement in the draw .
World* world_test_new() {
	int i;
	int width = 75, height = 45;
	World* world;
	
	world = world_new(width, height);

	if (!world) return NULL;

	world->background = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
	world->tileSet = gf2d_sprite_load_all(
		"images/backgrounds/ts1.png",
		16,
		16,
		1,
		1);
	//slog("TileSet's frame width = %i", world->tileSet->frame_w);
	for (i = 0; i < width; i++) {
		world->tileMap[i] = 1;
		world->tileMap[i + ((height - 1)*width)] = 1;
	}
	for (i = 0; i < height; i++) {
		world->tileMap[i * width] = 1;
		world->tileMap[i * width + (width - 1)] = 1;
	}
	return world;
}
*/

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
	//slog("JSON world object created successfulyl"); //Jlog
	world = world_new();
	if (!world) { return NULL; }
	//slog("World created successfully");  //Jlog
	rows = sj_object_get_value(json, "tileMap"); //rows is the 2D array == tileMap
	//slog("Rows array obtained successfully");  //Jlog
	rowCount = sj_array_get_count(rows);
	if (rowCount) {
		
		//for(j = 0; j <rowCount; j++ ){ //for every Row
			//some kind of check  idk what
		row = sj_array_get_nth(rows, 0); //don't.. get the J'th row.. just get the 0th, they're all the same. MINE will be... at least
		if (!row) {
			slog("World %s, tileMap missing rows", filename);
			sj_free(json);
			world_free(world);
			return NULL;
		}
		columnCount = sj_array_get_count(row); //count the columns in that Row (json object)
		//After we have both columnCount and Rowcount,  set tileMapSize
		world->tileMapSize.x = columnCount;
		world->tileMapSize.y = rowCount;
		//idk why I had this^ for loop up there... I just-  I think we copied FROM the bottom half to use into the top.. and,, maybe he got rid of it and I didn't see?
//But regardless,  my tileMAP would be symmetric.  There would just be certain tiles turned on, and some turned off
//There should be no reason for me to iterate through the rows JUST to obtain the rowCount and columnCount for the first time,,

		//slog("RowCount and Column count are: %i and %i respectively",rowCount, columnCount);
		if (world->tileMap) free(world->tileMap); //just in case it already has a tilemap, highkey
		world->tileMap = gfc_allocate_array(sizeof(Uint8), rowCount * columnCount);
		if (!world->tileMap) {
			slog("Failed to allocate tileMap for world %s",filename);
			sj_free(json);
			world_free(world);
			return NULL;
		}

		for (j = 0; j < rowCount; j++) {
			row = sj_array_get_nth(rows, j);
			if (!row) continue;
//We have column count already by the time we get here
			for (i = 0; i < columnCount; i++)	//J  vertically (rows).   i Horizontally (columns)
			{
				column = sj_array_get_nth(row, i);
				if (!column) continue;	//THE REASON I had world->tileMap[] as the 2nd parameter was 'cause that's where I needed to store it!!! silly. I just hadn't input the index, 'cause I didn't realize what we were talking abouttt [2d array, index it smartly, not just i]
				sj_get_uint8_value(column, &world->tileMap[j * columnCount + i]); //We need to give it the tileMap.  But which one ?.  Since we're saving the 2d array into a 1D array with gfc_allocate.. how do we determine stuff?
				//When j is 0: 0*my width  then add i (which is the # of columns). And it does this for each row, j. So when j = 1, we're in the next row. 1D ARRAY indexing baybee
			}
		}
	}
	slog("World '%s's tileMap initialized.", filename);
	string = sj_object_get_string(json, "name");
	if (string) gfc_line_cpy(world->name,string);

	string = sj_object_get_string(json, "background");
	if (string) {
		world->background = gf2d_sprite_load_image(string);
	}
	//determine systematically  which I think is that giant block above name
	string = sj_object_get_string(json, "tileSet");
	if (string) {
		sj_object_get_vector2d(json, "tileSetSize", &frameSize);
		sj_object_get_uint32(json, "tileSetFPL", &framesPerLine);
		slog("Loading world sprite...");
		world->tileSet = gf2d_sprite_load_all(
			string,
			(Uint32)frameSize.x,
			(Uint32)frameSize.y,
			framesPerLine,
			1);
	}
	
	sj_free(json);  //don't forget to clean the json object

	
	//Still need to handle the entity_list
	//GFC_List* entity_list;


	return world;
}


/*Uint8 world_get_tile_at(World* world, GFC_Vector2D position)
{
	if ((!world) || (world->tileMap)) return 0;
	return world->tileMap( (Uint32)position.y = (Uint32)world->tileMapSize.y + (Uint32)position.x);
}
*/

void world_draw(World* world) {
	//Uint8 tile;
	int i, j;		//I'm p sure \|/
	int index;
	int frame;
	GFC_Vector2D position = {0};
	
	if (!world) return;
	//also maybe the background ?
	gf2d_sprite_draw_image(world->background, gfc_vector2d(0, 0));

	if (!world->tileSet) return;
	for (j = 0; j < world->tileMapSize.y; j++) {  //'cause I'm pretty sure our Uint8 tileMap is the buffer that will contain each ROW.  wait wtf chat is this column first..?
		for (i = 0; i < world->tileMapSize.x; i++) {
			index = i + (j * world->tileMapSize.x); //To iterate through a 1D array as if it were 2D
			if ( world->tileMap[index] == 0) continue; //If the tile from our tileSet is == 0, that tile is off, so skip draw
			position.x = i * world->tileSet->frame_w;
			position.y = j * world->tileSet->frame_h; //So that we can spawn each tile in it's appropriate position
			frame = world->tileMap[index] - 1;
			gf2d_sprite_draw(
				world->tileSet,
				position,	//position
				NULL,		//scale
				NULL,		//center which is a 2D vector
				NULL,		//rotation
				NULL,		//flip
				NULL,		//colorShift
				frame);
		}
	}
	//however !  This way of drawing the tiles tends to be rather unoptimized, since we've got soo many draw calls happening
	//One way we can optimize it is by creating a tileLayer to draw everything _TO_ - using the gf2d_sprite_draw_to_surface() function


}


void world_free(World* world) {  //What you make you must destroy
	if (!world) return;

	if (world->background) {
		gf2d_sprite_free(world->background);
		if (!world->background) slog("World Background freed");
	}
	gf2d_sprite_free(world->tileSet);
	if (!world->tileSet) slog("World Tile Set freed");
	//missed smth here  I'm assuming what I missed here was just tileMapSize
	if (world->tileMap) { free(world->tileMap); if (!world->tileMap) slog("World tileMap freed"); }
	
	//I have to free the MapSize.   and now I am 99% sure that freeing a pointer to a GFC_Vector is _just_ achieved by free()ing it,
	//if (world->tileMapSize) { free(world->tileMapSize); if (!world->tileMapSize) slog("World tileMapSize freed"); }
		//nvm  I had this back when I still mistakenly had MapSize declared as a pointer. Just,, ignore this for now

	//clean up the entity list.  But if we remove the Player from the World's list,  then we don't have to worry about them being cleaned up
	/*int i, c;
	Entity* ent;
	if (world->entity_list) {
		c = gfc_list_count(world->entity_list);
		for (i = 0; i < c; i++) {
			ent = gfc_list_nth(world->entity_list, i);
			if (!ent) continue;
			entity_free(ent);
		}
		gfc_list_delete(world->entity_list);
	}*/

	free(world); //now that pointer is dead
	slog("World freed successfully");
}

World* world_new() {
	World* world;
	world = gfc_allocate_array(sizeof(World), 1);
	if (!world) {
		slog("ERROR: failed to allocate world");
		return NULL;
	}
	
	/*Hard coding this to test to make sure it even works
	world->tileMap = gfc_allocate_array(sizeof(Uint8), height * width);
	world->tileMapSize.x = width;
	world->tileMapSize.y = height;
	*/

	return world;
}


