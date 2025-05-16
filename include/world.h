#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_list.h"
#include "gf2d_sprite.h"

typedef struct {
	GFC_TextLine	name;
	Sprite			*background;
	Sprite			*midground;
	Sprite			*foreground;

	Sprite			*tileLayer; /**<prerendered tile layer*/
	Sprite			*tileSet;	/**<Sprite image of the Tiles. In my def file for the given World, tileSet would be the name of the image file for the Sprite*/
	Uint8			*tileMap;	/**<The "2D array"/map of Which tiles will be On or Off*/
	GFC_Vector2D	tileMapSize; /**<Vector containing the width and height of our tileMap*/
	
	GFC_List		*entity_list; /**<List of entities to be contained in our world*/	//GFC_List is a dynamic list

}World;


/**
 * @brief Create a single Surface layer and draw the tileSet of the World onto it
 * @param world - pointer to the world whose tileSet is to be drawn
*/
void world_tile_layer_build(World* world);

/**
 * @brief A test function purely to test whether I can create and draw a world. Works
World* world_test_new();*/


/**
 * @brief Load/Configure a World
 * @param filename - name of the world to load
 * @return NULL on error, or a pointer to the world that was loaded
*/
World* world_load(const char* filename);

/**
 * @brief Add an entity to the world's entity_list
 * @param list - pointer to the entity_list of the world; ent - pointer to the entity to add
*/
void world_add_entity(GFC_List* list, Entity* ent);

/**
 * @brief Remove an enity from the world's entity_list
 * @param list - pointer to the entity_list of the world; ent - pointer to the entity to add
*/
void world_remove_entity(GFC_List* list, Entity* ent);


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

/**
 * @brief draw the world
 * @param world - the world to draw
 */
void world_draw(World* world);

/**
 * @brief Test to see if shape is colliding with active world tiles
 * @param world = pointer to the world, shape - shape that would be colliding
 * @return NULL if no collision,  Pointer to the vector that describes the direction I collided. y=1 if player is going Down INTO a tile. x=1 if player is going right INTO a tile.

	I think I want to swap things..   return the Tile,  send the Direction vector as a pointer parameter..

 */
GFC_Rect world_test_shape(World* world, GFC_Shape shape, GFC_Vector4D* direction);

void world_collision_handle(Entity* player, GFC_Shape shape, GFC_Vector4D* dir);

//these two .  are NOT working brooo
GFC_List* world_get_collided(World* world, GFC_Shape shape);
void tile_collide_check(GFC_Rect* rect, GFC_Shape shape, GFC_Vector4D* direction);


/**
 * @brief Get the active world (bc activeWorld is static)
 * @return NULL if it failed, else a pointer to the active world, 
*/
World* world_get_active();

/**
 * @brief Ste the active world
 * @param worldToSet - the world to set Active
 */
void world_set_active(World* worldToSet);

/**
 * @brief Get the position of the ground based on the active world (bc activeWorld is static)
 * @return NULL (0 vector) if it failed, else the vector position of the first world tile that is on,
*/
GFC_Vector2D world_get_ground();

/**
 * @brief Transition the current world to the new one - free-ing up the old one and loading the new one
 * @param old - pointer to the old (current) world; newWorld - the filename of the new world to load [using world_load()]; targetPlayerSpawn - optional,,, why is it a string LOL
 * @return NULL if fail;  a pointer to the new world object created otherwise
*/
void world_transition(World* old, const char* newWorld, GFC_Vector2D targetPlayerSpawn);


#endif