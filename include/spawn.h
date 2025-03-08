#ifndef __SPAWN_H__
#define __SPAWN_H__

#include "entity.h" //includes Vector2D


/**
 *@brief spawn an entity by its name
 *@param name the name of the entity
*/
Entity* spawn_entity(const char* name, GFC_Vector2D position);

//Entity* spawn_entity_from_json(SJson* item);



#endif