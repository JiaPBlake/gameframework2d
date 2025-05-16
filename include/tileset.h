#ifndef __TILESET_H__
#define __TILESET_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_list.h"
#include "gf2d_sprite.h"

typedef struct {		//OR  all this nifo could just be in a Def file.
	Uint8			frame;  //which tile in the tile set
	Uint8			blocks; //if true, prevent movement
	GFC_TextLine	name; //for debugging purposes  ? :)		Remember this is all just demonstration.  I'm probably not gonna use this system.  But it's an option!!
	float			damage;
	float			movementMod;
}TileDef;

typedef struct {
	GFC_TextLine		name;
	Sprite				*tiles;
	GFC_Vector2D		tileSize;
	GFC_List			*tileDefs;

}TileSet;

#endif