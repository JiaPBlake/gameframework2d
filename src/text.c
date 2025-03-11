#include "simple_logger.h"

#include "gfc_config.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"
//#include "camera.h"   //I shouldn't need camera  because Text is Screen-based  (just like an actual User Interface)

#include "text.h"


// Render / Draw the text
void text_rndr( SDL_Texture *txt, SDL_Rect rect ) {
	SDL_RenderCopy(gf2d_graphics_get_renderer(), txt, NULL, &rect);
}
