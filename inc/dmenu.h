#include "include.h"



typedef struct dmenu_menu dmenu_menu_t;
typedef struct dmenu_menu_base dmenu_menu_base_t;
typedef struct dmenu_item dmenu_item_t;



typedef struct dmenu_item { // Size 0xB8/0x?? (seems to vary, don't think there's one universal dmenu_item struct- needs checking)
    void * FunctionArray;
    unsigned long unused1; // Always zero
    char item_text_buffer[0x20];
    char * item_text; // Just Points 32 bytes above? I assume it gets used to redirect strings that are too long for the item_text_buffer
    dmenu_menu_t * parent_menu;
    dmenu_item_t * next_item;
    char item_info[8]; // Changes per-item, but not sure what it is yet. first byte seems to be the item type, though (ONLY SET WHEN MENU IS OPEN)
    char unknown2[0x18];
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour;
    unsigned int padding;
    char unknown3[0x18];

} dmenu_item_t;



typedef struct dmenu_menu // Size = 0xE0
{
    void * FunctionArray;
    char unused1[0x8]; // Always zero
    char item_text_buffer[0x20]; // Only used if the item text is less than 32 bytes, otherwise the game allocates additional space and uses that instead
    char * item_text; // Just Points 32 bytes above? I assume it gets used to redirect strings that are too long for the item_text_buffer
    char unknown1[0x58]; // Technically the colour settings are in here, but I haven't seen whether they're used yet 
    dmenu_menu_base_t * menu_base; // Points to the dmenu base entry [0x90] 
    char unknown2[0x4];
    int selection_index;
    unsigned long item_count; // The amount of items in the root of the menu
    char unknown3[0xB];
    dmenu_item_t * first_item; // Points to the first/top item in the menu [0xB0]
    char unknown4[0x28];

} dmenu_menu_t;



typedef struct dmenu_menu_base // 0x328 total size
{
    void * FunctionArray;
    char unused1[0x8]; // Always zero
    char item_text_buffer[0x20];
    char * item_text; // Just Points 32 bytes above? I assume it gets used to redirect strings that are too long for the item_text_buffer
    char unused2[0x20]; // Always zero
    char unmapped[0x10]; // Unmapped
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour; // Not actually sure about this one, gotta check
    unsigned int padding;
    char unmapped2[0x30]; // Unmapped
    dmenu_menu_t * menu; // 0xA8
    unsigned long * UnknownPointer;
    char UnknownBytes1[0x8];
    char UnknownBytes2[0x8];
    char unused3[0x260]; // Why on earth does it allocate so much space for the struct???

} dmenu_menu_base_t;
