#ifndef __NPC_H__
#define __NPC_H__

#include "entity.h"

/**
 * @brief spawn a new NPC entity
 * @param position where to spawn t
 * @return NULL on error,  or a pointer to the spawned player entity
*/
Entity* npc_new_entity(GFC_Vector2D position, const char* defFile);

void npc_think(Entity* self);
void npc_update(Entity* self);		//I doubt NPC's are gonna do much updating..  MAYBE a sprite change if I feel like.. god damn making the skeletons fall apart unpon being interacted with???

void npc_data_free(Entity* self);

#endif