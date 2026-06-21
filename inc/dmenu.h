#include "include.h"

#define T2_FREE 0x02a8f680
#define T2_MALLOC 0x02a8f690


typedef struct dmenu_menu_base dmenu_menu_base_t;
#define DMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t *) 0x6b63998
#define DMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t *) 0x6b64670
#define DMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t *) 0x6b639a0


#define T_DMENU_MENU_BASE


enum dmenu_type_ids : u_long
{
    menu_base = 52618888,
    menu = 52618672,
    dyn_menu = 52122184, // slightly larger than normal menus, I think these are ones generated when opened. //! check when headache gone
    unk1_menu = 52592816,

    submenu_item = 52618376,
    
    bool_item = 52615704,
    exec_item = 52617784,
    alt_exec_item = 52618080,
    string_item = 52617192,
    string_list_item = 52616296,
    float_item = 52617488,
    decimal_item = 52616888,
    separator_item = 52614816
};


enum dmenu_item_types : int
{
    dmenu_base, // The base instance containing all menu
    dmenu_menu, // the menus themselves, whether it be the "Quick Menu" menu or "Rendering" submenu
    dmenu_dyn_menu, // generated ones- first item is in a diff spot
    dmenu_unk1_menu, // idk
    dmenu_submenu, // The items that load submenus
    dmenu_item // anything else
};


typedef struct dmenu_menu dmenu_menu_t;
typedef struct dmenu_item dmenu_item_t;


typedef struct dmenu_item // Size 0xB8/0x?? (seems to vary, don't think there's one universal dmenu_item struct- needs checking)
{
    void * function_array;
    char unused1[0x8]; // Always zero
    char item_text_buffer[0x20];
    char * item_text; // Just Points 32 bytes above? I assume it gets used to redirect strings that are too long for the item_text_buffer
    dmenu_menu_t * parent_menu;
    dmenu_item_t * next_item;
    const unsigned short horizontal_padding; 
    const unsigned short vertical_padding; 
    const unsigned short width; 
    const unsigned short height; 
    char unknown0[0x10];
    unsigned long unknown_struct_pointer;
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int unknown_state_colour_1;
    unsigned int unknown_state_colour_2;
    char unknown2[0x18];
    dmenu_menu_base_t * menu_base; // Points to the dmenu base entry [0x90] 
    char unknown3[0x4];
    int selection_index;
    unsigned int item_count; // The amount of items in the root of the menu
    unsigned int entry_type_id_maybe; // The amount of items in the root of the menu
    void * event_handler;
    unsigned long item_variable_buffer; // Bool for options that can be toggled, points to the header item for submenu entry items
    char padding1[0x10];

} dmenu_item_t;



typedef struct dmenu_menu // Size = 0xE0
{
    void * function_array;
    char unused1[0x8]; // Always zero
    char item_text_buffer[0x20]; // Only used if the item text is less than 32 bytes, otherwise the game allocates additional space and uses that instead
    char * item_text; // Just Points 32 bytes above? I assume it gets used to redirect strings that are too long for the item_text_buffer
    dmenu_menu_t * parent_menu;
    const char * next_item; // Next item in the regular entries. seems to still be read here, though
    const unsigned short horizontal_padding;
    const unsigned short vertical_padding; 
    const unsigned short width; 
    const unsigned short height; 
    char unknown1[0x10];
    unsigned long unknown_struct_pointer;
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int unknown_state_colour_1;
    unsigned int unknown_state_colour_2;
    char unknown2[0x18];
    dmenu_menu_base_t * menu_base; // Points to the dmenu base entry [0x90] 
    char unknown3[0x4];
    int selection_index;
    unsigned int item_count; // The amount of items in the root of the menu
    unsigned int entry_type_id_maybe; // The amount of items in the root of the menu
    int menu_closed_by_other_menu;
    const char alignment1[0x4];
    dmenu_item_t * first_item; // Points to the first/top item in the menu [0xB0]
    char unknown4[0x4];
    int menu_is_open;
    char unmapped2[0x8];
    char unmapped3[0x4];
    int on_screen_item_count;
    char unknown5[0x10];

} dmenu_menu_t;



typedef struct dmenu_menu_base // 0x328 total size
{
    void * function_array;
    char unused1[0x8]; // Always zero
    char item_text_buffer[0x20];
    char * item_text; // Just Points 32 bytes above? I assume it gets used to redirect strings that are too long for the item_text_buffer
    char unused2[0x30]; // Always zero
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int unknown_state_colour_1; // Not actually sure about this one, gotta check
    unsigned int unknown_state_colour_2;
    char unmapped2[0x30]; // Unmapped
    dmenu_menu_t * menu; // 0xA8
    unsigned long unknown_struct_pointer; // Menu Options, maybe?
    char open_menu;
    char draw_menu;
    char unused4[0x6];
    unsigned long unknown_long; // Gets changed every time the menu is opened or close, but it's different every time and doesn't look like a pointer //! check ghidra
    char unused3[0x240]; // Why on earth does it allocate so much space for the struct???
    char unmapped3[0x8];
    void * unknown_struct_pointer1;
    char unmapped4[0x8];
    void * unknown_struct_pointer2;

} dmenu_menu_base_t;
