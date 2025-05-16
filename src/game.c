#include <SDL.h>
#include <SDL_ttf.h>
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_audio.h"      //J TO BE ADDED
#include "gfc_config_def.h" //J TO BE ADDED

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

//Start  //J ADDED:
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "object.h"
#include "world.h"
#include "camera.h"
#include "spawn.h"
//#include "ui.h" //J TO BE ADDED
#include "window.h" //includes ui.h
#include "items.h" 
#include "text.h" //J TESTING.  Works as a basic thing. Gotta optimize and move things around         **ALSO INCLUDED IN WINDOW --> UI
#include "particles.h"  //J TO BE TESTED (CLASS)
#include "moves.h"  //J testing

#include "dialogue.h"  //J testing

#include "mainmenu.h"


Uint8 _DRAWBOUNDS = 0;          //.... I need to think of a better way to implement these paths but.  Later..
//Uint8 _CHANCE1 = 1;        //A change flag (or 3)  that will be initialized as some number,  and will increment repeatedly  (from 0 to 9) as the game loop goes
extern Uint8 _INVENTORY_FLAG;
extern Uint8 _INBATTLE;
Entity* otherEnt;
Uint32 _MOUSEBUTTON;
Uint8 _PRESSED = 0;
extern Uint8 health_frame;
int mouseClickTimer = 0; //10 frames


Uint8 _START_SCREEN;
Uint8 _GAME_STARTED;
Uint8 _PAUSED;
Uint8 _ANIMPLAYING;
//void parse_args(int argc, char* argv[]);

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    //Sprite *sprite;
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(100,100,100,200);//J CHANGE:  Changing the Cursor color / Pointer color.  Used to 255, 100, 255, 200
   
    
    //J START:
    Entity* thePlayer;
    Entity* monster;
    Entity* cave;
    GFC_Vector2D mouse_position;
    GFC_Vector2D default_pos = gfc_vector2d(-1, -1);
    UI_Window* window;

    World* world;

    Sprite* health_bar;
   // GFC_Sound* test_sound;
    GFC_Sound* music;

    //Text* testText;  //Jtested  works!

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
    

    

    //slog("Initializing Config File");  //Jlog
    gfc_input_init("config/my_input.cfg");  //this is the funciton we use to initalize our inputs  a.k.a our keybinds!!
    //That being said--  there's already a SAMPLE confic within the gfc folder: gameframework2d\gfc\sample_config
    gf2d_graphics_set_frame_delay(16);          //It will wait 16 ms  before each frame - AT MINIMUM.
 
     
    gf2d_sprite_init(1024);

    //  I will put initialize_game_state()   here  NEED TO INCLUDE MAINMENU.H
    initialize_game_state();        //_START_SCREEN set to 1, and _PAUSED set to 0 in here.
    dialogue_init(15);


    //SJson* dialogueList;
    //dialogueList = sj_object_get_value(json, "dialogue");
    ////This  SJson variable  is a JSon array.
    //slog("Checkpoint 1");
    ////dialogue_configure(self, dialogueList);
    //dialogue_configure(test_dia, dialogueList);
    //dialogue_free(test_dia);


    //  OKAY !!!  IT WORKS!!

    //Old initialization functions:
/*
    //J TO BE ADDED (AUDIO):
    gfc_audio_init(
        100,
        64,
        4,
        1,
        1,
        1);
//   entity_system_init( 100 );    //J ADDED - initalize our Entity system AFTER the sprite system.  Since it depends on the sprites
//   text_init(50);  //I believe this works.  I've used it enough times to know. We're good  :)

//    ui_system_init(100);
//    window_system_init(10);
//    window_masterlist_initialize("def/ui_windows.def");
//    configure_all_windows();

//    items_initialize("def/items.def");
//    move_masterlist_initialize("def/moveList.def");
//    move_system_init(20);
//    configure_all_moves();
*/
    

    //uncomment this once your Windows are in a working state

    //particle_system_init(1000);  //J TO BE ADDED particles
    ////what the FUCK
    //gfc_config_def_init();
    //gfc_config_def_load("config/particleList.cfg");   //Cheks what the resource nam



    SDL_ShowCursor(SDL_DISABLE);        //He had the cursor hidden because he uses his own sprite for it !

    /*Testing fonts  - leaving this here as I got it from an online source and I like their wording explaining each line of code :)
    SDL_Surface* surfaceMessage = NULL;
    SDL_Texture* Message = NULL;
    SDL_Rect Message_rect; //create a rect 
    SDL_Color White = { 255, 255, 255 };

    // as TTF_RenderText_Solid could only be used on  SDL_Surface then you have to create the surface first
    surfaceMessage = TTF_RenderText_Solid(test_font, "RAAAAAAAGHHHGHGH", White);
    if (!surfaceMessage) { slog("Text Surface not created"); }

    // now you can convert it into a texture
    Message = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceMessage);  
    if (!Message) { slog("Text Texture not created"); }



    // Now since it's a texture, you have to put RenderCopy
    // in your game loop area, the area where the whole code executes

    // you put the renderer's name first, the Message, the crop size (you can ignore this if you don't want to dabble with cropping),
    // and the rect which is the size and coordinate of your texture
        

    // Don't forget to free your surface and texture
    */

//--------------------------------

    /*demo setup*/
    //sprite = gf2d_sprite_load_image("images/backgrounds/dest_bg.png");  //J CHANGE:  used to be "images/backgrounds/bg_flat.png"
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    
    slog("press [escape] to quit");
    
    
    //J START:
//    world = world_load("def/levels/treasure_trove.level"); /*world_test_new();*/
    health_bar = gf2d_sprite_load_all("images/healthbar_full.png", 320, 64, 6, 0);
    GFC_Vector2D h_center = gfc_vector2d(160, 32);
    
    //litERALLY  waht the fuck audio:
    music = gfc_sound_load("audio/Ahrix_Nova_shorter.mp3", 0.3, 0);
    //test_sound = gfc_sound_load("audio/dink.mp3", 0.3, 1);
    //if (!test_sound) slog("Couldn't load test_sound");

    //J testing Text:
    /*slog("Configuring Test TEXTTTTTTTT");
    testText = text_new();
    text_configure_from_file(testText, "def/text/testText.texty");
    slog("testText configured??");*/
    
    //Sprite* attack = gf2d_sprite_load_all("images/ui/attack_button.png", 180, 100, 2, 0);     this was me trying to draw the attack button.
            // I forgot to include either gfc_config of gf2d_graphics in my window.c file I forget which one. but ONE function wasn't defined and that's why it wasn't working
    
    thePlayer = player_get_the();
    world = world_get_active();
    camera_set_size(gf2d_graphics_get_resolution());  //Feb 26: J added

    //MAKE  the main menu.   (For his 3D game - the second picture.   his gf2d_windows_draw_all();  draws the World as well
       //main_menu_start()
    //Once the game starts   we don't even need the Main Menu

    /*main game loop*/
    gfc_sound_play(music, 2, 0.1, -1, -1);
    slog("\n==============================================================\n");
    while(!done)
    {
        gfc_input_update(); // update SDL's internal event structures   //J NOTE - must be called once a frame. If not called, nothing updates, then none of these conditions will ever set
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        
        _MOUSEBUTTON = SDL_GetMouseState(&mx, &my);  //HAHAAA  MY MOUSEBUTTON VARIABLE WORKS!! YIPPEEE works for clicks AND holds!!  although that being said a human "click" runs like 5 times in this loop lmao
        //This is true if  ANY mouse button is pressed!!

        
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
        if (mouseClickTimer <= 0) { //We can click the mouse (leave it 1). And Reset the cooldown
            mouseClickTimer = 10;
        }
        else if (mouseClickTimer > 0) {  //On cooldown.  Do not register the click
            _MOUSEBUTTON = 0;
            mouseClickTimer--;
        }  
        
        //J TO BE ADDED particles
        /*if (_MOUSEBUTTON) {   //he had  JUST "blood_spray" here.  Because of the loading def thingy he showed us. NO .particle extension
            particles_from_def("spray_water.particle", 100, gfc_vector2d(mx, my), gfc_vector2d(5, 6), gfc_vector2d(2,1) );

            //Yeah lemme try this
            particles_from_def("spray", 100, gfc_vector2d(mx, my), gfc_vector2d(5, 6), gfc_vector2d(2,1) );
        }*/


        //For demonstration:  Level editor 
        //if (_MOUSEBUTTON) { //  he called it "mb", but I alr had this variable
        //    camera = camera_get_position();                 //bc _MB is true for ANY mouse button. We gotta check that it's the LEFT
        //    world_set_tile(world, gfc_vector2d(mx + camera.x, my + camera.y), (_MOUSEBUTTON & SDL_BUTTON(1) ) ? 1 : 0);  //IF the Left mouse button is being pressed, 1. Else 0
        //    
        //}
        if (_START_SCREEN) {
            window = window_search_by_name("Start Screen");
            window_set_active(window);
            mouse_position = gfc_vector2d(mx, my);
            if (mouse_position.x > 435 && mouse_position.x < 765 && mouse_position.y < 553 && mouse_position.y > 387 ) {
                set_selected(1);
                if (_MOUSEBUTTON) {
                    _START_SCREEN = 0;
                    _GAME_STARTED = 1;
                }
            }
            else {
                reset_selected();
            }
        }
        if (_GAME_STARTED) {
            window_set_active(NULL);
            _GAME_STARTED = 0;
        }


        if (!_START_SCREEN) {
            entity_system_think_all();   //THINK FIRST   //J ADDED
            //game_think()

            entity_system_update_all();  //then update shit  //J ADDED

            camera_bounds_check(); //Feb 26: J Added
            //game_update()
            if (_MOUSEBUTTON) {
                //slog("Player current position X: %f  &  Y: %f", thePlayer->position.x, thePlayer->position.y);
                //slog("click detected. Enabling flag to draw the item");
                //_INVENTORY_FLAG = 1;

                slog("Click detected Playing audio");
                //if(test_sound) gfc_sound_play(test_sound, 0, 0.1, -1, -1);

                slog("Click detected. Incrementing all points");
                inc_player_points(ENT_MAX);
            }
        
        }
        
        

        gf2d_graphics_clear_screen();// clears drawing buffers                //J NOTE: ALL YOUR DRAW CALLS must be within Clear and Next_Frame
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first       
            //gf2d_sprite_draw_image(sprite,gfc_vector2d(0,0));    //don't need this background anymore
        
            

            //game_draw();

            if (world) {
                //slog("oh bitch we drawin");
                world = world_get_active();
                world_draw(world);   //J ADDED
                if (_DRAWBOUNDS) world_draw_bounds(world); //J ADDED
            }
            entity_system_draw_all();    //draw shit -- I want my entities to exist in front of the background, but drawn before the mouse  //J TO BE ADDED
            
            //J TO BE ADDED:  particles
            /*particle_system_draw();*/


            //Hellooo  Jia after Break here:   In terms of maybe cleaning up the Main Game loop. (BATTLE)
                // What i could do is, just have a   window_draw_active()  function call here. And this would be the ONLY draw call for ANY UI window (discounting inventory. My god)
                //Whether it be Battle... Loading Screen, Home Screen, Win Screen. Whatever !  and ofc within window.c the active window would be swapped around and used internally as needed
                //2 WEEKS after Break Jia here:    ideally _draw_active() would cycle through ALL the worlds that have the _active   flag on.

    //========================================================

            //draw all items.. ?
            
            //Maybe I should have a extern flag in here? _INBATTLE.  so that I can start drawing the Battle UI when need-be
            //if (_INBATTLE) {                
                //..  basically.  I want a single draw call here:    to Draw the Proper UI_Elements.  While I have them THINK out there ^
                window_draw(window_get_active());
            //}
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

            /*if (_DRAWATTACKNAME) {
                Text* create_text_raw(const char* text, FS_medium, GFC_COLOR_WHITE, gfc_vector2d(600, 100), 1);
            }*/

            if(!_START_SCREEN){
            gf2d_sprite_draw(
                health_bar,
                gfc_vector2d(300, 650),
                NULL,
                &h_center,
                NULL,
                NULL,
                NULL,
                health_frame);
            }

            //SDL_RenderCopy(gf2d_graphics_get_renderer(), Message, NULL, &Message_rect);
            //text_rndr(Message, Message_rect);
            
     //------------ tEXT     

            font_draw("Press 'ESC' to quit", FS_small, GFC_COLOR_WHITE, gfc_vector2d(940,5));
            
            //Works.  no longer testing.
            //text_draw(testText, gfc_vector2d(100,200));

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

    world_free(world);  //My worlds are not maaged by one System. I will not initialize them ALL before the game loads up. I need to free EACH one when needed. soooo somewhere within Encounter.c
    
    //slog( "Button 2's position: %f", but2->position.x);  //just testing to make sure the Button list of a given window was properly accessible
    //window_free(win);
    //TTF_CloseFont(test_font);     //J FONT
    //SDL_FreeSurface(surfaceMessage);
    //SDL_DestroyTexture(Message);

    //TTF_Quit();       //J FONT
    //slog("Text system de-initialized");

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
