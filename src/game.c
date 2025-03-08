#include <SDL.h>
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


Uint8 _DRAWBOUNDS = 1;          //.... I need to think of a better way to implement these paths but.  Later..
//Uint8 _CHANCE1 = 1;        //A change flag (or 3)  that will be initialized as some number,  and will increment repeatedly  (from 0 to 9) as the game loop goes
//extern Uint8 _INVENTORY_FLAG = 0;
extern Uint8 _INBATTLE;
Entity* otherEnt;
Uint32 _MOUSEBUTTON;
Uint8 _PRESSED = 0;

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
    Sprite *battle_alert;
    //Uint32 mouseButton;  //making this a global flag instead to let any file see the Mouse state

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
    /*ui_system_init( 100 )*/     //J TO BE ADDED
    SDL_ShowCursor(SDL_DISABLE);
    
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
        _MOUSEBUTTON = SDL_GetMouseState(&mx,&my);  //HAHAAA  MY MOUSEBUTTON VARIABLE WORKS!! YIPPEEE works for clicks AND holds!!  although that being said a human "click" runs like 5 times in this loop lmao
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
            
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
        if (_MOUSEBUTTON) { slog("H"); }

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
            
            //Maybe I should have a extern flag in here? _INBATTLE.  so that I can start drawing the Battle UI when need-be
            if (_INBATTLE) {
                //slog("in battle");
                
                //..  basically.  I want a single draw call here:    to Draw the Proper UI_Elements.  While I have them THINK out there ^
                GFC_Vector2D center = gfc_vector2d(302, 67);
                gf2d_sprite_draw(
                    battle_alert,
                    gfc_vector2d(600,100),
                    NULL,
                    &center,
                    NULL,
                    NULL,
                    NULL,
                    0);
            }

            /*same thing for showing hte inventory at the push of a button I suppose
            if (_INVENTORY_FLAG) {
                slog("Inventory should actively be on screen");
            }*/



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
    world_free(world);  //actually- lied my ass off. my worlds are not maaged by one System. I will not initialize them ALL before the game loads up. I need to free EACH one when needed. soooo somewhere within Encounter.c

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
