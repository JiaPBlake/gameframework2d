#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_list.h"
#include "gf2d_sprite.h"

typedef struct {
	GFC_TextLine	name;
	Sprite			*background;
	//Sprite			*tileLayer; /**<prerendered tile layer*/
	Sprite			*tileSet;	/**<Where tileSet, in my def file for the given World, would be the name of the image file for the Sprite !(grid of tiles)*/
	Uint8			*tileMap;	/**<The "2D array"/map of Which tiles will be On or Off*/
	GFC_Vector2D	tileMapSize; /**<Vector containing the width and height of our tileMap*/	//THIS USED TO BE A POINTER .   I THINK THAT WAS A MISTAKE BUT HONESTLY I'M NOT SURE.  bc when trying to set the size in world.c after obtaining Col & row counts shit didn't work so like ..
	//yeah no I cannot envision why this was a pointer .  nvm dw about it for now
	//GFC_List		*entity_list; /**<List of entities to be contained ??? in our world?*/	//GFC_List is a dynamic list

}World;

/**
 * @brief Load/Configure a World
 * @param filename - name of the world to load
*/
World* world_load(const char* filename);

/**
 * @brief A test function purely to test whether I can create and draw a world. Works

World* world_test_new();*/

/**
 * @brief draw the world
 * @param world - the world to draw
 */
void world_draw(World* world);

/**
 * @brief Free a previously loaded World
 * @param world - the world to free
*/
void world_free(World* world);
/**
 * @brief Create new world object
 * @return NULL if unable to allocate, returns a pointer to the World otherwise
 *For testing purposes,  namely while using the world_test_new() funciton, this function took Uint32 width, Uint32 height  as parameters
*/
World* world_new();



#endif