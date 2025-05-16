#ifndef __MAINMENU_H__
#define __MAINMENU_H__

//I want main menu to be the root of all things.  THIS is going to call everything.  NOTHING should inlcude this


/**
 * @brief	<purpose>
 * @param	<param_name> - <what it's for>
 * @return <NULL if yada yada>, <otherwise: return type>
 * @note <Notes for  freeing  or other important things>
*/


/**
 * @brief	First screen players see when the game opens.   Is mainly just a UI_Window that can be interacted with by user controls / cursor movement
 * @param	<param_name> - <what it's for>
 * @return <NULL if yada yada>, <otherwise: return type>
 * @note <Notes for  freeing  or other important things>
*/
void start_screen();


void initialize_game_state();


void game_draw();


#endif