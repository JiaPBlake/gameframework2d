#ifndef __INVENTORY_H__
#define __INVENTORY_H_

#include "simple_json.h"

#include "gfc_list.h"
#include "items.h"

typedef enum { //because I can count the amount of numbers I have on 2 hands, enumeration would be a shout
	II_Arti,
	II_Drum,
	II_Orb,
	II_Stone,
	II_Trident,
	II_MAX

}ItemIndices;

typedef struct {
	GFC_List		*itemsList; //I'll only have some.. 7 or 8 items in my game

}Inventory;

void inventory_init(Inventory* inventory);

void inventory_cleanup(Inventory* inventory);

Item* inventory_get_item_by_name(Inventory* inventory, const char* name);


void inventory_add_item(Inventory* inventory, const char* name);

void inventory_remove_item(Inventory* inventory, const char* name);

//To be used in player.c  which only has access to inventory
void display_item(Inventory* inventory, const char* name);

#endif