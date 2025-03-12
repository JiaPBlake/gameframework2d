#ifndef __PLAYER_H__
#define __PLAYER_H_

#include "entity.h"


/*
 * @brief spawn a new player entity
 * @param position where to spawn the player, defFile - definition file that defines all of the player entity attributes
 * @return NULL on error,  or a pointer to the spawned player entity
*/
Entity* player_new_entity(GFC_Vector2D position, const char* defFile);

/*
 * @brief return the player
 * @return NULL on error,  or a pointer to the player entity
*/
Entity* player_get_the();

/*
 * @brief return the points of the type provided that the player currently has
 * @return the number of points the player has.  -1 if something went wrong
*/
int get_player_points(EntityType type);


/**
 * @brief player think function -- evaluate what decisions should be made based on the world state
 * @param self, pointer to the entity to be thinking
*/
void player_think(Entity* self);

void player_think_battle(Entity* self);


/**
 * @brief player update function -- execute the decisions that were made in think, _actually_ update it to make it happen on screen.
 * @param self, pointer to the entity to be updated-ing
*/
void player_update(Entity* self);

/**
 * @brief free function to clean up the various pieces of Player-specific entity data
 * @param self, pointer to the entity whose data needs to be cleaned
*/
void player_data_free(Entity* self);



//Inventory testing
void player_show_inven(Entity* self);


#endif