#ifndef __PLAYER_H__
#define __PLAYER_H_

#include "entity.h"

/*
 * @brief spawn a new player entity
 * @param position where to spawn the player
 * added @param defFile  player def file
 * @return NULL on error,  or a pointer to the spawned player entity
*/
Entity* player_new_entity(GFC_Vector2D position);

/**
 * @brief player think function -- evaluate what decisions should be made based on the world state
 * @param self, pointer to the entity to be thinking
*/
void player_think(Entity* self);

/**
 * @brief player update function -- execute the decisions that were made in think, _actually_ update it to make it happen on screen.
 * @param self, pointer to the entity to be updated-ing
*/
void player_update(Entity* self);


#endif