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

typedef enum {		//Right now (April 2, '25), this is only used in font_init()  to append Font objects of different sizes, but the SAME actual FONT STYLE, to our Manager's lsit
	FS_small,
	FS_medium,
	FS_big,
	FS_really_big,
	FS_MAX
}FontSizes;

//Fonts  are the resources I'm managing.  Text objects will have Fonts

typedef struct Text_S{		
		//wait fuck I lied. maybe I ALSO should have an _inuse flag..?? :|     I'M THINKING TOO HIGH. I'm thinkin' too large .
	Uint8				_inuse;
	Uint8				_drawn;			/**<Equivalent of an inuse flag. But for a given piece of UI, if it's in use, it would ideally be drawn to the screen!*/
	TextType			tag;			//For things like  Inventory (The names for the items);  Stats;  Dialogue
	GFC_TextLine		name;   //perhaps for the sake of searching for thing to print

	//What makes up Text (for cache and drawing purposes)
	TTF_Font			*font;			/**<Pointer to the font (really, just a pointer to the specific font size) that the Text object is using.    No need to be freed while working with Text. Because said Font Object is only opened once - in the font_init() function*/
	GFC_TextBlock		text;			/**<The actual text to draw*/
	FontSizes			font_size;		/**<Font Size from the small list of possible sizes we have up there in the enumeration^*/
	SDL_Color			color;			/**<Color of the text (SDL COLOR  NOT GFC) */
	GFC_Vector2D		position;		/**<Where the text should be drawn*/   //the position will be used to make the rectangle (and the rectangle will be used for drawing it)
	
//ACTUALLY??  I DO want to save the surface and texture info.  I'll just clean it all up later (in the text_close function_
	//actual drawing:
	SDL_Surface			*surface;		/**<The surface information created from the font object.						NEEDS TO BE FREED*/
	SDL_Texture			*texture;		/**<The texture information created from the surface and the font object		NEEDS TO BE FREED*/
	SDL_Rect			rect;



	//definitely won't need these in the conventional sense.  But text will be udpating based on stats
	void				(*think)(struct Text_S* self);   /**< a */ 
	void				(*update)(struct Text_S* self);  /**< a */ 
	//To implement user interaction:
	//int					(*collide)(struct Ui_S* self, struct Entity_S* other, EntityCollisionType type); //oh fuck me. I wanna do with using the cursor :SOB:


}Text;


/*
 * @brief purely for testing purposes, try to SDL_Render_Copy through another function.
	Works
*/
void text_rndr(SDL_Texture* txt, SDL_Rect rect);

/*
 * @brief Initialize the font system (Through the use of TTF_Init).  PRIMARILY just for the different sized Fonts.
 * @note: auto closes atexit with font_close()
*/
void font_init();


/*
 * @brief Initialize the sub system responsible for keeping track of struct Text objects.
 * @param maxText - the maximum number of Text instances
 * @note: auto closes atexit			; ...matter of fact I might just make this call font_init(). So I don't have to put font_init() in game.c
*/
void text_init(Uint32 maxText);

/*
 * @brief Free a previousy created Text object
 * @param self - a pointer to the Text object
*/
void text_free(Text* self);

/*
 * @brief Allocate a space on the textList
 * @returns a pointer to the space on the list where the Text object is housed
*/
Text* text_new();

/*
 * @brief: ADD (through the use of |= ) a tag to a given Text object
 * @param: self - pointer to the Text object whose tag should be set
 * @param: tag - the integer equivalent of the TextType tag
*/
void text_obj_set_tag(Text* self, int tag);

/*
 * @brief: Initialize the sub system responsible for keeping track of struct Text objects.
 * @param: self - pointer to the Text object that needs to be configured
 * @param: filename - name of the (JSON) file to configure from
*/
void text_configure_from_file(Text* self, const char* filename);


/*
 * @brief periodically call this to clean up any internal text cache
*/
void cache_cleanup();

/*
 * @brief Draw font to the screen by supplying text in the function.  DOES NOT use the Text structure.
 * @param text - what to render
 * @param font_size - what FontSize render the font with		-- may implement styles (like. Comic Sans vs Arial - JUST AS AN EXAMPLE - styles of fonts later)
 * @param color - the color of the text
 * @param position - where on the screen to draw to
*/
void font_draw(const char* text, FontSizes font_size, GFC_Color color, GFC_Vector2D position);

//Literally does the same thing as font  but this is SPECIFICALLY for the stat screen :salute:
void stats_draw(const char* text, FontSizes font_size, GFC_Color color, GFC_Vector2D position, int value);



//I noticed I have no cache functions explicitly declared here


//After break additions:

/*
 * @brief Draw a Text object (extracts the info of the Text object for you in order to call font_draw)
*/
void text_draw(Text* txtObj, GFC_Vector2D position);





#endif