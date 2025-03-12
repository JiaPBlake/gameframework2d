#include <SDL.h>
#include <SDL_ttf.h>
#include "simple_logger.h"

#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "player.h"  //J ADDED:
#include "monster.h"
#include "object.h"
#include "world.h"
#include "camera.h"
#include "spawn.h"
//#include "ui.h" //J TO BE ADDED
#include "window.h"
#include "items.h" //J TESTING
#include "text.h" //J TO BE TESTED


Uint8 _DRAWBOUNDS = 1;          //.... I need to think of a better way to implement these paths but.  Later..
//Uint8 _CHANCE1 = 1;        //A change flag (or 3)  that will be initialized as some number,  and will increment repeatedly  (from 0 to 9) as the game loop goes
extern Uint8 _INVENTORY_FLAG;
extern Uint8 _INBATTLE;
Entity* otherEnt;
Uint32 _MOUSEBUTTON;
Uint8 _PRESSED = 0;
extern Uint8 health_frame;
int mouseClickTimer = 0; //10 frames

//void parse_args(int argc, char* argv[]);

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    //Sprite *sprite;
    World* world;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(100,100,100,200);//J CHANGE:  Changing the Cursor color / Pointer color.  Used to 255, 100, 255, 200
    Entity* player;   //J START:
    Entity* thePlayer;
    Entity* monster;
    Entity* cave;
    GFC_Vector2D player_position;
    player_position.x = 20;
    player_position.y = 200;
    GFC_Vector2D monster_position = gfc_vector2d(600, 400);
    GFC_Vector2D default_pos = gfc_vector2d(-1, -1);
    Sprite* battle_alert;
    Sprite* health_bar;
    //Uint32 mouseButton;  //making this a global flag instead to let any file see the Mouse state
    //UI_Element* stats_screen;  //J TO BE ADDED (UI)
    //UI_Window* battleWin;


    /*program initializtion*/
    init_logger("gf2d.log",0);
    //parse_args(argc, argv); //argc will always be 1 at LEAST  //J ADDED
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    //do NOT INITIALIZE  gfc input here.   gfc_update()  is per class.  So if you want to update in PLAYER, initalizing it n game wouldn't work.
    gf2d_graphics_set_frame_delay(16);          //It will wait 16 ms  before each frame - AT MINIMUM.
    gf2d_sprite_init(1024);
    entity_system_init( 100 );    //J ADDED - initalize our Entity system AFTER the sprite system.  Since it depends on the sprites
    font_init();          //J TO BE ADDED (FONTS)  please delete the other fonts down below
    
    button_system_init(20);
    window_system_init(10);
    items_initialize("def/items.def");

    SDL_ShowCursor(SDL_DISABLE);

    //TTF_Init();          //J ADDED
    if (TTF_WasInit() == 0) slog("Text not initialized"); else slog("TTF system intialized");

    //Testing fonts
    /*SDL_Surface* surfaceMessage = NULL;
    SDL_Texture* Message = NULL;
    SDL_Rect Message_rect; //create a rect 

    //this opens a font style and sets a size
    TTF_Font* test_font = TTF_OpenFont("fonts/SansSerifCollection.ttf", 24);
    if (!test_font) {
        slog("No font holy shit");
        TTF_CloseFont(test_font);
    }
    
    else {
        SDL_Color White = { 255, 255, 255 };

        // as TTF_RenderText_Solid could only be used on  SDL_Surface then you have to create the surface first
        surfaceMessage = TTF_RenderText_Solid(test_font, "RAAAAAAAGHHHGHGH", White);
        if (!surfaceMessage) { slog("Text Surface not created"); }

        // now you can convert it into a texture
        Message = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceMessage);  
        if (!Message) { slog("Text Texture not created"); }

        Message_rect.x = 0;  //controls the rect's x coordinate 
        Message_rect.y = 0; // controls the rect's y coordinte
        Message_rect.w = 300; // controls the width of the rect
        Message_rect.h = 200; // controls the height of the rect

        // Now since it's a texture, you have to put RenderCopy
        // in your game loop area, the area where the whole code executes

        // you put the renderer's name first, the Message, the crop size (you can ignore this if you don't want to dabble with cropping),
        // and the rect which is the size and coordinate of your texture
        

        // Don't forget to free your surface and texture
    }*/

//--------------------------------

    /*demo setup*/
    //sprite = gf2d_sprite_load_image("images/backgrounds/dest_bg.png");  //J CHANGE:  used to be "images/backgrounds/bg_flat.png"
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    
    slog("press [escape] to quit");
    //J START:
    //monster = monster_new_entity(/*monster_position*/ default_pos, "def/fierce.def" ); //artifact of old thinking
    //cave = object_new_entity(default_pos, "def/cave.def" );
    //player = player_new_entity(default_pos /*player_position */, "def/player.def");//artifact of old thinking
    world = world_load("def/levels/treasure_trove.level"); /*world_test_new();*/
    battle_alert = gf2d_sprite_load_all("images/border.png", 605, 74, 1, 0);
    health_bar = gf2d_sprite_load_all("images/healthbar_full.png", 320, 64, 6, 0);
    
    GFC_Vector2D h_center = gfc_vector2d(160, 32);
    //stats_screen = ui_make();     //J TO BE ADDED (UI)
    
    UI_Window* starting_win = window_new();
    window_configure_from_file(starting_win, "def/testWindow.def");
    
    
    //battleWin = window_configure("def/testWindow.def");
    //if (!battleWin) { slog("Shit. no window"); }
    ////else { slog("Name of the window, to prove I got it to Configure: %s",battleWin->name); }
    //Button *but1 = gfc_list_get_nth(battleWin->button_list, 0);
    //if (but1) {
    //    slog("Button Sprite's FPL %i", but1->sprite->frames_per_line);
    //}

    //Sprite* attack = gf2d_sprite_load_all("images/ui/attack_button.png", 180, 100, 2, 0);     this was me trying to draw the attack button. I forgot to include either gfc_config of gf2d_graphics in my window.c file I forget which one. but ONE function wasn't defined and that's why it wasn't working
    //Button* but2 = gfc_list_get_nth(win->button_list, 1);  //just testing to make sure the Button list of a given window was properly accessible
    
    thePlayer = player_get_the();

    //Let's try spawning some entities          --Has since been updated so that the WORLD loaded will take care of spawning the entities needed per world
    //player = spawn_entity("player", default_pos);   //Using this Spawn function takes care of everything.  The entity is MADE and CONFIGURED, 
    //monster = spawn_entity("fierce_dragon", default_pos);  //now I just..  dk what to do with the pointer to the thing... Maybe I don't even need it..?  Bc all 3 of these entities
    //cave = spawn_entity("cave_f", default_pos);       //are IN my ent manager. They exist in the list..  They'd even be drawn to the screen without these pointer assignments !

    //otherEnt = monster; //artifact of old thinking (collision check)
    camera_set_size(gf2d_graphics_get_resolution());  //Feb 26: J added


    /*main game loop*/
    while(!done)
    {
        //world = world_get_active();
        //_CHANCE1++;
        SDL_PumpEvents();   // update SDL's internal event structures   //J NOTE - must be called once a frame. If not called, nothing updates, then none of these conditions will ever set
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        
        font_cleanup(); //to periodically clean up out fonts  //J ADDED (TEXT)
        _MOUSEBUTTON = SDL_GetMouseState(&mx,&my);  //HAHAAA  MY MOUSEBUTTON VARIABLE WORKS!! YIPPEEE works for clicks AND holds!!  although that being said a human "click" runs like 5 times in this loop lmao
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
        if (mouseClickTimer <= 0) { //We can click the mouse (leave it 1). And Reset the cooldown
            mouseClickTimer = 10;
        }
        else if (mouseClickTimer > 0) {  //On cooldown.  Do not register the click
            _MOUSEBUTTON = 0;
            mouseClickTimer--;
        }  
        

        entity_system_think_all();   //THINK FIRST   //J ADDED
        entity_system_update_all();  //then update shit  //J ADDED
        camera_bounds_check(); //Feb 26: J Added
        //world = world_get_active();  //I used to have this here because I planned on transitioning worlds in a file INSTEAD Of just calling the thing in the main game loop.  WHICH I could still probably do but .  Proof of concept comes first

        //WORKSSSS
        /*if (_MOUSEBUTTON) {
            if (!_PRESSED) /*world = world_transition(world_get_active(), "def/levels/testLevel.level", thePlayer->position);
            slog("World successfully reassigned, %s. Back to the main Game loop",world->name);
            _PRESSED = 1;
        }*/
        //slog("HERE");
        if (_MOUSEBUTTON) {
            //slog("Player current position X: %f  &  Y: %f", thePlayer->position.x, thePlayer->position.y);
            slog("click detected. Enabling flag to draw the item");
            _INVENTORY_FLAG = 1;
        }


        gf2d_graphics_clear_screen();// clears drawing buffers                //J NOTE: ALL YOUR DRAW CALLS must be within Clear and Next_Frame
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            //gf2d_sprite_draw_image(sprite,gfc_vector2d(0,0));    //don't need this background anymore
            
            
            if (world) {
                //slog("oh bitch we drawin");
                world = world_get_active();
                world_draw(world);   //J ADDED
                if (_DRAWBOUNDS) world_draw_bounds(world); //J ADDED
            }
            entity_system_draw_all();    //draw shit -- I want my entities to exist in front of the background, but drawn before the mouse  //J TO BE ADDED
            
            //draw all items.. ?
            
            //Maybe I should have a extern flag in here? _INBATTLE.  so that I can start drawing the Battle UI when need-be
            if (_INBATTLE) {
                //slog("in battle");
                
                //..  basically.  I want a single draw call here:    to Draw the Proper UI_Elements.  While I have them THINK out there ^
                window_draw(window_get_active());
                /*GFC_Vector2D center = gfc_vector2d(302, 67);
                gf2d_sprite_draw(
                    battle_alert,
                    gfc_vector2d(600,100),
                    NULL,
                    &center,
                    NULL,
                    NULL,
                    NULL,
                    0);*/
            }
            //slog("Button's turn");
            /*gf2d_sprite_draw(
                attack,
                but1->position,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0);*/
            
            if (_INVENTORY_FLAG) {
                player_show_inven(thePlayer);
                stats_draw("Fierce:", FS_medium, GFC_COLOR_RED, gfc_vector2d(50, 120), get_player_points(ENT_fierce)); //we'll just try it with Fierce first
                stats_draw("Docile:", FS_medium, GFC_COLOR_CYAN, gfc_vector2d(50, 185), get_player_points(ENT_docile)); //we'll just try it with Fierce first
                stats_draw("Cunning:", FS_medium, GFC_COLOR_GREEN, gfc_vector2d(50, 250), get_player_points(ENT_cunning)); //we'll just try it with Fierce first
            }
            gf2d_sprite_draw(
                health_bar,
                gfc_vector2d(300, 650),
                NULL,
                &h_center,
                NULL,
                NULL,
                NULL,
                health_frame);

            //draw_stats(stats_screen, health_frame);  //J TO BE ADDED (UI)

            //SDL_RenderCopy(gf2d_graphics_get_renderer(), Message, NULL, &Message_rect);
            //text_rndr(Message, Message_rect);
            
     //------------ tEXT     
            
            font_draw("Press 'ESC' to quit", FS_small, GFC_COLOR_WHITE, gfc_vector2d(940,5));
            
            

            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    //entity_free(player); //J ADDED
    //entity_free(monster); //J ADDED
    world_free(world);  //My worlds are not maaged by one System. I will not initialize them ALL before the game loads up. I need to free EACH one when needed. soooo somewhere within Encounter.c
    
    //slog( "Button 2's position: %f", but2->position.x);  //just testing to make sure the Button list of a given window was properly accessible
    //window_free(win);
    //TTF_CloseFont(test_font);     //J FONT
    //SDL_FreeSurface(surfaceMessage);
    //SDL_DestroyTexture(Message);

    //TTF_Quit();       //J FONT
    slog("Text system de-initialized");

    slog("---==== END ====---");
    return 0;
}

/*void parse_args(int argc, char* argv[]) {
    int i;
    if (argc < 2) return;
    for (i = 1; i < argc; i++) {
        if (gfc_strlcmp( "--drawbounds", argv[i]) == 0) {
            _DRAWBOUNDS = 1;
        }
    }
}*/

/*eol@eof*/
