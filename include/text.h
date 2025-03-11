#ifndef __TEXT_H__
#define __TEXT_H__
#include <SDL.h>
#include <SDL_ttf.h>

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef enum {
	TT_Always = 1,
	TT_Inven = 2,
	TT_Stats = 4,		//RIPPED STRAIGHT FROM ENTITY.  THIS IS FOR EXAMPLE. DELETE THIS LATER LMAO
	TT_Dialogue = 8
}TextType;


typedef struct Font_S{		
		//wait fuck I lied. maybe I ALSO should have an _inuse flag..?? :|     I'M THINKING TOO HIGH. I'm thinkin' too large .
	Uint8				_inuse;
	Uint8				_drawn;			/**<Equivalent of an inuse flag. But for a given piece of UI, if it's in use, it would ideally be drawn to the screen!*/
	
	TTF_Font			*font;

	GFC_Vector2D		position;		/**<where to draw it*/
	GFC_TextBlock		body;			/**<To be set with gfc_block_cpy(dst, src)*/

	TextType			tag; //For things like  Inventory (The names for the items);  Stats;  Dialogue

	//definitely won't need these in the conventional sense.  But text will be udpating based on stats
	void				(*think)(struct Ui_S* self);   /**< a */ 
	void				(*update)(struct Ui_S* self);  /**< a */ 
	//To implement user interaction:
	//int					(*collide)(struct Ui_S* self, struct Entity_S* other, EntityCollisionType type); //oh fuck me. I wanna do with using the cursor :SOB:


}Font;

/*
 * @brief purely for testing purposes, try to SDL_Render_Copy through another function.
	Works
*/
void text_rndr(SDL_Texture* txt, SDL_Rect rect);

#endif