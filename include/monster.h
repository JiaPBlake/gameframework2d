#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

/**
 * @brief spawn a new player entity
 * @param position where to spawn t
 * @return NULL on error,  or a pointer to the spawned player entity
*/

Entity* monster_new_entity(GFC_Vector2D position, const char* defFile);

void monster_think(Entity* self);
void monster_update(Entity* self);

void monster_data_free(Entity* self);
#endif