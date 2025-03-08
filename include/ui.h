#ifndef __UI_H__
#define __UI_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"

//typedef enum {
//	ECL_none = 1,
//	ECL_World = 2,
//	ECL_Entity = 4,		//RIPPED STRAIGHT FROM ENTITY.  THIS IS FOR EXAMPLE. DELETE THIS LATER LMAO
//	ECL_Item = 8,
//	ECL_ALL = 16
//}EntityCollisionLayers;

typedef enum {
	//UIT_Border = 1,
	UIT_Loading = 1,
	UIT_Battle = 2, 
	UIT_Inventory = 4,
	UIT_All = 8,
	UIT_MAX
}UI_Type;
/*For example:
* Layer 1 = 1							00001
* Layer 2 = 2							00010
* Layer 3 = 4							00100
* Layer 4 = 8	which correspondes to:	01000  respectively.
*/

typedef struct Ui_S{		
		//wait fuck I lied. maybe I ALSO should have an _inuse flag..?? :|     I'M THINKING TOO HIGH. I'm thinkin' too large .
	Uint8				_inuse;
	Uint8				_drawn;			/**<Equivalent of an inuse flag. But for a given piece of UI, if it's in use, it would ideally be drawn to the screen!*/
	
	Sprite				*sprite;		/**<graphical representation of the UI element*/
	GFC_Vector2D		sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float				frame;			/**<for drawing the sprite*/
	Uint32				framesPerLine;		
	GFC_Rect			bounds;		//x,y  Top left corner
	GFC_Vector2D		center;
	float				rotation;		

	GFC_Vector2D		position;		/**<where to draw it*/
	
	UI_Type				type;
	GFC_TextBlock		body;
	//maybe a textline or whatever here for,,,  t e xt...



	//Don't think I need these at all but.  Here's the framework just in case  WAIT LIED MY ASS OFF LMAO this is how I'm going to click on shit.
	void				(*think)(struct Ui_S* self);   /**< a */ 
	void				(*update)(struct Ui_S* self);  /**< a */ 
	//To implement user interaction:
	//int					(*collide)(struct Ui_S* self, struct Entity_S* other, EntityCollisionType type); //oh fuck me. I wanna do with using the cursor :SOB:


}UI_Element;

		//hooohh fuck  do I want to do this using a SYSTEM???  ... maybe I should just start small first-
//HOLD ON!!   yes.  I should make a System, JUST like Sprite, and just like Entity.  'cause the whole point of a system is to Create, allocate, and load alll the assets we know we're going to need.
  //Every single encounter will have the exact same UI.  Save for the monster and it's respective attributes.  And,,, whatever I got going on with the Player-


/**
 * @brief initializes sub system for UI Elements
 * @param maxEnts upper limit for how many entitise ca exist at once
 */
void ui_system_init(Uint32 maxElems);

/**
 * @brief free all UI Elements in the manager;
 */						// \|/no idea if I wanna make this a List
void ui_system_free_all(UI_Element* ignore); //J SPECIFIC:  adding an entity to ignore when cleaning up

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



#endif