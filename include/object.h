#ifndef __OBJECT_H__
#define __OBJECT_H_

#include "entity.h"

/*
 * @brief spawn a new object entity
 * @param position where to spawn the object
 * added @param defFile  player def file
 * @return NULL on error,  or a pointer to the spawned player entity
*/
Entity* object_new_entity(GFC_Vector2D position, const char* fileName);

/**
 * @brief Just tesing out the functioning of Exits  with Objects 'cause idk how / WHEN to clean up exit data
*/
void object_think(Entity* self);

//void object_update(Entity* self);

#endif