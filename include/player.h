#ifndef __PLAYER_H__
#define __PLAYER_H_

#include "entity.h"

/**
 * @brief spawn a new player entity
 * @param position where to spawn the player
 * added @param defFile  player def file
 * @return NULL on error,  or a pointer to the spawned player entity
*/

Entity* player_new_entity(GFC_Vector2D position);

void player_think(Entity* self);

void player_update(Entity* self);


#endif