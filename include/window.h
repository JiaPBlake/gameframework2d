#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"

#include "ui.h"

//Yeah  plop this into UI Element once that's done	esp since Window to Open is just a textline
	//Button has since been plopped into ui.h


typedef struct Window_S {
	//wait fuck I lied. maybe I ALSO should have an _inuse flag..?? :|     I'M THINKING TOO HIGH. I'm thinkin' too large .
	Uint8				_inuse;
	Uint8				_drawn;			/**<Equivalent of an inuse flag. But for a given piece of UI, if it's in use, it would ideally be drawn to the screen!*/
	Uint32				winid;			/**<Unique ID number for the Window*/
	struct Window_S		*prev;			/**<Pointer to a parent window*/
	struct Window_S		*next;			/**<Pointer to a child window*/

	Sprite				*layer; //where this will be a single layer that holds all the individual Sprites drawn onto it

	GFC_TextLine		name;
	Sprite				*sprite;		/**<graphical representation of the UI element*/
	GFC_Vector2D		sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float				frame;			/**<for drawing the sprite*/
	Uint32				framesPerLine;
	GFC_Rect			bounds;		//x,y  Top left corner
	GFC_Vector2D		center;	//Just note that Using this center in the params for gf2d_sprite_draw()  WILL misplace it..
	GFC_Vector2D		position;		/**<where to draw it*/
	float				rotation;	 //deadass DO NOT know how rotation works 

	//GFC_TextBlock		body;
	//Font  //aha .  I'm making a Font type,,,,

	//Mayhaps I make this a list of Elements. Wherein Elements is a structure that's a union of.. however many different types of Widgets there are,, yippee !
	//GFC_List			*button_list; /**<List of button pointers*/  //artifact of old thinking

	GFC_List			*element_list; /**<List of button pointers*/
	Uint8				labelCount;
	Uint8				imageCount;
	Uint8				buttonCount;

	//To implement user interaction:
	//int					(*collide)(struct Ui_S* self, struct Entity_S* other, EntityCollisionType type); //oh fuck me. I wanna do with using the cursor :SOB:


}UI_Window;



//How he used the union operator in gfc_shape.h - Line 40   (just for reference)
/*typedef struct
{
	GFC_ShapeTypes type;    //if we have a Rectangle,  we can ONLY ACCESS  .r  (well.. we COULD access any, but they'd be garbage values 'cause we never defined them)
	union
	{                   //Union lets us kinda like. CHOOSE which one of these members (s)  we wanna us for a given Shape.  We CHOOSE which one we want to use. Hence why we keep track of the type ^ above
		GFC_Circle c;
		GFC_Rect r;
		GFC_Edge2D e;
	}s;
}GFC_Shape;

//And then as further example for how  the more general class of  Shape  is used in a more specific manner (like,  as a Rectangle):
GFC_Shape gfc_shape_rect(float x, float y, float w, float h)
{
	GFC_Shape shape;
	shape.type = ST_RECT;
	shape.s.r.x = x;
	shape.s.r.y = y;
	shape.s.r.w = w;
	shape.s.r.h = h;
	return shape;
}*/



//-------------------------------------------

/**
 * @brief initializes sub system for UI Windows - containing Buttons
 * @param maxWindow - upper limit for how many windows can exist at once
 */
void window_system_init(Uint32 maxWindows);

/**
 * @brief free all windows in the manager;
 */
void window_system_free_all();

/**
 * @brief free a previously created ui window
 * @param Pointer to the ui window to free
 */
void window_free(UI_Window*);

/**
 * @brief get a new empty entity to work with
 * @return NULL if out of room for windows,  or a blank window otherwise
 */
UI_Window* window_new();

/**
 * @brief Get the ButtonType according to the action of the button
 * @param action - the string obtained from the json key "action"
 * @return 0 if no action was provided or did not match. Otherwise, returns the enumerated value for the corresponding ButtonType
 */
int button_get_type(const char* action);

//J:To be deleted
UI_Button* button_create(SJson* json);

/**
 * @brief Configures all the data members of a UI_Button
 * @param json - pointer to the JSon object that will be used to fill out the UI_Button's data fields
 * @return NULL on error; pointer to the UI_Button object that was made otherwise;
 */		//J:To be deleted
void button_configure(UI_Button* self, SJson* json);


void window_configure(UI_Window* self, SJson* json);
//UI_Window* window_configure(const char* filename);

void window_configure_from_file(UI_Window* self, const char* filename);
//UI_Window* window_configure_from_file(const char* filename);

void window_draw(UI_Window* window);

void window_layer_build(UI_Window* window);

void window_transition(UI_Button* self);
void window_go_back();

UI_Window* window_get_prev();
UI_Window* window_get_next();
UI_Window* window_get_active();
void window_set_active(UI_Window* windowToSet);

/**
 * @brief Perform the action of the button selected
 * @param selected - the index of the selected button ; win - pointer to the window the button belongs to
 */
void button_perform_action(int selected, UI_Window* win);

#endif