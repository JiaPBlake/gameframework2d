#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"

#include "entity.h" //which can also help me add an Entity to my entityList
#include "world.h"
#include "camera.h"
#include "player.h"
#include "spawn.h"

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
	//Creat the Surface OF the Sprite
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
	GFC_Vector2D default_pos;
	

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
			}
		}
	}
//slog("World '%s's tileMap initialized.", world->name);	//Jlog

	string = sj_object_get_string(json, "background");
	if (string) {
		world->background = gf2d_sprite_load_image(string);
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
	activeWorld = world; //set the active world to the one we just loaded
	//Still need to handle the entity_list	--OKAY!  I think !!  I think this works!!
	world->entity_list = gfc_list_new();
	entList = sj_object_get_value(json, "entity_List"); //This is now a list/array of N objects
	entCount = sj_array_get_count(entList);
//slog("The number of entities that belong to this World's list is: %i",entCount);	//Jlog
	
	for (i = 0; i < entCount; i++) {	//! Also what I want to do is I want to be able to use this list of entities to spawn them in.  So call spawn as well
		default_pos = gfc_vector2d(-1, -1);
		enty = sj_array_get_nth(entList, i); //enty is now the JSon object that contains all the info about the ent.   That being said !!  The only info my WORLD needs to know about it   is it's name
		//SO THAT:  I can call  spawn_entity("player", default_pos); :   meaning I want a name and its position OVERRIDE  if there is one..

		if (enty) {
			ent_name = sj_object_get_string(enty, "name");    //Get the name of the entity/JSon object
			sj_object_get_vector2d(enty, "position", &default_pos); //if the position argument does not exist, it doesn't change the values of default_pos at all
			//Spawn the entity using its name in the Spawn list & its position in the World def file


			//Spawn in the entity.  THIS adds them into the Entity Manager
			ent = spawn_entity(ent_name, default_pos);
			if (ent) {	
//slog("Spawning and appending entity %s to the list", ent->name); //Jlog
				gfc_list_append(world->entity_list, ent); //yeah,  I nearly appended just the names, but I suppose it's easier to appent the Entity object itself
			}
			/*else { slog("ohhhh we didn't spawn the entity... Name retrieved by the SJon object is: %s", ent_name); }*/
		}
		
		//else { slog("Enty index #%i not found. Not spawning", i); }
	}
	sj_free(json);  //don't forget to clean the json object

	world_tile_layer_build(world); //J ADDED -  create the tileLayer so that it can be drawn

	
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

Uint8 world_get_tile_at(World* world, GFC_Vector2D position) //position will actually, more often then not.. BE the tile index. So dw about converting screen-space
{
	if ((!world) || (!world->tileMap)) return 0;
	//I need to convert screen-space coords to my tileMap index.  Which means I need to INTEGER divideee   (int)(position.x / world->tileMapSize.x)  + (int)(position.y / world->tileMapSize.y)
	return world->tileMap[ ((Uint32)position.y * (Uint32)world->tileMapSize.x) + (Uint32)position.x  ]; //AHA position, but like. *Tile height. and then add x position
	
}


void world_draw(World* world) {
	GFC_Vector2D camera;
	GFC_Vector2D position = { 0 };

	camera = camera_get_offset();  //AHA   since out offset is a negative value,  this changes where the Top Left corner of the world is DRAWNNNN  SO IF I'm in the middle of the world, the world ITSELF will be shifted in the negative direction-  sir DJ you are so smart and good at your job
	position = gfc_vector2d(0, 0);
	gfc_vector2d_add(position, position, camera);

	if (!world) return;
	//also maybe the background ?
	gf2d_sprite_draw_image(world->background, position); //So, to incorporate my camera offset. Instead of a 0,0 vector using gfc_vector2d(0, 0)    I use position the variable

	
	if (!world->tileSet) return; //I can also get rid of this check here, since we check it in the creation of tileLayer
	//Now, instead of the for loop we have down there,  if we have a tileLayer, we can just
	gf2d_sprite_draw_image(world->tileLayer, position);


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
	GFC_Shape test;
	GFC_Vector2D camera;
	camera = camera_get_offset();
	rect.w = world->tileSet->frame_w; //actually, let's just determine the width and height of my rectangle in a Data driven manner
	rect.h = world->tileSet->frame_h;

	for (j = 0; j < world->tileMapSize.y; j++) {
		for (i = 0; i < world->tileMapSize.x; i++) {
			tileIndex = world_get_tile_at(world, gfc_vector2d(i, j));
			if (!tileIndex) continue;

			//make a rectangle based on the tile position
			rect.x = (i * rect.w) + camera.x;
			rect.y = (j * rect.h) + camera.y;
			gf2d_draw_rect(rect, GFC_COLOR_YELLOW);
		}
	}

	return; //if we went through the entire Loop and found nothing. Return nothing
}

GFC_Vector2D world_get_ground() {
	World* world = activeWorld;
	GFC_Vector2D ground = {0};
	if (!world) { slog("Active world is NULL"); return ground; }
	int i, j;
	Uint8 tileIndex;
	//*Do NOT include camerae offset here :salute:   This is supposed to return position in WORLD space,  not on my screen

	//iterate through the world's tiles
	for (j = 0; j < world->tileMapSize.y; j++) {
		for (i = 0; i < world->tileMapSize.x; i++) {
			tileIndex = world_get_tile_at(world, gfc_vector2d(i, j));
			if (!tileIndex) continue;
			//The moment I hit my first 1 -- because of the way my maps are just surfaces.
//slog("tileIndex issss %i,%i",i,j);
			ground.x = (i * world->tileSet->frame_w) /* + camera.x*/;
			ground.y = (j * world->tileSet->frame_h) /* + camera.y */ ;
			//return the vector of the ground
			return ground;
		}
	}
	return ground;
}

//so we can include world.h  IN entity.c  so that our Entity_update  can call this function to ask the world : we good ? ? Where the shape in
//wait nvm (Feb 26)  I think this is called in game..?			This one is so I don't collide with any of the tiles in my world
//(Feb 28 - myself, at home)  hehe nah I'm calling it in player.c  'cause I included world.h  and it works out this way
int world_test_shape(World* world, GFC_Shape shape) {
	if ( (!world) || (!world->tileSet) ) return 0;

	int i, j;
	Uint8 tileIndex;
	GFC_Rect rect = { 0 }; //w and h of EACH tile.  I think mine are 240 by 80
	GFC_Shape test;
//*Do NOT include camerae offset here :salute:   This is supposed to be based off positions in WORLD space,  since my Player's position is ALSO only ever in world space

	rect.w = world->tileSet->frame_w; //actually, let's just determine the width and height of my rectangle in a Data driven manner
	rect.h = world->tileSet->frame_h; //
	//gfc_rect_slog(rect);

	for (j = 0; j < world->tileMapSize.y; j++) { //iterate through every tile .
		for (i = 0; i < world->tileMapSize.x; i++) {
			tileIndex = world_get_tile_at(world, gfc_vector2d(i,j));
			
			if (!tileIndex) continue;	
//slog("tileIndex is %i,%i", i, j);
			//make a rectangle based on the tile position
			rect.x = (i * rect.w) /* + camera.x*/;
			rect.y = (j * rect.h) /* + camera.y*/;
			test = gfc_shape_from_rect(rect);
			if (gfc_shape_overlap(test, shape)) return 1; //thissss is how we do world bounds

		}
	}

	return 0; //if we went through the entire Loop and found nothing. Return nothing
}


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
	else { slog("New world LOADED: %s", world->name); }
	
	activeWorld = world;
	//and move the player's position to the target spawn point for the new world
	gfc_vector2d_copy(player->position, targetPlayerSpawn);
	//return world;
	
}


