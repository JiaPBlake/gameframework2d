#ifndef __INVENTORY_H__
#define __INVENTORY_H_

#include "simple_json.h"

#include "gfc_list.h"
#include "gfc_config.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"
#include "items.h"

typedef enum { //because I can count the amount of numbers I have on 2 hands, enumeration would be a shout
	II_Arti,
	II_Drum,
	II_Orb,
	II_Stone,
	II_Trident,
	II_MAX

}ItemIndices;

typedef struct {
	GFC_List			*itemsList; //I'll only have some.. 7 or 8 items in my game

	//Sprite				*layer;			/**where this will be a single layer that holds all the individual Sprites drawn onto it*/
		//nvm. I can't use a layer for the same reason I can't use a layer for UI.  the stuff is always changing and I don't think I'm allowed to render smthing to the surface AFTER creating the texture ://
	Sprite				*sprite;		/**<Background sprite OF the player inventory*/
	GFC_Vector2D		sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float				frame;			/**<for drawing the sprite*/
	Uint32				framesPerLine;
	GFC_Rect			bounds;			//x,y  Top left corner
	GFC_Vector2D		position;

}Inventory;

void inventory_init(Inventory* inventory);

void inventory_cleanup(Inventory* inventory);

void inventory_draw(Inventory* inventory);
//void inventory_layer_build(Inventory* inventory);

Item* inventory_get_item_by_name(Inventory* inventory, const char* name);


void inventory_add_item(Inventory* inventory, const char* name);

void inventory_remove_item(Inventory* inventory, const char* name);

//To be used in player.c  which only has access to inventory
void display_item(Inventory* inventory, const char* name);

#endif