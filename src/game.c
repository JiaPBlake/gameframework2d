#include <SDL.h>
#include "simple_logger.h"

#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "player.h"
#include "monster.h"

//You're gonna wanna include  entity.c  right around here.  And then also player.h

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    Sprite *sprite;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(100,100,100,200);//J CHANGE:  Changing the Cursor color / Pointer color.  Used to 255, 100, 255, 200
    Entity* player;   //J START  
    Entity* monster;
    GFC_Vector2D player_position;
    player_position.x = 0;
    player_position.y = 0;
    GFC_Vector2D monster_position = gfc_vector2d(600, 400);


    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gfc_input_init("config/my_input.cfg");  //he added this  AHHH this is the funciton we use to initalize our inputs  a.k.a our keybinds!!
    //That being said--  there's already a SAMPLE confic within the gfc folder: gameframework2d\gfc\sample_config
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    entity_system_init( 50 );    //J ADD:  initalize our Entity system AFTER the sprite system.  Since it depends on the sprites
    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    sprite = gf2d_sprite_load_image("images/backgrounds/dest_bg.png");  //J CHANGE:  used to be "images/backgrounds/bg_flat.png"
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    slog("press [escape] to quit");
    
    player = player_new_entity(player_position);
    monster = monster_new_entity(monster_position);

    /*main game loop*/
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures       //J NOTE - must be called once a frame. If not called, nothing updates, then none of these conditions will ever set
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
            
        entity_system_think_all();   //THINK FIRST   //J TO BE ADDED
        entity_system_update_all();  //then update shit  //J TO BE ADDED

        gf2d_graphics_clear_screen();// clears drawing buffers          //J NOTE: ALL YOUR DRAW CALLS must be within Clear and Next_Frame
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            gf2d_sprite_draw_image(sprite,gfc_vector2d(0,0));
            
            entity_system_draw_all();    //draw shit -- I want my entities to exist in front of the background, but drawn before the mouse  //J TO BE ADDED
            
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
    entity_free(player); //J ADDED
    entity_free(monster);
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
