#include "simple_logger.h"

#include "gfc_input.h"		//ohhh  this is included in player. C  NOT H   I need this here lol


//do n o t  include world that might be a circular dependency
#include "battle.h"
//MOVE IS INCLUDED THROUGH ENTITY

#include "player.h"		//I need these to be specific.   I NEED player, not just entity. Because ideally I get access to PlayerEntityData..  I don't want to just stuff EVERYTHING - stats included into the parent Entity struct
#include "monster.h"
#include "window.h" // yeah... don't know how to use any buttons if the Player think function refuses to work in a different file.. (this file)

//Personally I think it makes more sense to include window here in battle,,  well-   actually yeah.  Only if I make all the necessarily button_related functions available in the Header.. which,, I SHOULD. that'd be great



extern int keySelectTimer; //10 frames
extern Uint8 _INBATTLE;

//As a result.  The battle is going to need access to the Player's info.  And the Monster's info
//#include "player.h"
//#include "monster.h"   //I JUST PEEPED I DON'T EVEN NEED THESE

void battle_start(Entity* player, Entity* monster) {
	
	slog("BATTLE START");

	reset_selected();
	//player
	_INBATTLE = 1;  //I only want health to decrease ONCE PER collision.  not while I'm colliding
	//self->velocity.x = 0;
	player->think = player_think_battle;
	player->acceleration = gfc_vector2d(0,0);
	player->velocity = gfc_vector2d(0, 0);
	monster->firstCombat = 0;

	GFC_Vector2D newposition = gfc_vector2d(monster->bounds.w*2, 0); // Move the player  2* the monster frame size over to the right
	gfc_vector2d_scale(newposition, newposition, -1);
	gfc_vector2d_add(player->position, player->position, newposition);

	window_set_active(window_search_by_name("battle_screen_test"));

	//Hmm  maybe I should split it into battle_think  and Draw ?  where Draw is essentially my update function  :o

}

void player_think_battle(Entity* self) {
	//We should ONLY be in this function if the _INBATTLE flag is on.
	if (!_INBATTLE) {
		slog("Not in battle.  Exiting Battle Think function and setting player think back to normal");
		self->think = player_think;
	}

	self->position = gfc_vector2d(400, 620);

	UI_Window* window = { 0 };
	UI_Button* button = {0};
	//Select UI box
	if (gfc_input_command_down("right")) {  //if I'm pressin'  right
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			//slog("Going right");
			inc_selected();
			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;					//just realized this decrementation should be happening regardless of if I'm pressing a key lmfao.  so OUTSIDE the  gfc_input_command  blocks... but it still feels great as it is now
		}
	}
	else if (gfc_input_command_down("left")) {  //if I'm pressin'  left
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			dec_selected();
			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}

	if (gfc_input_command_down("down")) {  //if I'm pressin'  down.  to exit
		//self->velocity.x = 1.0;
		reset_selected();
		_INBATTLE = 0;
		self->think = player_think;
		window_set_active(NULL);		//especially if I'm no longer  ONLY drawing the window IF(_INBATTLE)
	}

	if (gfc_input_command_down("enter")) {  //if I'm pressin'  enter
		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0
			
			
			/*//Get what type of window is currently up.  If we're still in the main battle window, that means we don't need to attack.
					//i.. I don't really need to do this lmao  but we'll see
			window = window_get_active();
			//But if we're in any of the other windows !   (4 Attacks,  4 Converse, etc...)  then I can perform those actions HERE


			button = get_selected_button(get_selected(), window);
			//oo !   But if I'm attacking.  I don't want window to take care of that.  I just wanna send the attack!!


			//Window_configure  sets the action function  IF  a button should change a window.   But otherwise, the rest is up to me, here in battle.c
			if (button->actionType == BT_NewWindow || button->actionType == BT_CloseWindow) {
				button->action(button);  //CALL said action function, which is already set per Window Configure
			}
			else {
				
				slog("Fressed");
				battle_end();
			}
			*/
			slog("Performing action");
			button_perform_action(get_selected(), window_get_active());

			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}

	if (gfc_input_command_down("back")) {

		if (keySelectTimer <= 0) { //Less than or equal to 0  just in case we miss 0

			slog("Trying to back out to last window");
			window_go_back();

			keySelectTimer = 10;
		}
		if (keySelectTimer > 0) {  //ONLY decrement if it's positive.   Just in case
			keySelectTimer--;
		}
	}

	//I  want toooooooo   call  battle_start()   in my Think
			//and battle_end  in THIS think

	//slog("I am at the end of Player BATTLE THINK.  Battle is set to: %i,  and the address of the think funciton is now: %p",_INBATTLE,self->think);
}


void battle_end() {
	//player->think = player_think;
	slog("The value of the _INBATTLE flag is: %i.  Ending Battle", _INBATTLE);
	_INBATTLE = 0;

	window_set_active(NULL);
}