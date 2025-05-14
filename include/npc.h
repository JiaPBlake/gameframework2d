#ifndef __NPC_H__
#define __NPC_H__

#include "entity.h"

/**
 * @brief spawn a new NPC entity
 * @param position where to spawn t
 * @return NULL on error,  or a pointer to the spawned player entity
*/
Entity* npc_new_entity(GFC_Vector2D position, const char* defFile);

/**
 * @brief Configure the data unique to an NPC
 * @param self - pointer to the entity
 * @param defFile - the string representing the name of the def file
*/
void npc_data_configure(Entity* self, const char* defFile);

/**
 * @brief Clean up NPC-specific data
 * @param self - pointer to the entity
*/
void npc_data_free(Entity* self);

void npc_think(Entity* self);
void npc_update(Entity* self);		//I doubt NPC's are gonna do much updating..  MAYBE a sprite change if I feel like.. god damn making the skeletons fall apart unpon being interacted with???
	//do NOT try to use npc_update  as a function pointer,, LOL   every NPC wouuld carry out its action every frame bc of entity_update_all(),,,,


#endif