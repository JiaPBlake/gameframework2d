#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "gfc_shape.h" //shape includes vector,  and vector includes type
#include "entity.h"

/*
 * @brief Check if one entity has collided with another
 * @param owner - The entity (main character) that may have moved onto a new entity
 * @param other - The other entity, the one who is BEING collided with
 * @return false if nothing collided, true if they collided
*/
Bool collision_check(Entity *owner, Entity *other);


#endif