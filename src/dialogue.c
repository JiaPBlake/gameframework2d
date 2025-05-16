#include <SDL_ttf.h>
#include "simple_logger.h"
#include "simple_json.h"


#include "gfc_config.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"

#include "dialogue.h"
#include "text.h"   //J testing   for Labels..



typedef struct {  //our singleton entity system/manager.  our big-ass list of entities
	Uint32		max_dialogue;
	Dialogue*	dialogue_list;		//this will be a POINTER  to a list of entities

}DialogueSystem;

static DialogueSystem dial_manager = { 0 };


void dialogue_free_all();
void dialogue_system_close();

//	Done.
void dialogue_free(Dialogue *self) { //  font_cache_free()'s all cache'd objects.
	if (!self || !self->_inuse) return;
	int i, c;
	Text* txt;
	if (self->textList) {
		c = gfc_list_count(self->textList);
		//slog("The number of entities in the World about to be freed is: %i",c);	//Jlog
		for (i = 0; i < c; i++) {
			txt = gfc_list_nth(self->textList, i);
			if (!txt) continue;
			if(txt->text) slog("Text of: '%s' is going to be freed from Dialoguelist",txt->text); //Jlog

			text_free(txt);
		}
		gfc_list_delete(self->textList); //delete the GFC_List itself
	}

	memset(self, 0, sizeof(Dialogue));
	self->_inuse = 0; //Set its inuse flag to 0
}

//good.
void dialogue_free_all() {
	int i;
	for (i = 0; i < dial_manager.max_dialogue; i++) {
		if (dial_manager.dialogue_list[i]._inuse) {
			dialogue_free(&dial_manager.dialogue_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}

//done.
void dialogue_system_close() {
	//FREE ALL TEXT
	if (dial_manager.dialogue_list != NULL) {
		dialogue_free_all();
		free(dial_manager.dialogue_list);
		slog("System's List of Dialogue freed");
	}
	dial_manager.dialogue_list = NULL;
	memset(&dial_manager, 0, sizeof(DialogueSystem));
	slog("Dialogue System Closed");
}

//done:   atexit(dialogue_system_close)
void dialogue_init(Uint32 max_dialogue) {
	//Checking for duplicate system
	if (dial_manager.dialogue_list) {
		slog("Cannot initialize more than one Dialogue System/List. One is already active");
		return;
	}
	if (!max_dialogue) {
		slog("cannot initialize dialogue system for a dialogue limit of 0");
		return;
	}

	dial_manager.dialogue_list = gfc_allocate_array(sizeof(Dialogue), max_dialogue);	//highkey double check the video for this honestly  just 'cause I wanna be sure I'm giving it 
	if (!dial_manager.dialogue_list) {
		slog("failed to access dialogue system's list");
	}
	dial_manager.max_dialogue = max_dialogue;

	atexit(dialogue_system_close);
	slog("Dialogue System Initialized");
}

//======================
//Creating a Dialogue object

//done.
Dialogue* Dialogue_new() {
	int i;
	for (i = 0; i < dial_manager.max_dialogue; i++) {
		if (dial_manager.dialogue_list[i]._inuse) {
			continue;
		}
		memset(&dial_manager.dialogue_list[i], 0, sizeof(Dialogue)); //Clean up the space before I return it to you

		dial_manager.dialogue_list[i]._inuse = 1;  //set the inuse flag
		dial_manager.dialogue_list[i].textList = NULL;	//JUST in case. working with double pointers is just.. ever so slightly scary.

		return &dial_manager.dialogue_list[i];
	}
	slog("failed to allocate new dialogue object: list full");
	return NULL; //return NULL outside the for loop

}


//======================================================



Dialogue dialogue_entity_configure(Entity* ent, SJson* json) {
	
	Dialogue* new_dialogue = { 0 };

	int i, c;

	new_dialogue->diaCount = sj_array_get_count(json);


}

void dialogue_configure(Dialogue* self, SJson* json) {
	int i;
	const char *string;
	Text *textPointer;
	GFC_Vector4D col = { 0 };
	GFC_Color color;
	GFC_Vector2D pos = { 0 };
	self->diaCount = sj_array_get_count(json);
	
	//save Colors as 4D vectors
	sj_object_get_vector4d(json, "color", &col);
	color = gfc_color8((int)col.x, (int)col.y, (int)col.z, (int)col.w);
	//slog("Color is: %f, %f, %f, %f", color.x, color.y, color.z, color.w);


	sj_object_get_vector2d(json, "position", &pos);


	for (i = 0; i < self->diaCount; i++) {
		string = sj_get_string_value(sj_array_get_nth(json, i));  //since the json object we're working with should ALREADY be  'dialogue', the array. get each member of the array
		if (string) {
			slog("The line extracted from the #%i Line of Dialogue is: %s", i, string);
			textPointer = create_text_raw(string, FS_medium, color, pos, 8);
		}
	}

}


/*
"dialogue": ["I am a wall... A special wall.. One might even say legendary...",
		"So legendary, in fact, that I can impart wisdom onto you:",
		"Bepis."
	],
*/
