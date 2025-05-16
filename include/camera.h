#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_config.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"


//*Might wanna make the world bigger to test this effectively
typedef struct {
	GFC_Rect camera; //He nearly said that the camera could JUST BE  a rectangle.   typedef GFC_Rect Camera;   was nearly the only line we needed
	GFC_Rect bounds;
	
}Camera;

void camera_set_position(GFC_Vector2D position);

/**
 * @brief set size of the camera (the screen)
 * @param size - the length and width for the camera
*/
void camera_set_size(GFC_Vector2D size);


/**
 * @brief set the bounds of the camera (the world)
 * @param bounds - the Rectangle that the camera cannot leave
*/
void camera_set_bounds(GFC_Rect bounds);

/**
 * @brief get the position of the camera in World Space
 * @return the 2D vector of the camera's position in world space
*/
GFC_Vector2D camera_get_position();

/**
 * @brief get the draw offset to put something in camera space
 * @return the negative of the 2D vector of the camera's position in world space; to be added to the draw position of all sprites
*/
GFC_Vector2D camera_get_offset();

/**
 * @brief make the camera centered on the provided point
*/
void camera_center_on(GFC_Vector2D point);


/**
 * @brief make sure the camera has not left the bounds
*/
void camera_bounds_check();

/**
 * @brief make sure the camera has not left the bounds
 * @return the rectangle composed of the camera's bounds a.k.a the size of the World
*/
GFC_Rect camera_get_bounds();


#endif