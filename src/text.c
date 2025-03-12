#include "simple_logger.h"

#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"
#include "gfc_list.h"
//#include "camera.h"   //I shouldn't need camera  because Text is Screen-based  (just like an actual User Interface)

#include "text.h"

//Cache the font so that it's drawn less wastefully every frame
typedef struct {
	char			*text;
	SDL_Texture		*texture;	//these 2 are dynamically created

	FontStyles		style;
	GFC_Vector2D	size;
	GFC_Color		color;
	Uint32			timestamp;

}FontCache;

typedef struct {
	GFC_List *fonts;

	GFC_List* recents;

}FontManager;

static FontManager font_manager = { 0 };

void font_close();
void font_cache_free(FontCache* cache);


void font_close() {
	TTF_Font* font;
	FontCache* cache;
	int i, c;
	//other cleanup first

	c = gfc_list_get_count(font_manager.fonts);
	for (i = 0; i < c; i++) {
		font = gfc_list_get_nth(font_manager.fonts, i);
		if (!font) continue;
		slog("Closing SDL font");
		TTF_CloseFont(font);
	}
	gfc_list_delete(font_manager.fonts);

	c = gfc_list_get_count(font_manager.recents);
	for (i = 0; i < c; i++) {
		cache = gfc_list_get_nth(font_manager.recents, i);
		if (!cache) continue;
		slog("Freeing cache'd font");
		font_cache_free(cache);
	}
	gfc_list_delete(font_manager.recents);

	memset(&font_manager, 0, sizeof(FontManager));
	TTF_Quit();
}

void font_init() {
	int i;
	TTF_Font* font;
	if (TTF_Init() != 0) {
		slog("failed to initialized SDL's TTF system");
		return;
	}
	else slog("TTF Initialized");

	font_manager.fonts = gfc_list_new(); //create the list.  gfc_list_deleted in font_close
	if (!font_manager.fonts) {
		slog("Font list unable to be created");
		TTF_Quit();
		return;
	}

	font_manager.recents = gfc_list_new(); //create the list.  gfc_list_deleted in font_close
	if (!font_manager.recents) {
		slog("Recent font list unable to be created");
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

		gfc_list_append(font_manager.fonts, font);
	}

	atexit(font_close);
}

void font_cache_free(FontCache* cache) {
	if (!cache) {
		slog("No cache provided to free");
		return;
	}

	if (cache->text) free(cache->text);
	if (cache->texture) SDL_DestroyTexture(cache->texture);
	free(cache);

}

void font_cleanup() {
	int i, c;
	FontCache* cache;
	Uint32 now;
	now = SDL_GetTicks();

	c = gfc_list_get_count(font_manager.recents);
	for (i = c - 1; i >=0 ; i--) { //iterate backwards so that we don't incur the wrath of garbage values screwing up our iterating process :|
		cache = gfc_list_get_nth(font_manager.recents, i);
		if (!cache) continue;

		if (now > cache->timestamp + 5000) {
			font_cache_free(cache);
			gfc_list_delete_nth(font_manager.recents, i); //delete that point in the list
		}

	}

}

void font_add_recent(const char* text, FontStyles style, GFC_Color color, SDL_Texture *texture, GFC_Vector2D size) {
	FontCache* cache;
	size_t length;

	if ((!text) || (!texture)) return;

	cache = gfc_allocate_array(sizeof(FontCache), 1); //freed in font_cache_free
	if (!cache) {
		slog("Cache could not be allocated");
		return;
	}
	//Copy everything over
	gfc_vector2d_copy(cache->size, size);
	cache->texture = texture;
	gfc_color_copy(cache->color, color);	
	cache->style = style;

	//Copy the text by first allocating enough space  BASED ON the length of the text
	length = strlen(text) + 1;
	cache->text = gfc_allocate_array(sizeof(char), length);

	strncpy(cache->text, text, length);
	//Set the timestemp  [ms]
	cache->timestamp = SDL_GetTicks();

	const char* buffer[GFCTEXTLEN];		//me testing how snprintf works bc I fully did not remember
	gfc_block_sprintf(buffer, "%s HEY %i", text, 2);
	if(buffer) slog("Buffer is equal to: %s",buffer);

	//Finally,  append the cache to the list
	gfc_list_append(font_manager.recents, cache);
	slog("Text appended to Cache list");

}

FontCache* font_get_recent(const char *text, FontStyles style, GFC_Color color) {
	if (!text) return NULL;

	FontCache* cache;
	int i, c;
	c = gfc_list_get_count(font_manager.recents);
	for (i = 0; i < c; i++) {
		cache = gfc_list_get_nth(font_manager.recents, i);
		if (!cache) continue;

		//slog("In font get recent. Trying to compare text");
		
		//I !!!  AM GOING  to try and change this  to NOT be a str l cmp  and instead just a cmp  so that substrings (for printing out stats) works.
							//the LONGER word must be second to match.  and Text will always be changing its number
		if ( /*gfc_strlcmp*/ strcmp(cache->text, text) != 0) continue;
		//slog("Checkpoint 1");
		if (cache->style != style) continue;
		if ( !gfc_color_cmp(color, cache->color) )  continue;
		return cache;
	}
}



// Render / Draw the text
void text_rndr( SDL_Texture *txt, SDL_Rect rect ) {
	SDL_RenderCopy(gf2d_graphics_get_renderer(), txt, NULL, &rect);
}

//This one checks cache  since this one will be used for unchanging pieces of text
void font_draw(const char* text, FontStyles style, GFC_Color color, GFC_Vector2D position) {
	TTF_Font* font;
	SDL_Surface* surface;
	SDL_Texture* texture;
	SDL_Color fc;
	SDL_Rect rect;

	FontCache* cache;
	//So NOW,  before we do anything
	cache = font_get_recent(text, style, color);
	if (cache) {
		rect.x = position.x;
		rect.y = position.y;
		rect.w = cache->size.x;
		rect.h = cache->size.y;
		//Go ahead and render it
		SDL_RenderCopy(gf2d_graphics_get_renderer(), cache->texture, NULL, &rect);
		//update the timestamp for this specific piece of text
		cache->timestamp = SDL_GetTicks();
		return;
	}
	//slog("Drawing Fresh");
	//if the tet we're tryna draw is NOT already in the cache:

	font = gfc_list_get_nth(font_manager.fonts, style);
	if (!font) {
		slog("Failed to render text '%s', missing font style %i", text, style);
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
	//SDL_DestroyTexture(texture);  //no longr destroying the Texture here since we're caching the pieces of Text that we make

	font_add_recent(text, style, color, texture, gfc_vector2d(rect.w, rect.h));

}

//Yeah there is.  LITERALLY no point in trying to cache it.
void stats_draw(const char* text, FontStyles style, GFC_Color color, GFC_Vector2D position, int value) {
	TTF_Font* font;
	SDL_Surface* surface;
	SDL_Texture* texture;
	SDL_Color fc;
	SDL_Rect rect;

	//See  the thing about this is that we will NOT be using the cache
	const char* buffer[GFCTEXTLEN];		//me testing how snprintf works bc I fully did not remember
	gfc_block_sprintf(buffer, "%s %i", text, value);
	//if (buffer) slog("Buffer is equal to: %s", buffer);

	font = gfc_list_get_nth(font_manager.fonts, style);
	if (!font) {
		slog("Failed to render text '%s', missing font style %i", buffer, style);
		return;
	}
	
	fc = gfc_color_to_sdl(color);

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

	//font_add_recent(text, style, color, texture, gfc_vector2d(rect.w, rect.h));

}
