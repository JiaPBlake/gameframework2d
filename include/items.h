#ifndef __ITEMS_H__
#define __ITEMS_H__

#include "simple_json.h"

#include "gfc_config.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h" //includes Types, Color, Vector, and Text
#include "entity.h"

//I just peeped what if I wanna make these entities...   hmmmm  .  Mayhaps the thing on the Map is an Entity
		//and the thing in my inventory is the Item !   like quake ! ahhHHH!
//SO perhaps I can leave the Sprite stuff up to the Entity version of Items ..?


//nvm  he added sprites.   Which I was thinking of doing anyway 'cause trying to split up the 2 sounds WAYY too convoluted

typedef struct {
	Uint8					_drawn;			//NOW I'll use this flag instead of _inuse..  because I'll SET the drawn flag to 1 IN WORLD  as I load all the world related items I need
	GFC_TextLine			name;

	Sprite					*sprite;		/**<graphical representation of the entity*/   //Pointer TO the sprite data managed by the Sprite Manager. As opposed to a copy of the picture
	GFC_Vector2D			sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float					frame;			/**<for drawing the sprite*/
	Uint32					framesPerLine;
	GFC_Vector2D			position;		/**<where to draw it*/
	GFC_Rect				bounds;		//x,y  Top left corner

	int						count;

}Item;


/**  
 * @brief initialize and load item definitions (the global variables)
 * @param filename - the json file containing item data
 */
void items_initialize(const char* filename);

/**
 * @brief get an item definition by its name
 * @param name - the search criteria
 * @return NULL if not found, otherwise the definition information of the item
 * @not do not free this data
 */
SJson* items_get_def_by_name(const char* name);

/**
 * @brief return the configured Item object using the Definition object stored in the list
 * @param name - the name of the object to create/configure
 * @Note:  Dynamically allocates space for the Item Object in this function. NEEDS to be freed with the item_free() function.  Done in inventory
 */
Item* item_new(const char* name);

/**
 * @brief free the item and its necessary data members (sprite)
 * @param item - pointer to the item that should be freed
 */
void item_free(Item* item);

/**
 * @brief return the configured Item object using the Definition object stored in the list
 * @param name - the name of the object to create/configure
 */
Entity* item_new_entity(GFC_Vector2D position, const char* defFile);


#endif