#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gf2d_sprite.h"

typedef struct {
	GFC_TextLine	name;
	Sprite			*background;
	GFC_Vector2D	size;
	Sprite			*tileSet;
	Uint8			*tileMap;
	GFC_Vector2D	*tileMapSize;

	GFC_List		*entity_list;	//GFC_List is a dynamic list

}World;

#endif