#ifndef __EXIT_H__
#define __EXIT_H__

#include "entity.h"

/**
 * @brief Think function for the entity that serves as my exit point 
*/
void exit_think(Entity* self);

/**
 * @brief Free the data specific to the Exit nature  OF the entity.  The Entity Manager will free the entity itself
*/
void exit_free(Entity* self);

/**
 * @brief Create an Exit entity
*/
Entity* exit_new_entity(GFC_Vector2D position, const char* defFile);


#endif