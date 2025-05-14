#ifndef __DIALOGUE_H__
#define __DIALOGUE_H__


#include "entity.h"
#include "text.h"



typedef struct {
	int		diacount;
	GFC_List diaList;		//pointer ?	List of Text objects that act as a single instance of dialogue.   hm..  or should they be GFC_TextLines.. and I render them AS Text Objects somewhere else..?

}DialogueList;

/**
 * @brief	Using the def file of the entity that should have this dialogue:  configure the list of dialogue options
 * @param	ent - pointer to the entity who should own this list of dialogue
 * @return DialogeList object with a NULL list, and a Count of 0, otherwise: return the DialogueList object fully configured
 * @note <Notes for  freeing  or other important things>
*/
DialogueList dialogue_list_configure(Entity* ent, SJson* json);		//npc should include dialogue.  And it should be configured in the NPC_new_net function. ... fuck maybe all entities..


/**
 * @brief	<purpose>
 * @param	<param_name> - <what it's for>
 * @return <NULL if yada yada>, <otherwise: return type>
 * @note <Notes for  freeing  or other important things>
*/


#endif