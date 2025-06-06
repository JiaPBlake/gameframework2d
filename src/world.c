#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"
#include "gfc_audio.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"

#include "entity.h" //which can also help me add an Entity to my entityList
#include "world.h"
#include "camera.h"
#include "player.h"
#include "spawn.h"  //spawn already includes player,,, but I think that's fine

//ONLY used this to help me fix the bugs I had before.of which, the bug was that I forgot the == 0 for the continue statement in the draw .
/*World* world_test_new() {
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
	
	world_tile_layer_build(world);
	slog("Tile Layer created");
	return world;
}
*/


//Make the world a singleton,,

static World* activeWorld = NULL;
GFC_Sound* test_sound;

//=======  Mon April 21.  a  Tile Layer CLEAR function
Uint8 world_get_tile_index_by_pos(World* world, GFC_Vector2I position);

void world_free(World* world) {  //What you make you must destroy.				DON'T FORGET to implement the Entity to ignore (the Player)
	if (!world) return;
	int i, c;			//don't forget to implement the Entity to ignore
	Entity* ent;

	if (world->background) {
		gf2d_sprite_free(world->background);
		if (!world->background) slog("World Background freed");
	}

	if (world->tileLayer) gf2d_sprite_free(world->tileLayer);

	gf2d_sprite_free(world->tileSet);
	if (!world->tileSet) slog("World Tile Set freed");
	//missed smth here  I'm assuming what I missed here was just tileMapSize
	if (world->tileMap) { free(world->tileMap); if (!world->tileMap) slog("World tileMap freed"); }


	//clean up the entity list.  But if we remove the Player from the World's list,  then we don't have to worry about them being cleaned up
	if (world->entity_list) {

		c = gfc_list_count(world->entity_list);
		//slog("The number of entities in the World about to be freed is: %i",c);	//Jlog
		for (i = 0; i < c; i++) {
			ent = gfc_list_nth(world->entity_list, i);
			//if(ent->name) slog("ent grabbed: %s",ent->name); //Jlog
			if (!ent) continue;

			entity_free(ent);
		}
		gfc_list_delete(world->entity_list); //delete the GFC_List itself
	}

	free(world); //now that pointer is dead/empty
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
	world->tileMapSize.y = height;*/


	return world;
}



void world_set_tile(World* world, GFC_Vector2D point, Uint8 tile) {
	GFC_Vector2I location = { 0 };  //Integer space vector
	if ((!world) || (!world->tileMap) || (!world->tileMapSize.x) || (!world->tileMapSize.y)) return;
	//from World space (pixels) to tile space (tile index)
	location.x = (float)point.x / world->tileMapSize.x;
	location.y = (float)point.y / world->tileMapSize.y;

	world->tileMap[world_get_tile_index_by_pos(world, location)] = tile;
	//He makes a function to instead GET YOU the index   of the tile AT 'location':  world_get_tile_by_pos


	//For the Level editor:
	//*Mouse is in screen coordinates.  We want to put it into WORLD coords,  so we must now ADD our camera's offset

}


void world_tile_layer_clear(World* world) {
	if (!world) return;
	gf2d_sprite_free(world->tileLayer);
	world->tileLayer = NULL;

}

// ==========

void world_tile_layer_build(World* world) {
	if (!world) return;
	if (!world->tileSet) return;
	if (world->tileLayer) gf2d_sprite_free(world->tileLayer); //in case it already had one, clean it up

	int i, j;
	Uint32 index;
	GFC_Vector2D position = {0};
	Uint32 frame;
	//We're going to make a Surface for our Sprite. So we can convert it to a Texture to use (such that we can draw it all at once)
		// "We're going to be using gf2d_sprite-draw_to_surface()  to draw TO the tileLayer (the SDL_Surface, I think)  all of our tile-stuff (all our Tile sprite related stuff)
	
	world->tileLayer = gf2d_sprite_new();	//Create a Sprite
	if (!world->tileLayer) slog("We didn't even make the damn tileLayer..");

//slog("The x dimension for the create surface command is: %i", (int)world->tileMapSize.x * world->tileSet->frame_w);
	//Create the Surface OF the Sprite
	world->tileLayer->surface = gf2d_graphics_create_surface(
		((int)world->tileMapSize.x*world->tileSet->frame_w),
		((int)world->tileMapSize.y*world->tileSet->frame_h));

	if (!world->tileLayer->surface) {
		slog("Failed to create tileLayer surface");
		return;
	}
	world->tileLayer->frame_w = world->tileMapSize.x * world->tileSet->frame_w; //set the width and height of my Layer, which would be the whole Map size
	world->tileLayer->frame_h = world->tileMapSize.y * world->tileSet->frame_h;

	for (j = 0; j < world->tileMapSize.y; j++) {  
		for (i = 0; i < world->tileMapSize.x; i++) {
			index = i + (j * world->tileMapSize.x);  //To iterate through a 1D array as if it were 2D
			if (world->tileMap[index] == 0) continue; //If the tile from our tileSet is == 0, that tile is off, so skip draw
			//This ^^ line works fine

			position.x = i * world->tileSet->frame_w;
			position.y = j * world->tileSet->frame_h;//So that we can spawn each tile in it's appropriate position

			frame = world->tileMap[index] - 1;
			frame = frame % 4;

			gf2d_sprite_draw_to_surface(
				world->tileSet,
				position,
				NULL,
				NULL,
				frame,
				world->tileLayer->surface);
		}
	}

	world->tileLayer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), world->tileLayer->surface);
	if (!world->tileLayer->texture) {
		slog("Failed to convert World tileLayer to a texture");
		return;
	}
	//slog("Layer created");  //Jlog
}


//===============   Wednesday April 16  I actually dk what topic this is  he started off saying parallax but somebody OHHH OH  I think it's how to save a world a player makes through the Content Editor
	//purely for proof of concept.  I'm probably never gonna use this	@brief save a world to a file
void world_save(World* world, const char* filename) {
	SJson* json;
	if ((!filename) || (!world)) return;
	json = sj_object_new();		//create a new JSon object
	if (!json) { slog("failed ot make new json for world saving"); return; }

	//Then fill out all the parameters that would NEED TO GO in a Def file for a Level/World

	sj_object_insert(json, "name", sj_new_str(world->name) );  //Make sure it aligns with your convention

	//*You could work to make a funciton that   Takes a Sprite,  and effectively converts it into all the info you'd need to be in a JSON file to create the sprite
	if (world->background) {
		sj_object_insert(json, "backgroundFile:", sj_new_str(world->background->filepath));  //where filepath is the data member of the Sprite structure 
		sj_object_insert(json, "backgroundFrameW", sj_new_uint32(world->background->frame_w));
		sj_object_insert(json, "backgroundFrameH", sj_new_uint32(world->background->frame_h));
	}

	//How to add another OBJECT to a json?





	sj_save(json, filename);
	sj_free(json);  //once it's saved as a def file.   Free the JSon object.  I don't need the json object


}

//=========

World* world_load(const char* filename) {
	GFC_Vector2D frameSize = { 0 };
	Uint32 framesPerLine = 0;
	const char* string = NULL;
	SJson* json;
	World* world;

	int rowCount, columnCount, entCount;
	int i, j;
	SJson* column, *rows, *row;

	int p = 0;
	SJson* entList, *enty;
	const char* ent_name = NULL;
	Entity* ent = NULL;
	Entity* player = NULL;
	GFC_Vector2D temp;
	GFC_Vector3D default_pos = { 0 };
	

	int spawnTileValue, tileValueForTesting;
	GFC_Vector3D *spawnTilePosition = { 0 };		//be sure to try and make this  Horizontal Center.  But bottom of entity sprite
	GFC_List *spawn_coords = gfc_list_new();
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

	string = sj_object_get_string(json, "name");
	if (string) {
		gfc_line_cpy(world->name, string);
//slog("World name is: %s", world->name);  //Jlog
	}

	string = sj_object_get_string(json, "background");
	if (string) {
		world->background = gf2d_sprite_load_image(string);
		//slog("The size of the background sprite is: %i",world->background->frame_w);  //Jlog
		//slog("World %s's BACKGROUND sprite loaded.", world->name);	//Jlog
	}
	string = sj_object_get_string(json, "midground");
	if (string) {
		world->midground = gf2d_sprite_load_image(string);
		//slog("The size of the background sprite is: %i",world->background->frame_w);  //Jlog
		//slog("World %s's BACKGROUND sprite loaded.", world->name);	//Jlog
	}
	string = sj_object_get_string(json, "foreground");
	if (string) {
		world->foreground = gf2d_sprite_load_image(string);
		//slog("The size of the background sprite is: %i",world->background->frame_w);  //Jlog
		//slog("World %s's BACKGROUND sprite loaded.", world->name);	//Jlog
	}

	//determine systematically  which I think is that giant block above name
	string = sj_object_get_string(json, "tileSet");
	if (string) {
		sj_object_get_vector2d(json, "tileSetSize", &frameSize);
		sj_object_get_uint32(json, "tileSetFPL", &framesPerLine);
		//slog("Loading world sprite...");
		world->tileSet = gf2d_sprite_load_all(
			string,
			(Uint32)frameSize.x,
			(Uint32)frameSize.y,
			framesPerLine,
			1);
		//slog("World %s's TileSET Sprite loaded.", world->name); //Jlog
	}

	rows = sj_object_get_value(json, "tileMap"); //rows is the 2D array == tileMap
//slog("Rows array obtained successfully");  //Jlog
	rowCount = sj_array_get_count(rows);
	if (rowCount) {
		
		row = sj_array_get_nth(rows, 0);
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
//slog("RowCount and Column count are: %i and %i respectively",rowCount, columnCount);

		if (world->tileMap) free(world->tileMap); //just in case it already has a tilemap, highkey
		world->tileMap = gfc_allocate_array(sizeof(Uint8), rowCount * columnCount);	//Allocate the array for the tiles
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
			for (i = 0; i < columnCount; i++)	// j vertically (the rows).   i Horizontally (columns)
			{
				column = sj_array_get_nth(row, i);
				if (!column) continue;	//THE REASON I had world->tileMap[] as the 2nd parameter was 'cause that's where I needed to store it!!! silly. I just hadn't input the index, 'cause I didn't realize what we were talking abouttt [2d array, index it smartly, not just i]
				sj_get_uint8_value(column, &world->tileMap[j * columnCount + i]); //We need to give it the tileMap.  But which one ?.  Since we're saving the 2d array into a 1D array with gfc_allocate.. how do we determine stuff?
				//When j is 0: 0*my width  then add i (which is the # of columns). And it does this for each row, j. So when j = 1, we're in the next row. 1D ARRAY indexing baybee
				spawnTileValue = world->tileMap[j * columnCount + i];
				
				// *It might also be worth it to consider changing this 4 TO my value of "framesPerLine"  incase I ever make for than 4 frames for a tileSet
				if (spawnTileValue > 4) {
					spawnTilePosition = gfc_vector3d_new();		//AHA!!  THIS is how I create a new vecor,  instead of constantly re-writing One.
					
					//slog("The tile at i: %i; j: %i  is the number %i", i, j, spawnTileValue);
					spawnTilePosition->x = i * world->tileSet->frame_w;  //corner of the tile
					spawnTilePosition->y = j * world->tileSet->frame_h;
					spawnTilePosition->z = spawnTileValue;
					gfc_list_append(spawn_coords, spawnTilePosition);
					//slog("Address of spawnTilePosition is %p", spawnTilePosition);   //Jlog
				}
			}
		}
	}
	//slog("after configuring the map. The number of SpawnPoints in my list is: %i, %p, x: %f", spawn_coords->count, spawn_coords[0], gfc_list_get_nth(0).x);
	/*if (spawn_coords->count > 1) {		//J test.  Works! I can add more than 1 distinct vector now, thanks to gfc_vector3d_new()
		GFC_Vector3D *test1, *test2;
		test1 = gfc_list_get_nth(spawn_coords, 0);
		test2 = gfc_list_get_nth(spawn_coords, 1);
		slog("First spawn point is at x:%f  y:%f\n", test1->x, test1->y);
		slog("Second spawn point is at x:%f  y:%f\n", test2->x, test2->y);
	}*/

//slog("World '%s's tileMap initialized.", world->name);	//Jlog

	activeWorld = world; //set the active world to the one we just loaded
	//Still need to handle the entity_list	--OKAY!  I think !!  I think this works!!
	world->entity_list = gfc_list_new();
	entList = sj_object_get_value(json, "entity_List"); //This is now a list/array of N objects
	entCount = sj_array_get_count(entList);
//slog("The number of entities that belong to this World's list is: %i",entCount);	//Jlog
	
	for (i = 0; i < entCount; i++) {	//! Also what I want to do is I want to be able to use this list of entities to spawn them in.  So call spawn as well
		enty = sj_array_get_nth(entList, i); //enty is now the JSon object that contains all the info about the ent.   That being said !!  The only info my WORLD needs to know about it   is it's name
		//SO THAT:  I can call  spawn_entity("player", default_pos); :   meaning I want a name and its position OVERRIDE  if there is one..

		if (enty) {
			ent_name = sj_object_get_string(enty, "name");    //Get the name of the entity/JSon object
			sj_object_get_vector2d(enty, "position", &temp); //if the position argument does not exist, it doesn't change the values of default_pos at all
			default_pos.x = temp.x;
			default_pos.y = temp.y;
			default_pos.z = 0;
			//Spawn the entity using its name in the Spawn list & its position in the World def file
			
			//hmmmm  maybe I SHOULD make a list.  bit a List of 3D vectors.  so that I can also save the value AT that tile.  So I can make
				//5 = Dragon spawn points.   6  = item spawn points.    7 =  Cave spawn points.  8 = NPCs. Saving those values as the 3d vector's .z value
			
			//if (spawnTilePosition && spawnTilePosition->x > 0 && spawnTilePosition->z != 5) {
			//	default_pos.x = spawnTilePosition->x;
			//	default_pos.y = spawnTilePosition->y;
			//	default_pos.z = 1;
				//Jlog   is default_pos is being overriden by any spawn tile Value
				//slog("Spawning entity at the position indicated BY the Map def file's tileSet, x position: %f",spawnTilePosition->x);
			//}
			//Spawn in the entity.  THIS adds them into the Entity Manager
			ent = spawn_entity(ent_name, default_pos, spawn_coords);
			if (ent) {	
//slog("Spawning and appending entity %s to the list. With position x at: %f", ent->name,default_pos.x); //Jlog
				gfc_list_append(world->entity_list, ent); //yeah,  I nearly appended just the names, but I suppose it's easier to appent the Entity object itself
			}
			/*else { slog("ohhhh we didn't spawn the entity... Name retrieved by the SJon object is: %s", ent_name); }*/
		}
		
		//else { slog("Enty index #%i not found. Not spawning", i); }
	}
	sj_free(json);  //don't forget to clean the json object

	world_tile_layer_build(world); //J ADDED -  create the tileLayer so that it can be drawn


	//Once we've spawned in all the entities in their proper place.  Delete the List of Spawn_coords:
	gfc_list_delete(spawn_coords);
	
	camera_set_bounds(gfc_rect(0,0,frameSize.x*world->tileMapSize.x, frameSize.y*world->tileMapSize.y)); //set the bounds of our camera to be the bounds of the current world
	camera_set_position(gfc_vector2d(0,0));
	return world;
}

World* world_get_active() {
	return activeWorld;
}
void world_set_active(World* worldToSet) {
	activeWorld = worldToSet;
}

//=== Mon April 21
//Get the index number of the tile based on its  i, j position.  (1D array index is returned from a 2D representation)
Uint8 world_get_tile_index_by_pos(World* world, GFC_Vector2I position)
{
	if ((!world) || (!world->tileMap)) return 0;
	//I need to convert screen-space coords to my tileMap index.
	return ( ((Uint32)position.y * (Uint32)world->tileMapSize.x) + (Uint32)position.x ); //AHA position, but like. *Tile height. and then add x position

}

//=====

//Returns the the value of the tile that is AT the index provided.  (This essentially just traverses through the 1D array in a 2D manner:   C*i + j
Uint8 world_get_tile_value(World* world, GFC_Vector2D index) //position will actually, more often then not.. BE the tile index. So dw about converting screen-space
{
	if ((!world) || (!world->tileMap)) return 0;
	//I need to convert screen-space coords to my tileMap index.  Which means I need to INTEGER divideee   (int)(position.x / world->tileMapSize.x)  + (int)(position.y / world->tileMapSize.y)
	return world->tileMap[ ((Uint32)index.y * (Uint32)world->tileMapSize.x) + (Uint32)index.x  ]; //AHA position, but like. *Tile height. and then add x position
	
}





void world_draw(World* world) {
	GFC_Vector2D camera;
	GFC_Vector2D position = { 0 };

	camera = camera_get_offset();  //AHA   since out offset is a negative value,  this changes where the Top Left corner of the world is DRAWNNNN  SO IF I'm in the middle of the world, the world ITSELF will be shifted in the negative direction-  sir DJ you are so smart and good at your job
	position = gfc_vector2d(0, 0);
	gfc_vector2d_add(position, position, camera);

	if (!world) return;
	//also maybe the background ?
	gf2d_sprite_draw_image(world->background, gfc_vector2d((position.x / 5), position.y)); //So, to incorporate my camera offset. Instead of a 0,0 vector using gfc_vector2d(0, 0)    I use position the variable

	if (world->midground) {
		gf2d_sprite_draw_image(world->midground, gfc_vector2d( (position.x / 2.5), position.y ));		//for some reason,,, the png is not png'ing..
	}

	if (!world->tileSet) return; //I can also get rid of this check here, since we check it in the creation of tileLayer
	//Now, instead of the for loop we have down there,  if we have a tileLayer, we can just
	gf2d_sprite_draw_image(world->tileLayer, position);
	


	//Jnote:   gotta draw this on top of all my entities

	if (world->foreground) {
		gf2d_sprite_draw_image(world->foreground, gfc_vector2d((position.x / 0.7), position.y));
	}

	/*
	int i, j;
	int index;
	int frame;
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
	}*/
}

//-----------------------------------
//BOUNDS AND COLLISION

//Wed Feb 26    Synced  This one is LITERALLY for the bounds of the world.  Like so I don't go out of my world
int world_bounds_test(World *world, GFC_Shape shape) {
	
	GFC_Rect boundingRect;
	
	if ((!world) || (!world->tileSet)) return 0;

	boundingRect = gfc_shape_get_bounds(shape); //will do the math to give me the Bounding rectangle for that given shape/object

	if (boundingRect.x < 0 || boundingRect.y < 0) return 1;
	if (boundingRect.x + boundingRect.w > world->tileSet->frame_w * world->tileMapSize.x || boundingRect.y < world->tileSet->frame_h * world->tileMapSize.y) return 1;
	return 0;
}

//Web feb 27   Works
void world_draw_bounds(World* world) {

	if (!world) return;

	int i, j;
	Uint8 tileIndex;
	GFC_Rect rect = { 0 }; //w and h of EACH tile.  I think mine are 240 by 80  his are 64 by 64
	GFC_Vector2D camera;
	camera = camera_get_offset();
	rect.w = world->tileSet->frame_w; //actually, let's just determine the width and height of my rectangle in a Data driven manner
	rect.h = world->tileSet->frame_h;

	for (j = 0; j < world->tileMapSize.y; j++) {
		for (i = 0; i < world->tileMapSize.x; i++) {
			tileIndex = world_get_tile_value(world, gfc_vector2d(i, j));
			if (!tileIndex) continue;

			//make a rectangle based on the tile position
			rect.x = (i * rect.w) + camera.x;
			rect.y = (j * rect.h) + camera.y;
			gf2d_draw_rect(rect, GFC_COLOR_YELLOW);
		}
	}

	return; //if we went through the entire Loop and found nothing. Return nothing
}

//I'm still going to use this function as a means of finding the absolute lowest point in the map. So that I never go past that
GFC_Vector2D world_get_ground() {
	World* world = activeWorld;
	GFC_Vector2D ground = {0};
	if (!world) { slog("Active world is NULL"); return ground; }
	int i, j;
	Uint8 tileIndex;
	//*Do NOT include camerae offset here :salute:   This is supposed to return position in WORLD space,  not on my screen

	//iterate through the world's tiles  BACKWARDS
	for (j = world->tileMapSize.y - 1; j >=0 ; j--) {   //j is the row
		for (i = world->tileMapSize.x  -1; i >=0; i--) {  //i is the column..   from Left--> Right  (x)
			tileIndex = world_get_tile_value(world, gfc_vector2d(i, j));
			if (tileIndex) continue;

			//The moment I hit my FIRST 1 -- because of the way my maps are just surfaces.

			// Ignore that ^  I'm doing this differently.  The moment I hit my first 0. That level UNDERNEATH it is my absolute ground. (i-1)
//slog("tileIndex issss %i,%i",i,j);
			//ground.x = (i * world->tileSet->frame_w) /* + camera.x*/;   //mmmmmm yeah bc I look for the FIRST 1 in the map.. this x is always the same.. gotta change this
			ground.y = ( (j+1) * world->tileSet->frame_h) /* + camera.y */ ;
			//return the vector of the ground
			return ground;
		}
	}
	return ground;
}


//in the works..  I want to get the Ground level (y level coordinate) of the tile  that is right underneath me (any given entity).   Most likely, my bounds will actually be colliding within that tile.
Uint8 world_get_ground_relative(World* world, GFC_Vector2I position) {
	if ((!world) || (!world->tileMap)) return 0;

	//dawg I am so god damn scatterbrained I have no idea what I'm doing..

	Uint8 tileIndex;
	//I need to convert screen-space coords to my tileMap index.
	tileIndex = world_get_tile_index_by_pos(world, position);

}


//Do NOT include camera offset here :salute:   This is supposed to be based off positions in WORLD space,  since my Player's position is ALSO only ever in world space
/*GFC_List* world_get_collided(World* world, GFC_Shape shape) {
	//See .   half of my problem is the fact that this only returns the FIRST time I'm colliding with... from the top of the map down. 
	//I suppose I..  COULD?? return a list.  But i still have to fix the underlying problem of handling x and y collision at appropriate times..
		//Which !! I think I;m really on to something with the distInGround thing..   honestly !? That has ALREADY solved my problem !! That's the way to go.
		//Now I just need to make sure that:  me running face first into a wall doesn't cause me to IGNORE the ground beneath me LOL
	
	if ((!world) || (!world->tileSet)) return NULL; //for the List implementation
	//fuck it.
	GFC_List* tiles;				//For list implementation
	tiles = gfc_list_new();
	//Let's make a list.  and start appending

	int i, j;
	Uint8 tileValue;
	GFC_Rect rect = { 0 }; //w and h of EACH tile.  I think mine are 240 by 80
	GFC_Shape test_shape;
	GFC_Rect* tile;

	rect.w = world->tileSet->frame_w; //actually, let's just determine the width and height of my rectangle in a Data driven manner
	rect.h = world->tileSet->frame_h; //
	//gfc_rect_slog(rect);

//iterate through every tile .
	for (j = 0; j < world->tileMapSize.y; j++) {		
		for (i = 0; i < world->tileMapSize.x; i++) {
			tileValue = world_get_tile_value(world, gfc_vector2d(i, j));

			if (!tileValue) continue;	  //if the tile here isn't ON (not 1)
			//slog("tileValue is %i,%i", i, j);

		//make a rectangle based on the tile position
			rect.x = (i * rect.w);
			rect.y = (j * rect.h);
			test_shape = gfc_shape_from_rect(rect);
			//create the Shape from the tile.

		//IF and ONLY IF I am colliding.  Get the direction and return it
			if (gfc_shape_overlap(test_shape, shape)) {  //IF I am colliding with this given rectangle:
				tile = &rect;
				gfc_list_append(tiles, tile);  //For list implementation
				slog("Tile appended");
				slog("Rectangle's X is: %f",rect.x);
				slog("WHAT THE FUCK:  %f",tile->x);
			}
		}
	}


	//For list implementation
	if (!gfc_list_count(tiles)) { //if, by the end of my for loop, I never collided with anything:
		gfc_list_delete(tiles);
		return NULL;
	}
	//slog("Checkpoint 3");

	return tiles;
}*/


//fuck this function and fuck GFC_Lists   there's a disconnect between what I'm SUPPOSEDLY appending .  and what I'm retrieving.

/*void tile_collide_check(GFC_Rect* rect, GFC_Shape shape, GFC_Vector4D* direction) {

	GFC_Rect tile = { 0 };
	gfc_rect_copy(tile, (*rect));
	//These are both rectangles.  and they SHOULD be overlapping.. but in the event they're not for some reason 'cause the list fucked up,  check and exit prematurely when needed
	GFC_Shape test_shape;
	test_shape = gfc_shape_from_rect(tile);
	if (gfc_shape_overlap(test_shape, shape)) { slog("Bro we aren't even colliding. What's going on"); return; }

	float distInGround = 0;
	//Back when I returned one tile  (tile)  at a time.
	distInGround = (shape.s.r.y + shape.s.r.h) - tile.y;
	//slog("I am %f pixels into the ground", distInGround);  //At my current speed, this number doesn't seem to go over 3.0

	//idea.   IF I'm colliding.  Get the direction I collided FROM.
		//Player collides on the Left of a Tile.   Conidering I'm already overlapping:  my Right edge JUST barely became > tile's Left edge
	if ((shape.s.r.x + shape.s.r.w) > (tile.x)) {
		slog(" (R) PLAYER RIGHT EDGE collision ON tile's Left");
		direction->x = 1;
	}
	else if ((shape.s.r.x) < (tile.x + tile.w)) {
		slog(" (L) PLAYER LEFT EDGE collision ON tile's Right");
		direction->x = -1;
	}

	slog("Checkoint 1");
	// IF  !  AND ONLY IF  I am moving Left or Right .  my Y should not take place
	//if (direction->z == 0) {
	if (distInGround < 4.0) {
		slog("Checkpoint 1.5");
		if (((shape.s.r.y + shape.s.r.h) > tile.y) && (shape.s.r.y < tile.y)) {
			slog(" G R O U N D -- PLAYER BOTTOM EDGE collision ON tile's Top");
			direction->y = 1;
			direction->z = 0;  //This needs to be here
		}
		else if (shape.s.r.y < (tile.y + tile.h)) {
			slog("PLAYER TOP EDGE collision ON tile's Bottom");
			direction->y = -1;
		}
	}
	return;
}*/




//so we can include world.h  IN entity.c  so that our Entity_update  can call this function to ask the world : we good ? ? Where the shape in
//wait nvm (Feb 26)  I think this is called in game..?			This one is so I don't collide with any of the tiles in my world
//(Feb 28 - myself, at home)  hehe nah I'm calling it in player.c  'cause I included world.h  and it works out this way
GFC_Rect world_test_shape(World* world, GFC_Shape shape, GFC_Vector4D* direction) {
	int i, j;
	Uint8 tileValue;
	GFC_Rect rect = { 0 }; //w and h of EACH tile.  I think mine are 240 by 80
	GFC_Shape test_shape;
	float distInGround = 0;
	//GFC_Vector4D direction = { 0 };
	//if ((!world) || (!world->tileSet)) return rect;


//*Do NOT include camera offset here :salute:   This is supposed to be based off positions in WORLD space,  since my Player's position is ALSO only ever in world space

	rect.w = world->tileSet->frame_w; //actually, let's just determine the width and height of my rectangle in a Data driven manner
	rect.h = world->tileSet->frame_h; 
	//gfc_rect_slog(rect);
	for (j = 0; j < world->tileMapSize.y; j++) {		//iterate through every tile .
		for (i = 0; i < world->tileMapSize.x; i++) {
			tileValue = world_get_tile_value(world, gfc_vector2d(i, j));

			if (!tileValue) continue;	  //if the tile here isn't ON (not 1)
		//make a rectangle based on the tile position
			rect.x = (i * rect.w) /* + camera.x*/;
			rect.y = (j * rect.h) /* + camera.y*/;
			test_shape = gfc_shape_from_rect(rect);
			//create the Shape from the tile.

		//IF and ONLY IF I am colliding.  Get the direction and return it
			if (gfc_shape_overlap(test_shape, shape)) {  //IF I am colliding with this given rectangle:

						//Back when I returned one rect  (tile)  at a time.
				distInGround =  (shape.s.r.y + shape.s.r.h) - rect.y;
				slog("I am %f pixels into the ground", distInGround);  //At my current speed, this number doesn't seem to go over 3.0

				//idea.   IF I'm colliding.  Get the direction I collided FROM.
					//Player collides on the Left of a Tile.   Conidering I'm already overlapping:  my Right edge JUST barely became > tile's Left edge
				if ( (shape.s.r.x + shape.s.r.w) > ( rect.x ) ) {
					slog(" (R) PLAYER RIGHT EDGE collision ON tile's Left");
					direction->x = 1;
				}
				else if ( (shape.s.r.x) < (rect.x + rect.w)) {
					slog(" (L) PLAYER LEFT EDGE collision ON tile's Right");
					direction->x = -1;
				}

				// IF  !  AND ONLY IF  I am moving Left or Right .  my Y should not take place
				//if (direction->z == 0) {
				if (distInGround < 4.0) {
					if ( ((shape.s.r.y + shape.s.r.h) > rect.y) && (shape.s.r.y < rect.y) ) {
						slog(" G R O U N D -- PLAYER BOTTOM EDGE collision ON tile's Top");
						direction->y = 1;
						direction->z = 0;  //This needs to be here
					}
					else if ( shape.s.r.y < (rect.y + rect.h) ) {
						slog("PLAYER TOP EDGE collision ON tile's Bottom");
						direction->y = -1;
					}
				}

				//This is when I returned the Tile through parameters
				/*if (tile->x) slog("Tile's x is: %f", tile->x);
				gfc_rect_copy( (*tile), rect);
				slog("Tile's x is NOW: %f", tile->x);*/


				return rect; //remember.  the tileValue (used to be called 'tileIndex') is JUST the value of the tile...
			}
		}
	}

	//This worked before.   Before when I returned the TILE I collided with.  However, I need to factor in more than just 1 tile.. can't return as soon as I reach the first one, 'cause then that messes up ground collision
	rect.x = 0;
	rect.y = 0;
	return rect; //if we went through the entire Loop and found nothing. Return nothing 
}

//Just move the damn player yourself bro.  For every tile
void world_collision_handle(Entity* player, GFC_Shape shape, GFC_Vector4D* dir) {
	if (!player) return;
	

	//holy fucking shit there's an edge case where:   I can overlap while being  ZERO pixels into the GRound.... and thus I will also be moved LEFT INSTEAD OF  up.


	World* world = world_get_active();
	if ((!world) || (!world->tileSet)) return;
	int i, j, index;
	Uint8 tileValue;
	GFC_Rect rect = { 0 };
	GFC_Shape test_shape;
	float distInGround = 0;
	float distInCeiling = 0;
	GFC_Vector4D direction = (*dir);

	GFC_Vector2I pos = {0};

	rect.w = world->tileSet->frame_w;
	rect.h = world->tileSet->frame_h;

	for (j = 0; j < world->tileMapSize.y; j++) {		//iterate through every tile .
		for (i = 0; i < world->tileMapSize.x; i++) {
			tileValue = world_get_tile_value(world, gfc_vector2d(i, j));

			if (!tileValue) continue;	  //if the tile here isn't ON (not 1)
			//make a rectangle based on the tile position
			rect.x = (i * rect.w);
			rect.y = (j * rect.h);
			test_shape = gfc_shape_from_rect(rect);
			//create the Shape from the tile.

		//IF and ONLY IF I am colliding.  Use the direction to determine how the player should move
			if (gfc_shape_overlap(test_shape, shape)) {  //IF I am colliding with this given rectangle:

				//Back when I returned one rect  (tile)  at a time.
				distInGround = (shape.s.r.y + shape.s.r.h) - rect.y;
				//slog("I am %f pixels into the ground", distInGround);  //At my current speed, this number doesn't seem to go over 3.0
				distInCeiling = (rect.y + rect.h) - shape.s.r.y;  //tile's bottom  minus my top

				//idea.   IF I'm colliding.  Get the direction I collided FROM.
			// AHA!  and now that I'm just giving the Player as a paramter. I can directly exract the direction of their velocity
				//slog("Player's angle of velocity is: %f", gfc_vector2d_angle(player->velocity));
				pos.x = i;
				pos.y = j;
				index = world_get_tile_index_by_pos(world, pos);
					//Player collides on the Left of a Tile.   Conidering I'm already overlapping:  my Right edge JUST barely became > tile's Left edge
				if ( ((shape.s.r.x + shape.s.r.w) > rect.x) && (shape.s.r.x < rect.x) ) {
					//slog(" (R) PLAYER RIGHT EDGE collision ON tile's Left.  On Tile index: %i", index);
					direction.x = 1;
				}
				else if ( (shape.s.r.x < (rect.x + rect.w)) && ((shape.s.r.x + shape.s.r.w) > (rect.x + rect.w)) ) {
					//slog(" (L) PLAYER LEFT EDGE collision ON tile's Right.  On Tile index: %i", index);
					direction.x = -1;
				}


		// ***Sometimes when I jump.  This disInGround if never occurs.  So it doesn't realize that I'm on the ground  and instead "You have been stopped."s me, pushing me left
			//NO!! NO  the dist if DOES happen.   neither of the inner Conditionals do.  Because I am NOT overlapping my edge. I am == to it. (distInGround == 0 !!)

				// IF  !  AND ONLY IF  I am moving Left or Right .  my Y should not take place  nvm. This is like 2/3 casses.
				if (distInGround < 12.0 ) {
					direction.z = 0;  //This needs to be here
					if (((shape.s.r.y + shape.s.r.h) > rect.y) && (shape.s.r.y < rect.y)) {
						//slog(" G R O U N D -- PLAYER BOTTOM EDGE collision ON tile's Top.  On Tile index: %i", index);
						direction.y = 1;
					}
					//else { slog("wait this is awkward... I'm very much so a large distance into the ground and yet..."); }
					
				}
				//if dist  is  GREATER  that most likely means I want to do  L / R Collission first  sooo!!! 
				else {
					if (distInCeiling < 12.0 && (shape.s.r.y < (rect.y + rect.h)) && ((shape.s.r.y + shape.s.r.h) > (rect.y + rect.h))) {
						slog("PLAYER TOP EDGE collision ON tile's Bottom.  On Tile index: %i", index);
						direction.y = -1;
						direction.z = 0;  //This needs to be here 
					}
					
					//else slog("Too far into ground on Tile Index: %i (suggests Side).  Speed responsible is:   X %f,  Y%f", index, player->velocity.x, player->velocity.y);
				}

//	PLAYER MOVEMENT ====================================================
				if (direction.y != 0 || direction.x != 0) { //my y position (taking into account my bounds) should be 614 compared to ground's 640 
					//slog("Checkpoint 2");
					//I have to handle collision here .   tile_collide_check tells me IF I am colliding.  AND the rect I'm colliding with. but only one at a time,,,

					//slog("Player's X position is: %f", player->position.x);

					//I return BOTH x & y dimensions for direction.  I choose whether I want to act on them, tho.

							//dir.z let's me know that I'm moving Right on the ground.
					if (direction.z != 0) {   //if I'm colliding right 
						if (direction.x > 0) {
							player->position.x = rect.x + player->bounds.x;
							direction.y = 0;
							//slog("You have been stopped .");
							//direction.z = player->velocity.x;
						}

						else if (direction.x < 0) {
							player->position.x = rect.x + rect.w - player->bounds.x;
							direction.y = 0;
							//slog("You have been stopped LEFT EDITION!!!");
							//direction.z = player->velocity.x;
						}
					}

					if (direction.z == 0) {  //if I'm colliding top.  and top ONLY.  Don't let me go into the ground.
						if (direction.y > 0) {
							//We turned z off.  Because we don't want GROUND telporting me Left.
							player->position.y = rect.y + player->bounds.y;
							dir->w = 1;
							//slog("You have been raised");
						}
						else if (direction.y < 0) {
							//We turned z off.  Because we don't want GROUND telporting me Left.
							player->position.y = rect.y + rect.h - player->bounds.y;
							dir->w = -1;
							slog("You have been lowered");
						}
					}

					//slog("Colliding");

				}




			//end of for loops
			}
		}
	}
	return;
}

//

//---------------------------------
//ENTITY LIST SECTION

void world_add_entity(GFC_List* list, Entity* ent) {
	if ((!list) || (!ent)) {
		slog("No list or no Entity provided. Not appending to World '%s's entList",activeWorld->name);
		return;
	}
	gfc_list_append(list, ent);
}

//Removes an entity from our entity_list
void world_remove_entity(GFC_List* list, Entity* ent) {
	if ((!list) || (!ent)) {
		slog("No list or no Entity provided. Cannot remove entity %s", ent->name);
		return;
	}
	gfc_list_delete_data(list, ent); //This function removes a single object from the list, not deleting hte list itself
}


void world_transition(World* old, const char* newWorld, GFC_Vector2D targetPlayerSpawn) {  //here we go
	Entity* player;
	World* world = NULL;
	player = player_get_the();
	if (!newWorld) {
		slog("No newWorld name provided");
		return;
	}

	if (old) {
		//slog("Checking if player");
		if (player) {
			//slog("Removing player");
			world_remove_entity(old->entity_list, player);  //remove the Player from the Old world's Ent List so that the player is not freed
		}
		//slog("Freeing world now");
		world_free(old);
	}

	slog("Loading new world");
	world = world_load(newWorld);  //AND  so that when we laod up the new world, it will just return the current active Player and add it to ITS list
	if (!world) {   //uh oh
		slog("Uh oh. newWorld failed to load");
		return;
	}
	else { slog("New world LOADED: %s.  TILESET sprite size: %i", world->name, world->tileSet->frame_w); }
	
	activeWorld = world;
	//and move the player's position to the target spawn point for the new world
	gfc_vector2d_copy(player->position, targetPlayerSpawn);
	//return world;
	
}


