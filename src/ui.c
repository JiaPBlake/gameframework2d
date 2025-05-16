#include <SDL_ttf.h>
#include "simple_logger.h"

#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"
#include "camera.h"

#include "ui.h"
#include "text.h"   //J testing   for Labels..


Uint8 selected_button_index = 0; //global variable that I can Set() and Get()  - primarily is use through the Player_think_battle function

typedef struct {  //our singleton system/manager
	Uint32			ui_elem_max;
	UI_Element*		ui_elem_list;		//this will be a POINTER  to a list of ui elements

}UISystem;

static UISystem ui_sub_system = { 0 };

void ui_system_close();

// Calls _free_all()   and free()'s the .ui_elem_list.  Done.
void ui_system_close() {  
	if (ui_sub_system.ui_elem_list != NULL)  //if our system still has elements in it  (if the pointer does not point to NULL)
	{ //free all the entities
		ui_system_free_all(); //J SPECIFIC. When I want to clear absolutely everything ('cause I'm shutting down the game), pass NULL to the ignore
		free(ui_sub_system.ui_elem_list); //free the dynamically allocated list itself
		slog("UI Element List Freed");
	}
	ui_sub_system.ui_elem_list = NULL; //reset the entity list pointer
	memset(&ui_sub_system, 0, sizeof(UISystem)); //completely unecessary.  This doesn't free nor clean up anything. Just sets the data there to be 0
	slog("UI system closed");
}

//Done.   Just allocates .ui_elem_list ;   atexit(ui_system_close)
void ui_system_init(Uint32 maxElems) {
	if (ui_sub_system.ui_elem_list) {
		slog("Cannot initialize more than one UI System/List. One is already active");
		return;
	}
	if (!maxElems) {
		slog("Cannot initialize ui element system for zero elements");
		return;
	}
	
	ui_sub_system.ui_elem_list = gfc_allocate_array(sizeof(UI_Element), maxElems);	//highkey double check the video for this honestly  just 'cause I wanna be sure I'm giving it 
	if (!ui_sub_system.ui_elem_list) {
		slog("Failed to access ui element list");
	}
	ui_sub_system.ui_elem_max = maxElems;
	atexit(ui_system_close);
	slog("UI system initialized");
}


void ui_system_free_all() {  //JUST iterates and calls  entity_free()
	int i;
	for (i = 0; i < ui_sub_system.ui_elem_max; i++) {
		if (ui_sub_system.ui_elem_list[i]._inuse) {
			ui_element_free(&ui_sub_system.ui_elem_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}

//updated (decided not to make dummy pointers)
void ui_element_free(UI_Element* self) {
	if (!self) return;
	
	//Based on the type of the UI Element:   (I can turn these into funciton pointers later if I want to)

	//if it's a Label, text_free() the:		text  (Text object)
	if (self->type & ELEMT_L) {
		//MATTER OF FACT .   IDEK IF I SHOULD DO THIS.  it depends on how the Labels' text is going to be created.  if it's JUST gfc_str_copying.. that's not technically malloc'd !!!
		//Yeah.  because a character pointer will just point to the colloquial string of characters at.. SOME place in memory. but that place in memory is not dynamically allocated

		//slog("About to free Label's text");
		text_free(self->ui.label.text);  //text, the data member of label  is a POINTER  (character array).  feed that array TO the free() funciton
		//slog("Label branch. Free'd text");
	}

	//if it's an Image, free the:		sprite
	if (self->type & ELEMT_I) {
		slog("image branch");
		gf2d_sprite_free(self->ui.image.sprite);  //I'm more comfortable having local variables be POINTERS TO things that already exist.  as opposed to maybe making another COPY OF an existing thing,  to store locally
		slog("Image's sprite has been freed");
	}

	//if it's a Button, free the:		sprite
	if (self->type & ELEMT_B) {
		//slog("button branch");
		gf2d_sprite_free(self->ui.button.sprite);  //I'm more comfortable having local variables be POINTERS TO things that already exist.  as opposed to maybe making another COPY OF an existing thing,  to store locally
		//slog("Button's sprite has been freed");
	}
	
	slog("We're at the end of UI_Element Free");
	//Just to set the rest of the memory that used to be in use   to 0.  
	memset(self, 0, sizeof(UI_Element));
	self->_inuse = 0;
}


//Done. Does not initialize the data members. Just returns pointer to the space in our list
UI_Element* ui_element_new() {
	int i;
	for (i = 0; i < ui_sub_system.ui_elem_max; i++) {
		if (ui_sub_system.ui_elem_list[i]._inuse)  //If this entity (from the list our pointer points to)  IS in use (_inuse flag is == 1)
			continue;
		memset(&ui_sub_system.ui_elem_list[i], 0, sizeof(UI_Element));   //Set the memory allocated at this spot to 0
	
		ui_sub_system.ui_elem_list[i]._inuse = 1;					//Set the inuse flag to 1.
		return &ui_sub_system.ui_elem_list[i]; 
	}
	slog("failed to allocate new ui element: list full");
	return NULL; //return NULL outside the for loop
}

//========================	LABEL


void label_configure(UI_Label* self, SJson* json) {
	if ((!self) || (!json)) { slog("No JSON object (OR BUTTON) provided to create the button"); return; }

	//yeah, highkey ?  Just  text_create() the object and send the json to  text_configure anyway...
		//Since !  EACH Label JSON object will be formatted the same way Text def files are. (really.. Text def files shouldn't really exist on their own unless they're.. some Fancy ass text ig)
	/*const char* string = {0};
	string = sj_object_get_string(json, "text");  //in text.c, for def files to contain Text I have it the key named 'print'
	if (!string) {
		slog("Could not get UI_Label's text from the json object");
	}
	GFC_Vector2D pos = { 0 };
	sj_object_get_vector2d(json, "button_position", &pos);*/
	//self->text = create_text_raw(string, FontSizes font_size, GFC_Color color, GFC_Vector2D position, TextType tag);

	self->text = text_new();
	text_configure(self->text, json);
	slog("The text for the Label should be configure");

}


//========================	IMAGE

	//not done.. idt
void image_configure(UI_Image* self, SJson* json) {
	if ((!self) || (!json)) { slog("No JSON object (OR BUTTON) provided to create the image"); return; }

		//well. this is easy. it's just sprite at a very specific location within our window
	const char* image_sprite = NULL;
	image_sprite = sj_object_get_string(json, "sprite");
	if (image_sprite) { //if the Window itself has a sprite
		//slog("This images's Sprite is '%s'", image_sprite);
		GFC_Vector2D i_sp_sz = { 0 };
		sj_object_get_vector2d(json, "sprite_size", &i_sp_sz);  //function from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %i", (Uint32)i_sp_sz.x);

		Uint32 framesPerLine;
		sj_object_get_int32(json, "spriteFPL", &framesPerLine);
		//load the sprite into the object's sprite data member
		self->sprite = gf2d_sprite_load_all(
			image_sprite,
			(Uint32)i_sp_sz.x,
			(Uint32)i_sp_sz.y,
			framesPerLine,
			1);
		//if (!self->sprite->surface) { slog("Image does not have a sprite SURFACE"); }			//don't think I need this
		//slog("Button Sprite's Frame Height is: %i", self->sprite->frame_h);
		self->sprite_size = i_sp_sz;
		self->center = gfc_vector2d(i_sp_sz.x * 0.5, i_sp_sz.y * 0.5);
		self->framesPerLine = framesPerLine;		//might want to data drive this ?  oh. definitely wanna data drive this.  For the Dialogue-continue-arrow
		self->frame = 0;

		//images don't really need bounds..

		/*GFC_Vector4D bounds;
		sj_object_get_vector4d(json, "bounds", &bounds); 
		self->bounds = gfc_rect_from_vector4(bounds); */

		GFC_Vector2D position = { 0 };
		sj_object_get_vector2d(json, "position", &position);
		//slog("image position should be set");
		self->position = position;

	}
	else { slog("Image Sprite couldn't be found"); }


}




//=================================	Button

int button_get_type(const char* action) {
	if (!action) { slog("No button action String provided"); return 0; }
	if (gfc_strlcmp(action, "newWindow") == 0) {
		//slog("Button should open a new window");
		return BT_NewWindow;
	}
	if (gfc_strlcmp(action, "exitWindow") == 0) {
		//slog("UI_Button should close the window");
		return BT_CloseWindow;
	}
	if (gfc_strlcmp(action, "attack") == 0) {
		//slog("UI_Button should close the window");
		return BT_Attack;
	}
	if (gfc_strlcmp(action, "converse") == 0) {
		//slog("UI_Button should close the window");
		return BT_Converse;
	}
	if (gfc_strlcmp(action, "tame") == 0) {
		slog("Tame button found.");
		return BT_Tame;
	}
	if (gfc_strlcmp(action, "flee") == 0) {
		//slog("UI_Button should close the window");
		return BT_Flee;
	}

	slog("Neither IF statement occurred,  returning ButtonType 0");
	return 0;
}

void button_configure(UI_Button* self, SJson* json) {
	if ((!self) || (!json)) { slog("No JSON object (OR BUTTON) provided to create the button"); return; }

	char* button_action_json = sj_object_get_string(json, "action");
	self->actionType = button_get_type(button_action_json);

	//Have to move this section into Window.   Right after the buttons are configured
	/*if (self->actionType == BT_NewWindow) {
		self->action = window_transition;
	}
	if (self->actionType == BT_CloseWindow) {
		//slog("This button would Close a window!");
		self->action = battle_end;
	}*/

	//Button Sprite section
	const char* button_sprite = NULL;
	button_sprite = sj_object_get_string(json, "sprite");
	if (button_sprite) { //if the Window itself has a sprite
		//slog("This button's Sprite is '%s'", button_sprite);
		GFC_Vector2D b_sp_sz = { 0 };
		sj_object_get_vector2d(json, "sprite_size", &b_sp_sz);  //function from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %i", (Uint32)b_sp_sz.x);

		//load the sprite into the object's sprite data member
		self->sprite = gf2d_sprite_load_all(
			button_sprite,
			(Uint32)b_sp_sz.x,
			(Uint32)b_sp_sz.y,
			2,
			1);
		if (!self->sprite->surface) { slog("Button does not have a sprite SURFACE"); }
		//slog("Button Sprite's Frame Height is: %i", self->sprite->frame_h);
		self->sprite_size = b_sp_sz;
		self->center = gfc_vector2d(b_sp_sz.x * 0.5, b_sp_sz.y * 0.5);
		self->framesPerLine = 2;	//yeah 'cause if I want the buttons to be highlighted upon Selection-  wait . FUCK
		self->frame = 0;

		GFC_Vector4D bounds;
		sj_object_get_vector4d(json, "bounds", &bounds); 
		self->bounds = gfc_rect_from_vector4(bounds);

		GFC_Vector2D button_pos = { 0 };
		sj_object_get_vector2d(json, "button_position", &button_pos);
		self->position = button_pos;
		//slog("This button's position is: %f, %f", self->position.x, self->position.y);
	}
	else { slog("Button Sprite couldn't be found"); }


	//Save the next window it's meant to create (IF it needs to)
	const char* button_new_window = NULL;
	if (self->actionType & BT_NewWindow) {
		if (self->actionType & BT_CloseWindow) slog("Just testing the other actionType to make sure it DOESN'T proc");

		button_new_window = sj_object_get_string(json, "nextWindow");
		gfc_line_cpy(self->windowToOpen, button_new_window);
		slog("Button's next window to open should be: %s", self->windowToOpen);
	}

}

void set_selected(int index) {
	selected_button_index = index;
}
void inc_selected() {
	if (selected_button_index < 3) {
		selected_button_index++;
	}
}
void dec_selected() {
	if (selected_button_index > 0) {
		selected_button_index--;
	}
}
Uint8 get_selected() {
	return selected_button_index;
}

void reset_selected() {
	selected_button_index = 0;
}


//==========================

void ui_element_configure(UI_Element* self, SJson* json) {
	if ((!self) || (!json)) return;

	const char* string = NULL;
	string = sj_object_get_string(json, "name");
	if (string) {
		gfc_line_cpy(self->name, string);  //something something  Copy A into B  and make sure it's not longer than the length of a Line.
		//slog("Name copied: %s",move_name);
	}

	//Depending on the type,  call either  Label_configure  or button configure
	if (self->type == ELEMT_L) {
		//slog("This UI Element is a Label");
		label_configure(&self->ui.label, json);
		self->elem_draw = label_draw;
	}

	if (self->type == ELEMT_I) {
		//slog("This UI Element is an Image");
		image_configure(&self->ui.image, json);
		self->elem_draw = image_draw;
	}

	if (self->type == ELEMT_B) {
		//slog("This UI Element is a Button.  Calling Button Configure functions");
		button_configure(&self->ui.button, json);
		self->elem_draw = button_draw;
	}

}


//idt I'll ever need this but hey!
void ui_element_configure_from_file(UI_Element* ui, const char* filename) {
	SJson* json;
	if (!ui) return;
	json = sj_load(filename);
	if (!json) return;
	ui_element_configure(ui, json);
	//close it
	sj_free(json);
}


/**
* @brief draw all inuse entities, if they have a sprite
*/
void ui_system_draw_all();  //**********I MIGHT!!  want a parameter here for the type of encounter

//I SHOULD MAKE THIS A  Draw_Entire_Window( Window* win) KINDA THING 



//me trying to bullshit a UI:  LMAO
/*UI_Element* ui_make() {
	
	UI_Element* ui = ui_element_new();
	ui->sprite_size = gfc_vector2d();
	ui->framesPerLine = 6;
	ui->sprite = gf2d_sprite_load_all(
		filename,
		(Uint32)sprite_size.x,
		(Uint32)sprite_size.y,
		framesPerLine,
		0
	);

	ui->frame = 0;


	GFC_Rect			bounds;			//x,y  Top left corner
	GFC_Vector2D		center;
	float				rotation;

	GFC_Vector2D		position;		/**<where to draw it

	return ui;
}*/

//=============================
//Draw functions

void label_draw(UI_Element* self) {
	//this should just call text_draw and that's it, really
	text_draw(self->ui.label.text, self->ui.label.text->position);
}

void image_draw(UI_Element* self) {
	int frame;

	UI_Image* image = &(self->ui.image);
	frame = 0;


	gf2d_sprite_draw(image->sprite,
		image->position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
		NULL,			//scale
		&image->center,		//center which is a 2D vector
		NULL,	//rotation
		NULL,		//flip
		NULL,		//colorShift
		frame);
}


void button_draw(UI_Element* self) {
	int frame;
	
	UI_Button* button = &(self->ui.button);
	frame = 0;

	//when drawing the elements, I want to know WHICH one is selected. So that I can make THIS specific iteration's frame = 1
	if (self->ui.button._selected == selected_button_index) {
		frame = 1;
		//slog("Selected_Button_index == %i ; SELECTED == %i", selected_button_index, selected);
	}  //and first frame selected SHOULD be 0 so !  this works actually
	else { frame = 0; }
	
	gf2d_sprite_draw(button->sprite,
		button->position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
		NULL,			//scale
		NULL,		//center which is a 2D vector
		NULL,	//rotation
		NULL,		//flip
		NULL,		//colorShift
		frame);
}

void draw_health_stat(UI_Image* self, float frame) {
	if (!self) return;
	if (!self->sprite) return; //can't work without a sprite

	GFC_Vector2D camera, position;  //Everything I draw will now have to honor the Camera's position J CAMERA
	camera = camera_get_offset();
	gfc_vector2d_add(position, camera, self->position);


	GFC_Rect rect = { 0 };

	float f = frame;
	if (frame < 0) {
		f = self->frame;
	}

	gf2d_sprite_draw(self->sprite,
		/*self->*/position,	//position      without offset we use self's position.  WITH the camera's offset, we use the position vector created above 
		NULL,				//scale
		&self->center,		//center which is a 2D vector
		&self->rotation,	//rotation
		NULL,				//flip
		NULL,				//colorShift
		(Uint32)f); //For when I make more than JUST the Willpower counter

}


/*if (_INBATTLE) {
	load ALLL battle assets


}*/
