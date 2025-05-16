#ifndef __DIALOGUE_H__
#define __DIALOGUE_H__


#include "entity.h"
#include "text.h"



typedef struct {
	Uint8		_inuse;
	int			diaCount;
	GFC_List	*textList;		//yeah nvm forget the double pointer idea.  I don't know well enough how I should terminate my search through a double pointer WITHOUT knowing the number in advance.. like.. variable sized array type shit... scary

}Dialogue;




//ohhh boy it's a resource alright...


void dialogue_init(Uint32 max_dialogue);


void dialogue_free(Dialogue* self);

//Allocates spot on the list
Dialogue* Dialogue_new();



/**
 * @brief	Using the def file of the entity that should have this dialogue:  configure the list of dialogue options
 * @param	ent - pointer to the entity who should own this list of dialogue
 * @return DialogeList object with a NULL list, and a Count of 0, otherwise: return the DialogueList object fully configured
 * @note <Notes for  freeing  or other important things>
*/
//Dialogue dialogue_configure(Entity* ent, SJson* json);		//npc should include dialogue.  And it should be configured in the NPC_new_net function. ... fuck maybe all entities..


void dialogue_configure(Dialogue* self, SJson* json);

/**
 * @brief	<purpose>
 * @param	<param_name> - <what it's for>
 * @return <NULL if yada yada>, <otherwise: return type>
 * @note <Notes for  freeing  or other important things>
*/


#endif