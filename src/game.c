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
#include "items.h" //J TESTING (should be good honestly
#include "text.h" //J TESTING.  Works as a basic thing. Gotta optimize and move things around         **ALSO INCLUDED IN WINDOW --> UI
#include "particles.h"  //J TO BE TESTED (CLASS)



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
    //J TO BE ADDED (AUDIO):
   /* gfc_audio_init(
        1000,
        128,
        4,
        1,
        1,
        1);*/
        
    gf2d_sprite_init(1024);
    entity_system_init( 100 );    //J ADDED - initalize our Entity system AFTER the sprite system.  Since it depends on the sprites
    
    //font_init();          //J TO BE ADDED (FONTS)  please delete the other fonts down below
    // YEOOO IMPORTANT:   since I separated  font_init() and text_init()  I wonder what the order of operations is there.
        //For example, is what I have in text.c equivalent to calling font_init()  THEN text_init() on the next line??
    text_init(50);  //J CURRENTLY TESTING


    //button_system_init(20);
    ui_system_init(100);
    window_system_init(10);
    items_initialize("def/items.def");
  
    //uncomment this once your Windows are in a working state

    //particle_system_init(1000);  //J TO BE ADDED particles
    ////what the FUCK
    //gfc_config_def_init();
    //gfc_config_def_load("config/particleList.cfg");   //Cheks what the resource nam



    SDL_ShowCursor(SDL_DISABLE);

    //TTF_Init();          //J ADDED
    //if (TTF_WasInit() == 0) slog("Text not initialized"); else slog("TTF system intialized");

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
    world = world_load("def/levels/testLevel.level"); /*world_test_new();*/
    battle_alert = gf2d_sprite_load_all("images/border.png", 605, 74, 1, 0);
    health_bar = gf2d_sprite_load_all("images/healthbar_full.png", 320, 64, 6, 0);
    
    GFC_Vector2D h_center = gfc_vector2d(160, 32);
    //stats_screen = ui_make();     //J TO BE ADDED (UI)
    
    UI_Window* starting_win = window_new();
    window_configure_from_file(starting_win, "def/ui/testWindow.def");
    

    //J testing text:
    /*slog("Configuring Test TEXTTTTTTTT");
    testText = text_new();
    text_configure_from_file(testText, "def/text/testText.texty");
    slog("testText configured??");*/
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

    //MAKE  the main menu.   (For his 3D game - the second picture.   his gf2d_windows_draw_all();  draws the World as well
       //main_menu_start()
    //Once the game starts   we don't even need the Main Menu

    /*main game loop*/
    while(!done)
    {
        //world = world_get_active();
        //_CHANCE1++;
        gfc_input_update(); // update SDL's internal event structures   //J NOTE - must be called once a frame. If not called, nothing updates, then none of these conditions will ever set
        
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        
        cache_cleanup(); //to periodically clean up out fonts  //J ADDED (TEXT)
        _MOUSEBUTTON = SDL_GetMouseState(&mx,&my);  //HAHAAA  MY MOUSEBUTTON VARIABLE WORKS!! YIPPEEE works for clicks AND holds!!  although that being said a human "click" runs like 5 times in this loop lmao
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
            //slog("click detected. Enabling flag to draw the item");
            //_INVENTORY_FLAG = 1;
            slog("Click detected. Incrementing all points");
            inc_player_points(ENT_MAX);
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
            
            //J TO BE ADDED:  particles
            /*particle_system_draw();*/


            //Hellooo  Jia after Break here:   In terms of maybe cleaning up the Main Game loop. (BATTLE)
                // What i could do is, just have a   window_draw_active()  function call here. And this would be the ONLY draw call for ANY UI window (discounting inventory. My god)
                //Whether it be Battle... Loading Screen, Home Screen, Win Screen. Whatever !  and ofc within window.c the active window would be swapped around and used internally as needed
                //2 WEEKS after Break Jia here:    ideally _draw_active() would cycle through ALL the worlds that have the _active   flag on.

    //========================================================

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
            

            //draw_stats(stats_screen, health_frame);  //J TO BE ADDED (UI)
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
