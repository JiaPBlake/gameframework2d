#ifndef __TEXT_H__
#define __TEXT_H__
#include <SDL.h>
#include <SDL_ttf.h>

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef enum { //How the text would be used (when it should be drawn)
	TT_Always = 1,
	TT_Inven = 2,
	TT_Stats = 4,
	TT_Dialogue = 8
}TextType;

typedef enum {
	FS_small,
	FS_medium,
	FS_big,
	FS_really_big,
	FS_MAX
}FontStyles;

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
 * auto closes on exit
*/
void font_init();

/*
 * @brief periodically call this to clean up any internal text cache
 * 
*/
void font_cleanup();

/*
 * @brief purely for testing purposes, try to SDL_Render_Copy through another function.
	Works
*/
void text_rndr(SDL_Texture* txt, SDL_Rect rect);

/*
 * @brief render text to the screen
 * @param text - what to render
 * @param style - what style of font to render with
 * @param color - the color of the text
 * @param position - where on the screen to draw to
*/
void font_draw(const char* text, FontStyles style, GFC_Color color, GFC_Vector2D position);

//Literally does the same thing as font  but this is SPECIFICALYL for the stat screen :salute:
void stats_draw(const char* text, FontStyles style, GFC_Color color, GFC_Vector2D position, int value);

#endif