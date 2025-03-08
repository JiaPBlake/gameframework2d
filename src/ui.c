#include "simple_logger.h"
//#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"

#include "ui.h"



typedef struct {  //our singleton entity system/manager.  our big-ass list of entities
	Uint32			ui_elem_max;
	UI_Element*		ui_elem_list;		//this will be a POINTER  to a list of entities

}UISystem;

static UISystem ui_sub_system = { 0 };

// Calls _free_all()   and free()'s the .ui_elem_list
void ui_system_close() {  
	if (ui_sub_system.ui_elem_list != NULL)  //if our system still has entities in it  (if the pointer does not point to NULL)
	{ //free all the entities
		ui_system_free_all(NULL); //J SPECIFIC. When I want to clear absolutely everything ('cause I'm shutting down the game), pass NULL to the ignore
		free(ui_sub_system.ui_elem_list); //free the dynamically allocated list itself
		slog("UI Element List Freed");
	}
	ui_sub_system.ui_elem_list = NULL; //reset the entity list pointer
	memset(&ui_sub_system, 0, sizeof(UISystem)); //completely unecessary.  This doesn't free nor clean up anything. Just sets the data there to be 0
	slog("UI system closed");
}

//Done.   Just allocates .ui_elem_list
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
	ui_sub_system.ui_elem_max = maxElems;		//I think?? This line is the line where you initialize the Entity System's maxEnts field ? That's what is done in Sprite, and it's the only thing that makes sense in this spot.
	atexit(ui_system_close);  //I'ma highkey need him to talk again about When the atExit function takes place. I remember his emphasis on order... . Oh wait is this just when the Game itself closes?
	slog("UI system initialized");
}


//ok.  THIS function should be reserved for freeing / cleeaning MY WHOLE LIST.  highkey.  'cause WOULD there be JUST one Element I'd want to keep...?  maybe health..??
						// \|/no idea if I wanna make this a List
void ui_system_free_all(UI_Element* ignore) {  //JUST iterates and calls  entity_free()
	int i;
	for (i = 0; i < ui_sub_system.ui_elem_max; i++) {
		if (&ui_sub_system.ui_elem_list[i] != ignore && ui_sub_system.ui_elem_list[i]._inuse) {
			ui_element_free(&ui_sub_system.ui_elem_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}

void ui_element_free(UI_Element* self) {
	if (!self) return;
	
	//Each UI Element has its own Sprite.   I don't have to worry about freeing a textLine, that's not dynamic
	if (self->sprite) { 
		gf2d_sprite_free(self->sprite);
	}
	
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
	
		ui_sub_system.ui_elem_list[i]._inuse = 1; 
		return &ui_sub_system.ui_elem_list[i]; 
	}
	slog("failed to allocate new ui element: list full");
	return NULL; //return NULL outside the for loop
}

/**
 * @brief configures a UI Element - meaning sets all of its data members/parameters according to the def (json) file
 * @param json - pointer to the json object. (Created through use of the  sj_load(filename) function )
 */
void ui_element_configure(UI_Element* self, SJson* json) {
	if ((!self) || (!json)) return;

	//Let's get started .
	Uint8				_drawn;			/**<Equivalent of an inuse flag. But for a given piece of UI, if it's in use, it would ideally be drawn to the screen!*/

	const char* filename = NULL;
	filename = sj_object_get_string(json, "sprite"); //
	
	if (filename) { //if it has a sprite
		GFC_Vector2D sp_sz = { 0 };
		sj_object_get_vector2d(json, "sprite_size", &sp_sz);  //which we grabbed from gfc_config.h
		//slog("Sprite Size vector fetched from json object's x: %f", (Uint32)sp_sz.x);
		//For some reason this slog prints 0... but the saving from the sj obviously worked bc it loaded the sprite properly...
		Uint32 framesPerLine;
		sj_object_get_int32(json, "spriteFPL", &framesPerLine);
		self->sprite = gf2d_sprite_load_all(
			filename,
			(Uint32)sp_sz.x,
			(Uint32)sp_sz.y,
			framesPerLine,
			0
		);
		//GFC_Vector2D centre = gfc_vector2d(); //hehe british spelling to be different
		self->center = gfc_vector2d(sp_sz.x * 0.5, sp_sz.y * 0.5);
		self->framesPerLine = framesPerLine;
		self->frame = 0; //Since frame 0 will be the default for every entity,  just et the first frame here in the configure function

		GFC_Vector4D bounds; //Use this as a 4D vector to save the numbers from the json file. THEN uset he get Rect from 4D vector function to save it into the entity's bounds
		sj_object_get_vector4d(json, "bounds", &bounds);
		self->bounds = gfc_rect_from_vector4(bounds);

	}

	GFC_Vector2D pos = { 0 };
	sj_object_get_vector2d(json, "spawn_Position", &pos);
	self->position = pos;

	
	
	GFC_Vector2D		sprite_size;	/**<Size of the sprite. Can be divided in half for the center to pass to the Sprite draw argument*/
	float				frame;			/**<for drawing the sprite*/
	Uint32				framesPerLine;
	GFC_Rect			bounds;		//x,y  Top left corner
	GFC_Vector2D		center;
	float				rotation;

	GFC_Vector2D		position;		/**<where to draw it*/

	UI_Type				type;
	//maybe a textline or whatever here for,,,  t e xt...



	//Time to set up the think function baby oh boy
	void				(*think)(struct Ui_S* self);   /**<function to call to make decisions based on the world state*/  //The think function will take a pointer to an Entity
	void				(*update)(struct Ui_S* self);

}

void ui_element_configure_from_file(UI_Element* ui, const char* filename) {
	SJson* json;
	if (!ui) return;
	json = sj_load(filename);
	if (!json) return;
	ui_element_configure(ui, json);
	//close it
}

/**
* @brief draw all inuse entities, if they have a sprite
*/
void ui_system_draw_all();  //**********I MIGHT!!  want a parameter here for the type of encounter


/*if (_INBATTLE) {
	load ALLL battle assets


}*/
