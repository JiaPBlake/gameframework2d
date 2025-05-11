#ifndef __UI_H__
#define __UI_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"


#include "text.h" //For UI Labels

/*For example:
* Layer 1 = 1							00001
* Layer 2 = 2							00010
* Layer 3 = 4							00100
* Layer 4 = 8	which correspondes to:	01000  respectively.
*/
typedef enum {
	//UIT_Border = 1,
	UIT_Loading = 1,
	UIT_Battle = 2, 
	UIT_Inventory = 4,
	UIT_All = 8,
	UIT_MAX
}UI_Type;


/*------------------------------------------------------------------ -
		Let's start reworkin' shit		*/


typedef struct Label_S {
	Uint8				_selected;		/**< Flag for whetehr the button is selected (to change frame and highlight)*/
	Text				*text;		//NEEDS TO BE FREED
	//See..  I could do 2 things here with the Labels..  I COULD Have a flag. Which will tell me if the Label is dynamic. (Like my Dragon Point Stats).
				//and based on that flag, the specific Font Draw function would be determined-  actually.  FUCK that's not even how function pointers WORK!! >:(  My 2 draw functions have a diff. set of parameters!!! >:((
}UI_Label;


typedef struct Image_S {
	Uint8				_selected;		/**< Flag for whetehr the button is selected (to change frame and highlight)*/

	Sprite				*sprite;		/**< graphical representation of the UI element*/			//NEEDS TO BE FREED
	GFC_Vector2D		sprite_size;	/**< Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float				frame;			/**< current frame of the sprite for drawing the sprite*/
	Uint32				framesPerLine;
	GFC_Rect			bounds;			//x,y  Top left corner
	GFC_Vector2D		center;
	GFC_Vector2D		position;		/**< where to draw it*/
	float				rotation;	 //deadass DO NOT know how rotation works 


}UI_Image;

typedef enum {
	BT_none = 0,
	BT_NewWindow = 1,
	BT_CloseWindow = 2,
	BT_Attack = 4,
	BT_Converse = 8,
	BT_Tame = 16,
	BT_Flee = 32,
	BT_MAX = 64
}ButtonType;

typedef struct Button_S {
	Uint8				_selected;		/**< Flag for whetehr the button is selected (to change frame and highlight)*/
	Uint8				index;			//to perform button actions in the battle menu. 


	Sprite				*sprite;		/**< graphical representation of the UI element*/
	GFC_Vector2D		sprite_size;	/**< Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float				frame;			/**< current frame of the sprite for drawing the sprite*/
	Uint32				framesPerLine;
	GFC_Rect			bounds;			//x,y  Top left corner
	GFC_Vector2D		center;
	GFC_Vector2D		position;		/**< where to draw it*/
	float				rotation;	 //deadass DO NOT know how rotation works 

	//MAYBE FONT?? [a.k.a Label ?]   it'll all be hand-drawn anyway


	// Implement the Collision in player.c   JUST like world [write the function here, include it].  That's an order.
	//wait .  no fuck you we're operating on Enters.   YUP YUP!!  WASD AND ENTERS			lmao - April 2nd (so, after the mideterm, whereas these 2 lines were before.  WINDOW is supposed to handle input
	ButtonType			actionType;
	void				(*action)(struct Button_S* but);
	GFC_TextLine		windowToOpen;		//because this is allocated with a specific number of characters  a.k.a not malloc'd or anything.  No need to free().

}UI_Button;

typedef enum
{
	ELEMT_L = 1,
	ELEMT_I = 2,
	ELEMT_B = 4,
	ELEMT_MAX = 7
}ElemTypes;

typedef struct
{
	Uint8				_inuse;
	//  include EVERYTHING in this one thing :o
		//... actually- ok. My thing is. I can do this 2 ways (Check the Google Doc)
	ElemTypes		type;    //if we have a Rectangle,  we can ONLY ACCESS  .r  (well.. we COULD access any, but they'd be garbage values 'cause we never defined them)
	union
	{                   //Union lets us kinda like. CHOOSE which one of these members (s)  we wanna us for a given Shape.  We CHOOSE which one we want to use. Hence why we keep track of the type ^ above
		UI_Label	label;			//And Labels will just overlyyyy use my text.c functions
		UI_Image	image;
		UI_Button	button;
	}ui;

	//Free function can be assigned BASED ON  a given object's->type  data member
		//yeah no I did not do this ^   I don't PLAN on doing this.  I did all 3 possible cases in 1 Free function
	void					(*data_free)(struct Entity_S* self); /**<function to call to free any Sub-class specific Entity data (e.g. Player or Caves/Exits)*/
	void					(*elem_draw)(struct UI_Element* self/*, int selected*/);		//J note: I have since made use of the _selected flag for buttons lmao

}UI_Element;


//----------------------------------------


void button_system_init(Uint32 maxButtons);

void button_system_free_all();

/**
 * @brief free a previously created button
 * #param Pointer to the button to free
 */
void button_free(UI_Button* self);

UI_Button* button_new();


/**
 * @brief configures a Button UI Element specifically - meaning sets all of its data members/parameters according to the def (json) file
 * @param json - pointer to the json object. (Created through use of the  sj_load(filename) function )
 */
void ui_button_configure(UI_Button* self, SJson* json);

/**
 * @brief Create (and configure)  a UI_Element
 * @param element - pointer to the json object - which should be a list of a certain UI Element (i.e. a list of Buttons)
 */
UI_Element*	ui_elem_create(SJson* element);
//I actually dk which of these I wanna use..   I think I'm going to stick with the resource maanger.  Thus: Create it with ui_element_new,  THEN configure it with this void function
void ui_element_configure(UI_Element * self, SJson * json);

//	----------	------------	---------------	---------------


/**
 * @brief initializes sub system for UI Elements
 * @param maxEnts upper limit for how many entitise ca exist at once
 */
void ui_system_init(Uint32 maxElems);

/**
 * @brief free all UI Elements in the manager;
 */
void ui_system_free_all();

/**
* @brief draw all inuse entities, if they have a sprite
*/
void ui_system_draw_all();  //**********I MIGHT!!  want a parameter here for the type of encounter


/**
 * @brief Create a ui element.
 * @return NULL if not enough space; else pointer to the ui element's position in the list
 * @note Does not initialize the data members. Just allocates the space
 */
UI_Element* ui_element_new();

		//and then if I used a system.  That Systme would contain a list of UI's  (a pointer to a contiguous place in memory).  so I could make a
		//void clean_all_ui()    function that would iterate through that list and call clean_ui_element() a  poop ton of times
/**
 * @brief Free all the aspects of a given element
 * @param element - the element to clean up
 */
void ui_element_free(UI_Element *element);

/**
 * @brief configures a UI Element - meaning sets all of its data members/parameters according to the def (json) file
 * @param self - pointer to the UI element to configue; json - pointer to the json object. (Created through use of the  sj_load(filename) function )
 */
void ui_element_configure(UI_Element* self, SJson* json);

/**
 * @brief configures a UI Element Using the filename as a parameter. Will create the json object for you, then call ui_element_configure()
 * @param self - Pointer to the UI Element to configure; filename - the name of the file as a string
 */
void ui_element_configure_from_file(UI_Element* self, const char* filename);


void adjust_health(Sprite* healthbar);

//UI_Element* ui_make();

void label_draw(UI_Element* self);
void button_draw(UI_Element* self);

//For selecting buttons
void set_selected(int index);
void inc_selected();
void dec_selected();

Uint8 get_selected();

void reset_selected();



void draw_health_stat(UI_Element* self, float frame);

void draw_stats();

#endif