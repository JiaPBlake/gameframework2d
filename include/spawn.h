#ifndef __SPAWN_H__
#define __SPAWN_H__

#include "entity.h" //includes Vector2D


/**
 * @brief spawn an entity by its name
 * @param name the name of the entity
 * @param position - the position provided through the map def file.   If no position is specified in the LEVEL, then the default position specified by the ENTITY's def file will be used
 * @param spawn_coords -- the list of Spawn Coordinates (which is a 3D vector.  The z value representing the value of the tile)
*/
Entity* spawn_entity(const char* name, GFC_Vector2D position, GFC_List* spawn_coords);

/**
 * @brief Parse through the list of spawn coordinates (given to us by the world/map)
 * @param name the name of the entity
 * @param spawn_coords -- the list of Spawn Coordinates (which is a 3D vector.  The z value representing the value of the tile)
 * @return - the 2D vector representing the position of the entity to be spawned
*/
GFC_Vector2D find_entity_spawn_location(const char* name, GFC_List* spawn_coords);
//Entity* spawn_entity_from_json(SJson* item);



#endif