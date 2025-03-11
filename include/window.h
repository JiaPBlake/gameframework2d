#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef enum {
	BT_none = 0,
	BT_NewWindow = 1,
	BT_CloseWindow = 2,
	BT_MAX = 4, 
}ButtonType;


typedef struct Button_S {
	Uint8				_inuse;
	Uint8				_selected;    //to change frame and highlight

	Sprite				*sprite;		/**<graphical representation of the UI element*/
	GFC_Vector2D		sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float				frame;			/**<current frame of the sprite for drawing the sprite*/
	Uint32				framesPerLine; 
	GFC_Rect			bounds;			//x,y  Top left corner
	GFC_Vector2D		center;
	GFC_Vector2D		position;		/**<where to draw it*/

	float				rotation;	 //deadass DO NOT know how rotation works 

	//MAYBE FONT??   it'll all be hand-drawn anyway


	// Implement the Collision in player.c   JUST like world [write the function here, include it].  That's an order.
	//wait .  no fuck you we're operating on Enters.   YUP YUP!!  WASD AND ENTERS
	ButtonType			actionType;
	void				(*action)(struct Button_S* but);
	GFC_TextLine		windowToOpen;


}Button;


typedef struct Window_S {
	//wait fuck I lied. maybe I ALSO should have an _inuse flag..?? :|     I'M THINKING TOO HIGH. I'm thinkin' too large .
	Uint8				_inuse;
	Uint8				_drawn;			/**<Equivalent of an inuse flag. But for a given piece of UI, if it's in use, it would ideally be drawn to the screen!*/
	
	struct Window_S		*prev;  /**<Pointer to a previous window*/

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

	GFC_TextBlock		body;
	//Font  //aha .  I'm making a Font type,,,,

	GFC_List			*button_list; /**<List of button pointers*/


	//To implement user interaction:
	//int					(*collide)(struct Ui_S* self, struct Entity_S* other, EntityCollisionType type); //oh fuck me. I wanna do with using the cursor :SOB:


}UI_Window;

void button_system_init(Uint32 maxButtons);

void button_system_free_all();

/**
 * @brief free a previously created button
 * #param Pointer to the button to free
 */
void button_free(Button* self);

Button* button_new();

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

Button* button_create(SJson* json);

/**
 * @brief Configures all the data members of a Button
 * @param json - pointer to the JSon object that will be used to fill out the Button's data fields
 * @return NULL on error; pointer to the Button object that was made otherwise;
 */
void button_configure(Button* self, SJson* json);

void window_configure(UI_Window* self, SJson* json);
//UI_Window* window_configure(const char* filename);

void window_configure_from_file(UI_Window* self, const char* filename);
//UI_Window* window_configure_from_file(const char* filename);

void window_draw(UI_Window* window);

void window_layer_build(UI_Window* window);

void window_transition(Button* self);
void window_go_back();

UI_Window* window_get_prev();
UI_Window* window_get_next();
UI_Window* window_get_active();
void window_set_active(UI_Window* windowToSet);


//For selecting buttons
void set_selected(int index);
void inc_selected();
void dec_selected();

Uint8 get_selected();

void reset_selected();
/**
 * @brief Perform the action of the button selected
 * @param selected - the index of the selected button ; win - pointer to the window the button belongs to
 */
void button_perform_action(int selected, UI_Window* win);

#endif