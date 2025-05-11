#include "simple_logger.h"

#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"
#include "gfc_list.h"
//#include "camera.h"   //I shouldn't need camera  because Text is Screen-based  (just like an actual User Interface)

#include "text.h"

//Cache the font so that it's drawn less wastefully every frame
	//JUST for the sake of drawing.
typedef struct {
	Uint8			rawTag;		//Whether this cache was created from a raw C string,  or a Text object (Text Block)
	char			*text;
	GFC_TextBlock	words;
	SDL_Texture		*texture;	//these 2 are dynamically created

	FontSizes		font_size;
	GFC_Vector2D	size;
	GFC_Color		color;
	Uint32			timestamp;

}TextCache;

typedef struct {
	GFC_List *fontSizes;

	//MOVING THIS OUT OF THE FONT MANAGER
	//GFC_List* recents;		//For caching fonts

}FontManager;

typedef struct {
	Text		*text_list;
	Uint32		maxText;

	GFC_List	*recents;		//For caching fonts

}TextManager;

static FontManager font_manager = { 0 };
static TextManager text_manager = { 0 };

void font_close();
void cache_free(TextCache* cache);
void cache_free_all();
void text_free_all();
void text_system_close();

//TFF_CloseFont()'s all Fonts from FontSizes.   and TTF_Quit()'s
void font_close() {
	TTF_Font* font;
	int i, c;
	//other cleanup (if any) first
	if (font_manager.fontSizes == NULL) {
		slog("Font Sizes list already null. TTF_Quit()ing");
		TTF_Quit();
		slog("Quit TTF");
		return;
	}

	//Cleanup the small list of FontSizes we have.  Using TTF_CloseFont
	c = gfc_list_get_count(font_manager.fontSizes);
	for (i = 0; i < c; i++) {
		font = gfc_list_get_nth(font_manager.fontSizes, i);
		if (!font) continue;
		slog("Closing SDL font");
		TTF_CloseFont(font);
	}
	gfc_list_delete(font_manager.fontSizes);

	//cache free, ofc, moved to the text_system_close() function

	memset(&font_manager, 0, sizeof(FontManager));
	TTF_Quit();
	slog("Quit TTF");
}

//ONLY INITIALIZES  TTF_Init() and FontSizes
void font_init() {
	int i;
	TTF_Font* font;
	if (TTF_Init() != 0) {
		slog("failed to initialized SDL's TTF system");
		return;
	}
	else slog("TTF Initialized");

	font_manager.fontSizes = gfc_list_new(); //create the list.  gfc_list_delete()d in font_close()
	if (!font_manager.fontSizes) {
		slog("List of different Font Sizes unable to be created");
		TTF_Quit();
		return;
	}

	//create some font sizes!! :D
	for (i = 0; i < FS_MAX; i++) {
		font = TTF_OpenFont("fonts/SansSerifCollection.ttf", 20 + (i * 4));
		if (!font) {
			slog("Failed to open font");
			continue;
		}

		gfc_list_append(font_manager.fontSizes, font);
	}

	atexit(font_close);
	slog("Font system initialized");
}

void cache_free(TextCache* cache) {
	if (!cache) {
		slog("No cache provided to free");
		return;
	}

	if (cache->text) free(cache->text);  //it has come to my attention that I think I shouldn't have this line here //J ERROR
	if (cache->texture) SDL_DestroyTexture(cache->texture);
	free(cache);

}

void cache_free_all() {
	TextCache* cache;
	int i, c;
	c = gfc_list_get_count(text_manager.recents);

	for (i = 0; i < c; i++) {
		cache = gfc_list_get_nth(text_manager.recents, i);
		if (!cache) continue;
		slog("Freeing cache'd font");
		cache_free(cache);
	}
	gfc_list_delete(text_manager.recents);
}



//Let's create a text system, shall we?  :D


void text_free(Text* self) { //  font_cache_free()'s all cache'd objects.
	if (!self || !self->_inuse) return;


	if (self->surface) { SDL_FreeSurface(self->surface); slog("Text's surface freed"); }
	else { slog("What the fuck text doesn't have a surface..?"); }
	if(self->texture) { SDL_DestroyTexture(self->texture); slog("Text's texture freed"); }

	slog("");
	memset(self, 0, sizeof(Text));
	self->_inuse = 0; //Set its inuse flag to 0
}

void text_free_all() {
	int i;
	for (i = 0; i < text_manager.maxText; i++) {
		if (text_manager.text_list[i]._inuse) {
			text_free(&text_manager.text_list[i]); //free it so long as it's NOT the ignore pionter, && if it's _inuse
		}
	}
}

		//Caveat here. not quite done		(I think atExit is the thing that stacks so maybe we could
void text_system_close() {
//Free everything in the cache using our cache_free() function
	if (text_manager.recents != NULL) {
		cache_free_all();
		slog("Text Cache cleared");
	}
	text_manager.recents = NULL;

//FREE ALL TEXT
	if (text_manager.text_list != NULL) {
		text_free_all();
		free(text_manager.text_list);
		slog("List of Text freed");
	}
	text_manager.text_list = NULL;
	memset(&text_manager, 0, sizeof(TextManager));
	slog("Text System Closed");

	//ASSUMING FONT_INIT()  gets created first,,,     since it has it's own atExit() clause,  I don't need to call font_close() HERE in this function,,, I hope
}

//Done I think
void text_init(Uint32 maxText) {
	//Checking for duplicate system
	if (text_manager.text_list) {
		slog("Cannot initialize more than one Text System/List. One is already active");
		return;
	}
	if (!maxText) {
		slog("cannot initialize text system for a text limit of 0");
		return;
	}
	
	slog("In Text init.  Initializing Font_Manager to create 4 different font sizes");
	font_init();   //wait I should ask him if I should even be doing this

	//Cache
	slog("Initializing Text Cache ");
	text_manager.recents = gfc_list_new(); //create the [caching] list.  gfc_list_delete()d in text_system_close
	if (!text_manager.recents) {
		slog("Recents text list unable to be created. Deleting the list of fontSizes and TTF_Quit-ing");
		gfc_list_delete(font_manager.fontSizes);
		font_manager.fontSizes = NULL;
		TTF_Quit();
		return;
	}
	//Actual text
	text_manager.text_list = gfc_allocate_array(sizeof(Text), maxText);	//highkey double check the video for this honestly  just 'cause I wanna be sure I'm giving it 
	if (!text_manager.text_list) {
		slog("failed to access text list");
	}
	text_manager.maxText = maxText;


	atexit(text_system_close);
	slog("Text System Initialized");
}

//======================
//Creating a Text object

Text* text_new() {
	int i;
	for (i = 0; i < text_manager.maxText; i++) {
		if (text_manager.text_list[i]._inuse) {
			continue;
		}
		memset(&text_manager.text_list[i], 0, sizeof(Text)); //Clean up the space before I return it to you

		text_manager.text_list[i]._inuse = 1;  //set the inuse flag
		text_manager.text_list[i].surface = NULL; //set the surface and texture (pointers) to NULL explicitly. Just in case.
		text_manager.text_list[i].texture = NULL;
		return &text_manager.text_list[i];
	}
	slog("failed to allocate new text object: list full");
	return NULL; //return NULL outside the for loop

}

void text_obj_set_tag(Text* self, int tag) {
	if (tag == TT_Always) {
		self->tag |= TT_Always;
	}
	if (tag == TT_Inven) {
		self->tag |= TT_Inven;
	}
	if (tag == TT_Stats) {
		self->tag |= TT_Stats;
	}
	if (tag == TT_Dialogue) {
		self->tag |= TT_Dialogue;
	}
}

//Create AND configure a new() textObj [takes care of the making/allocating into the list.]
Text* create_text_raw(const char* text, FontSizes font_size, GFC_Color color, GFC_Vector2D position, TextType tag) {
	Text* self = text_new();
	
	gfc_block_cpy(self->text, text); //copy text into the Text object's text field

	//Create the font member FOR the Text object.  (just selecting font_size, really).
	self->font = gfc_list_get_nth(font_manager.fontSizes, font_size);
	if (!self->font) {
		slog("Failed to render text '%s', missing font font_size %i", text, font_size);
		return NULL;
	}

	//Set the color
	self->color = gfc_color_to_sdl(color);

	SDL_Surface* surface;  //so that I can split the two into distinct steps
	SDL_Texture* texture;

	surface = TTF_RenderUTF8_Blended_Wrapped(self->font, self->text, self->color, 0);
	if (!surface) {
		slog("Surface not rendered from text properly");
		return NULL;
	}


	self->surface = gf2d_graphics_screen_convert(&surface); //convert the surface to gf2d convention
	if (!self->surface) {
		slog("Text's surface was not converted to gf2d convention");
		SDL_FreeSurface(surface);
		return NULL;
	}

	texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	if (!texture) {
		SDL_FreeSurface(surface);
		slog("Texture was not created from surface");
		text_free(self); //Jnote:  This COULD cause a crash...  like- it SHOULDN'T!! it really really shouldn't but.. idk I've never done this before, just be weary
		return;
	}
	self->rect.x = position.x;
	self->rect.y = position.y;
	self->rect.w = self->surface->w;
	self->rect.h = self->surface->h;

	//no need to render it. This is not the draw function. Let the draw function take care of that
	//SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &self->rect);

	SDL_FreeSurface(surface);

	return self;
}

//create text from JSon object
void text_configure(Text* self, SJson* json) {
	const char* string = NULL;
	GFC_Vector4D col = { 0 };
	SDL_Surface* surface;	//so that I can split the two into distinct steps
	int tag;
	//size_t length;
	string = sj_object_get_string(json, "name");
	if (string) {
		gfc_line_cpy(self->name, string);
		//slog("name of this Text instance is: %s", self->name);  //Jlog
	}

	sj_object_get_int(json, "tag", &tag);		//SET THE TAG
	text_obj_set_tag(self, tag);
	//Things to extract


	slog("Checkpoint 1");
	string = sj_object_get_string(json, "text");
	if (string) {
		//length = strlen(string) + 1;
		gfc_block_cpy(self->text, string);		//RAAAAHH  STR COPY DOESN'T WORK  because a  const char * is a POINTERRR  IT CANNOT HOLD DATA !!! you cannot COPY characters INTO it!!!
		//strncpy(self->text, string, length);
		slog("The text to print from this Text instance is: %s", self->text);  //Jlog
	}

	sj_object_get_int(json, "font_size", &self->font_size);


	//save Colors as 4D vectors
	sj_object_get_vector4d(json, "color", &col);

	self->color = gfc_color_to_sdl( gfc_color8((int)col.x, (int)col.y, (int)col.z, (int)col.w) );
	slog("Color is: %f, %f, %f, %f", col.x, col.y, col.z, col.w);

	sj_object_get_vector2d(json, "position", &self->position);


	//Extract the font_size  (which is just a number)  from the JSon object  to create the font for the Textobj
	self->font = gfc_list_get_nth(font_manager.fontSizes, self->font_size);
	if (!self->font) {
		slog("Failed to render text '%s', missing font font_size %i", self->text, self->font_size);
		return;
	}

	surface = TTF_RenderUTF8_Blended_Wrapped(self->font, self->text, self->color, 0);
	if (!surface) {
		slog("Surface not rendered from text properly");
		return;
	}


	self->surface = gf2d_graphics_screen_convert(&surface); //convert the surface to gf2d convention
	if (!self->surface) {
		slog("Text's surface was not converted to gf2d convention");
		SDL_FreeSurface(surface);
		return;
	}

	self->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), self->surface);
	if (!self->texture) {
		SDL_FreeSurface(self->surface);
		slog("Texture was not created from surface");
		return;
	}
	self->rect.x = self->position.x;
	self->rect.y = self->position.y;
	self->rect.w = self->surface->w;
	self->rect.h = self->surface->h;

	//Render it [draw it to the screen]
	//SDL_RenderCopy(gf2d_graphics_get_renderer(), self->texture, NULL, &self->rect); 

	//Now that we've rendered the text  (into our rectangle),  we can free the Surface and Texture
	SDL_FreeSurface(surface);
	slog("Text with name '%s' configured", self->name);
}

void text_configure_from_file(Text* self, const char* filename) {
	SJson* json;
	if (!self) { slog("What the FUCK no text"); return; }
	json = sj_load(filename);
	if ( !json ) { slog("What the FUCKK NO JSON");  return; }
	slog("CALLING TEXT_CONFIGURE");
	text_configure(self, json);
	//close it
	sj_free(json);
}


//==================
//	CACHE

//For periodic cleanup of cached fonts
void cache_cleanup() {
	int i, c;
	TextCache* cache;
	Uint32 now;
	now = SDL_GetTicks();

	c = gfc_list_get_count(text_manager.recents);
	for (i = (c - 1); i >=0 ; i--) { //iterate backwards so that we don't incur the wrath of garbage values screwing up our iterating process :|
		cache = gfc_list_get_nth(text_manager.recents, i);
		if (!cache) continue;

		if (now > cache->timestamp + 10000) {
			cache_free(cache);
			gfc_list_delete_nth(text_manager.recents, i); //delete that point in the list
		}

	}

}

//Add a font to the Cache list
void font_add_recent(const char* text, FontSizes font_size, GFC_Color color, SDL_Texture *texture, GFC_Vector2D size) {
	TextCache* cache;
	size_t length;

	if ((!text) || (!texture)) return;

	cache = gfc_allocate_array(sizeof(TextCache), 1); //freed in font_cache_free()
	if (!cache) {
		slog("Cache could not be allocated");
		return;
	}
	//Copy everything over
	gfc_vector2d_copy(cache->size, size);
	cache->texture = texture;
	gfc_color_copy(cache->color, color);	
	cache->font_size = font_size;


	//Copy the text by first allocating enough space  BASED ON the length of the text
	slog("COPYING text from a FONT draw");
	length = strlen(text) + 1;
	cache->text = gfc_allocate_array(sizeof(char), length);

	strncpy(cache->text, text, length);
	
	cache->rawTag = 1;

	//Set the timestemp  [ms]
	cache->timestamp = SDL_GetTicks();

	const char* buffer[GFCTEXTLEN];		//me testing how snprintf works bc I fully did not remember
	gfc_block_sprintf(buffer, "%s HEY %i", text, 2);
	//if(buffer) slog("Buffer is equal to: %s",buffer);  //works

	//Finally,  append the cache to the list
	gfc_list_append(text_manager.recents, cache);
	slog("Text Cache created");
	slog("Text appended to Cache list, with text field: %s",cache->text);

}
//holy FUCK I think the only reason it wasn't woking when I tried to handle both cases  in ONE function.... is because I never returned NULL at the end of the for loop- OHHhhhh my .
void text_add_recent(Text *text) {
	TextCache* cache;
	size_t length;

	if ((!text) || (!text->texture)) return;

	cache = gfc_allocate_array(sizeof(TextCache), 1); //freed in font_cache_free()
	if (!cache) {
		slog("Cache could not be allocated");
		return;
	}
	//Copy everything over

	gfc_vector2d_copy(cache->size, gfc_vector2d(text->rect.w, text->rect.h)); //size of the font which isn't something I save in the Text object
	cache->texture = text->texture;
	gfc_color_copy(cache->color, gfc_color_from_sdl(text->color));
	cache->font_size = text->font_size;

	slog("Copying Word TextBlock");
	gfc_block_cpy(cache->words, text->text);
	

	cache->rawTag = 0;

	//Set the timestemp  [ms]
	cache->timestamp = SDL_GetTicks();

	//const char* buffer[GFCTEXTLEN];		//me testing how snprintf works bc I fully did not remember
	//gfc_block_sprintf(buffer, "%s HEY %i", text, 2);
	//if(buffer) slog("Buffer is equal to: %s",buffer);  //works

	//Finally,  append the cache to the list
	gfc_list_append(text_manager.recents, cache);
	slog("Text Cache created");
	slog("Text appended to Cache list, with text field: %s", cache->words);

}


TextCache* font_get_recent(const char *text, FontSizes font_size, GFC_Color color) {
	if (!text) return NULL;

	TextCache* cache;
	int i, c;
	c = gfc_list_get_count(text_manager.recents);
	for (i = 0; i < c; i++) {
		cache = gfc_list_get_nth(text_manager.recents, i);
		if (!cache) continue;

		//slog("In font get recent. Trying to compare text");
		
		//I !!!  AM GOING  to try and change this  to NOT be a str l cmp  and instead just a cmp  so that substrings (for printing out stats) works.
						//the LONGER word must be second to match.  and Text will always be changing its number
		if ( cache->rawTag != 1 || /*gfc_strlcmp*/ strcmp(cache->text, text) != 0) continue;
		

		if (cache->font_size != font_size) continue;
		if ( !gfc_color_cmp(color, cache->color) )  continue;
		return cache;
	}
	return NULL;
}

TextCache* text_get_recent(Text* self) {
	if (!self) return NULL;

	TextCache* cache;
	int i, c;
	c = gfc_list_get_count(text_manager.recents);
	for (i = 0; i < c; i++) {
		cache = gfc_list_get_nth(text_manager.recents, i);
		if (!cache) continue;

		//slog("In font get recent. Trying to compare text");

		//I !!!  AM GOING  to try and change this  to NOT be a str l cmp  and instead just a cmp  so that substrings (for printing out stats) works.
						//the LONGER word must be second to match.  and Text will always be changing its number
		if (cache->rawTag != 0 || !cache->words || gfc_block_cmp(cache->words, self->text) != 0) continue;

		if (cache->font_size != self->font_size) continue;
		if (!gfc_color_cmp(gfc_color_from_sdl(self->color), cache->color))  continue;
		return cache;
	}
	return NULL;
}


//==========================================
//	DRAWING



//TEST FUNCTION FOR PROOF OF CONCEPT/EASY ACCESS Render / Draw the text
void text_rndr( SDL_Texture *txt, SDL_Rect rect ) {
	SDL_RenderCopy(gf2d_graphics_get_renderer(), txt, NULL, &rect);
}

//NON-DYNAMIC: This one checks cache  since this one will be used for unchanging pieces of text
void font_draw(const char* text, FontSizes font_size, GFC_Color color, GFC_Vector2D position) {
	TTF_Font* font;
	SDL_Surface* surface;
	SDL_Texture* texture;
	SDL_Color fc;
	SDL_Rect rect = {0};

	TextCache* cache;
	//So NOW,  before we do anything
	cache = font_get_recent(text, font_size, color);
	if (cache) {
		rect.x = position.x;
		rect.y = position.y;
		rect.w = cache->size.x;
		rect.h = cache->size.y;
		//Go ahead and render it [actually drawing it to the screen]
		//slog("Drawing cache");
		SDL_RenderCopy(gf2d_graphics_get_renderer(), cache->texture, NULL, &rect);
		//update the timestamp for this specific piece of text
		cache->timestamp = SDL_GetTicks();
		return;
	}
	//slog("Drawing Fresh");
	//if the tet we're tryna draw is NOT already in the cache:

	font = gfc_list_get_nth(font_manager.fontSizes, font_size);  //gets a POINTER TO  one of the 4 font's we created.  This statement does NOT need to be freed anywhere down the line -- that's taken care of in font_close()
	if (!font) {
		slog("Failed to render text '%s', missing font font_size %i", text, font_size);
		return;
	}
	
	fc = gfc_color_to_sdl(color);

	surface = TTF_RenderUTF8_Blended_Wrapped(font, text, fc, 0);
	if (!surface) {
		slog("Text Surface not rendered properly");
		return;
	}

	surface = gf2d_graphics_screen_convert(&surface); //convert the surface to gf2d convention
	if (!surface) {
		slog("Surface was not converted to gf2d convention");
		return;
	}

	texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	if (!texture) {
		SDL_FreeSurface(surface);
		//slog("Texture was not created from surface");
		return;
	}
	rect.x = position.x;
	rect.y = position.y;
	rect.w = surface->w;
	rect.h = surface->h;

	//Render it [draw it to the screen]
	SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &rect);

	//Now that we've rendered the text  (into our rectangle),  we can free the Surface and Texture
	SDL_FreeSurface(surface);
	//SDL_DestroyTexture(texture);  //no longer destroying the Texture here since we're caching the pieces of Text that we make

	font_add_recent(text, font_size, color, texture, gfc_vector2d(rect.w, rect.h) );
	slog("Checkpoint 0");
}

//Yeah there is.  LITERALLY no point in trying to cache stats.
void stats_draw(const char* text, FontSizes font_size, GFC_Color color, GFC_Vector2D position, int value) {
	TTF_Font* font; //This should instead be a text object
	SDL_Surface* surface;
	SDL_Texture* texture;
	SDL_Color fc;
	SDL_Rect rect;

	//See  the thing about this is that we will NOT be using the cache
	const char* buffer[GFCTEXTLEN];
	gfc_block_sprintf(buffer, "%s %i", text, value);
	//if (buffer) slog("Buffer is equal to: %s", buffer);		//me testing how snprintf works bc I fully did not remember

	//I should get the Text object here:   oh- well.. I COULD  but this alr works so IF IT AIN'T BROKE !
	font = gfc_list_get_nth(font_manager.fontSizes, font_size);
	if (!font) {
		slog("Failed to render text '%s', missing font font_size %i", buffer, font_size);
		return;
	}
	
	fc = gfc_color_to_sdl(color);

	//And then use the TTF_Font *font  data MEMBER I have as part of my structure/class here !!!
	surface = TTF_RenderUTF8_Blended_Wrapped(font, buffer, fc, 0);
	if (!surface) {
		slog("Text Surface not rendered properly");
		return;
	}

	surface = gf2d_graphics_screen_convert(&surface); //convert the surface to gf2d convention
	if (!surface) {
		slog("Surface was not converted to gf2d convention");
		return;
	}

	texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	if (!texture) {
		SDL_FreeSurface(surface);
		slog("Texture was not created from surface");
		return;
	}
	rect.x = position.x;
	rect.y = position.y;
	rect.w = surface->w;
	rect.h = surface->h;

	SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &rect);

	//Now that we've rendered the text  (into our rectangle),  we can free the Surface and Texture
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);  //no longr destroying the Texture here since we're caching the pieces of Text that we make

	//font_add_recent(text, font_size, color, texture, gfc_vector2d(rect.w, rect.h));

}


//Ok..  so  After break-me here:   Maybe for the sake of UI Labels, I can make another function based really closely off of font_draw.  Where I ACTUALLY use the "Text" object I defined in text.h
		//'cause I'ma need something to HOUSE all this info about the font:  its size, position, the actual text it is,  SOMEWHERE.  So that it can also be drawn later.


void text_draw(Text* self, GFC_Vector2D position) {
	//I have a position local to the text object already soo forget the parameter
	if (!self) { slog("No self provided NOT DRAWING."); return; }
	if (!self->texture) { slog("Self doens't have a TEXTURE!!"); return; }
	
	SDL_Rect rect = { 0 };
	TextCache* cache;
	//So NOW,  before we do anything
	//so when I CHECK  for a cache'd font.  I want to match BASE ADDRESSES.     
	cache = text_get_recent(self);
	if (cache) {
		rect.x = position.x;
		rect.y = position.y;
		rect.w = cache->size.x;
		rect.h = cache->size.y;
		//Go ahead and render it [actually drawing it to the screen]
		//slog("Drawing cache");
		SDL_RenderCopy(gf2d_graphics_get_renderer(), cache->texture, NULL, &rect);
		//update the timestamp for this specific piece of text
		cache->timestamp = SDL_GetTicks();
		//slog("DRAWING CACHE");   //Jlog
		return;
	}
	
	
	SDL_RenderCopy(gf2d_graphics_get_renderer(), self->texture, NULL, &self->rect);

	text_add_recent(self);
}




