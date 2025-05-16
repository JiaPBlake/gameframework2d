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
Uint8 _INDIALOGUE;
Entity* dragon;		//global pointer to the thing I'm fighting, so I can use it in the Attack functions,  despite the way the Attack functions are called from player_think_battle()  who doesn't have access to the Monster entity.
Entity* play;		//wait nvm,, don't need this.  Attacking buttons should not call a button action function.  That function pointer only accepts a Button as its parameter
Uint8 turn;

Uint8 _DRAWATTACKNAME;

//As a result.  The battle is going to need access to the Player's info.  And the Monster's info
//#include "player.h"
//#include "monster.h"   //I JUST PEEPED I DON'T EVEN NEED THESE

void use_attack(Entity* self);
void dialogue_commence();

void battle_start(Entity* player, Entity* monster) {
	
	slog("BATTLE START");
	_INDIALOGUE = 0;
	reset_selected();
	//player
	_INBATTLE = 1;  //I only want health to decrease ONCE PER collision.  not while I'm colliding
	//self->velocity.x = 0;
	player->think = player_think_battle;
	player->acceleration = gfc_vector2d(0,0);
	player->velocity = gfc_vector2d(0, 0);
	play = player;
	monster->firstCombat = 0;
	dragon = monster;

	GFC_Vector2D newposition = gfc_vector2d(monster->bounds.w*1.5, 0); // Move the player  2* the monster frame size over to the right
	gfc_vector2d_scale(newposition, newposition, -1);
	gfc_vector2d_add(player->position, player->position, newposition);

	window_set_active(window_search_by_name("battle_screen"));

	//Hmm  maybe I should split it into battle_think  and Draw ?  where Draw is essentially my update function  :o
	turn = 1;
}

void player_think_battle(Entity* self) {
	//We should ONLY be in this function if the _INBATTLE flag is on.
	if (!_INBATTLE) {
		slog("Not in battle.  Exiting Battle Think function and setting player think back to normal");
		self->think = player_think;
	}

	//self->position = gfc_vector2d(400, 620);

	UI_Window* window = { 0 };
	UI_Button* button = {0};

	Move* move;

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
			if (_INDIALOGUE) {
				reset_selected();
				window_set_active(window_search_by_name("battle_screen"));
				//slog("Window switch ??");
				_INDIALOGUE = 0;
			}

			//Buttons will have an Attack name 
			
			//Get what type of window is currently up.  If we're still in the main battle window, that means we don't need to attack.
					//i.. I don't really need to do this lmao  but we'll see
			window = window_get_active();
			//But if we're in any of the other windows !   (4 Attacks,  4 Converse, etc...)  then I can perform those actions HERE


			button = get_selected_button(get_selected(), window);
			//oo !   But if I'm attacking.  I don't want window to take care of that.  I just wanna send the attack!!

			if (button) {  //lmfao  just in case get_selected returns NULL.  Don't wanna crash by trying to do button stuff on a non-button
				//Using the button's   actionType.  SET IT'S action  function to point to ATTACK  or CONVERSE
				//slog("Button's actionType is: %i. From window: '%s'", button->actionType, window->name);

				//SETTING the action function  for ALL actionTypes..  so I can just call button->action(button).

				//  BT_NewWindow  is the ONLY Button Type that assigns an action function.  We'll do the rest here.

				if (button->actionType & BT_CloseWindow) {
					//slog("This button would Close a window!");
					button->action = battle_end;
				}
				if (button->actionType & BT_Tame) {
					button->action = dialogue_commence;
					//slog("Button action assigned to DIALOGUE_COMMENCE");
				}
				if (button->actionType & BT_Flee) {
					//slog("This button would Close a window!");
					button->action = battle_end;
				}
				if ((button->actionType & BT_Attack) || (button->actionType & BT_Converse)) {
					button->action = NULL;
				}

				if (button->action) {
					//slog("Performing action");
					button->action(button);
				}
				else if ((button->actionType & BT_Attack) || (button->actionType & BT_Converse)){
					slog("Using attack");
					use_attack(self);
				}
				else {
					slog("Selected button does not have an Action assigned to it. Selected index: %i", get_selected());
				}
			}



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
			reset_selected();
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




void dialogue_commence() {
	_INDIALOGUE = 1;
	//slog("DIALOGUE HERE LMFAO");

	//I would set active window to window_dialogue.
	window_set_active(window_search_by_name("dialogue_screen"));

	//get the move the button is linked to:  A.K.A  just the index!! That's what I have get_selected() for;
	int moveIndex = get_selected();
	Move* playerMove = gfc_list_get_nth(play->move_list, moveIndex);


		//and then I draw the text on top of it. BASED on the Conversation

}




//==============================================		ATTACK SECTION  (and Conversing)

void entity_take_damage(Entity* target, int damage) {
	if (!target) {
		slog("No target provided");
		return;
	}

	if (target->name) {
		slog("Entity with name %s's Health to start is: %i", target->name, target->health);
	}

	if (target->health <= 0) {
		slog("Target has no health");
		return;
	}

	target->health -= damage;
	slog("Entity with name %s's Health is NOW: %i", target->name, target->health);

}

void use_attack(Entity* self) {
	if (!self) {
		slog("No Player passed to function. Cannot use attack");
		return;
	}

	if (!self->move_list) {
		slog("Player does not have a move list... cannot attack");
	}
	Move* player_move = { 0 };
	Move* dragon_move = { 0 };
	int attack_index;
	attack_index = get_selected();


	// ....  on the highest key I might wanna change how this works  ?   so I can use  get_move_by_name() .  wait. wait  nono I stored the ENTIRE move object (pointer)  so that I DIDN'T have to do that

	player_move = gfc_list_get_nth(self->move_list, attack_index);
	if (!player_move) {
		slog("Move not assigned");
		return;
	}
	if (!(player_move->type & MOVET_ATTACK)) {
		slog("This is not an attacking move. Canceling attack");
		return;
	}
	//slog("Heyy! We're good so far! name of the move is: %s", player_move->name);
	
	dialogue_commence();
	

	//Make the animation play:   set flag  _INATTACK  to pause thinking and JUST draw


	//Damage the dragon
	slog("Player used move: '%s'",player_move->name);
	entity_take_damage(dragon, player_move->m.attack.attackPower);



}



