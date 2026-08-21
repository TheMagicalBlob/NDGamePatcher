#include "include.h"




//#
//## The Last of Us Part II Remastered / T1x
//#
typedef struct dmenu_menu_base_s dmenu_menu_base_t;
#define T2RDMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6b63998
#define T2RDMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6b64670
#define T2RDMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6b639a0


#define T2R_FREE (void *) 0x02a8f680
#define T2R_MALLOC (void *) 0x02a8f690

#define T2R_NOVIS (int8_t *) 0x70cc124
#define T2R_LOOKUPSYMBOL (void *) 0x051c0c0








//#
//## The Last of Us Part I / T1x
//#
#define T1XDMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6a34a68
#define T1XDMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6a35720
#define T1XDMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6a34a70

#define T1X_FREE (void *) 0x30ed250
#define T1X_MALLOC (void *) 0x30ed260

#define T1X_NOVIS (int8_t *) 0x3a10794
#define T1X_LOOKUPSYMBOL (void *) 0x9a9150 //! TODO: REPLACE ME!!!





// DMenu component virtual pointers
typedef enum : u_long
{
    // The Last of Us Part II Remastered
    t2r_vptr_menu_base = 0x322E688,

    t2r_vptr_menu = 0x322E5B0,
    t2r_vptr_dyn_menu = 0x31B5248, // slightly larger than normal menus, I think these are ones generated when opened. //! check when headache gone
    t2r_vptr_weapon_upgrades_menu = 0x32280B0,

    t2r_vptr_submenu_item = 0x322E488,
    t2r_vptr_task_submenu_item = 0x3252D58,
    t2r_vptr_noreturn_task_submenu_item = 0x3252C30,

    t2r_vptr_bool_item = 0x322DA18,
    t2r_vptr_exec_item = 0x322E238,
    t2r_vptr_alt_exec_item = 0x322E360,
    t2r_vptr_string_item = 0x322DFE8,
    t2r_vptr_string_list_item = 0x322DC68,
    t2r_vptr_float_item = 0x322E110,
    t2r_vptr_decimal_item = 0x322DEB8,
    t2r_vptr_separator_item = 0x322D6A0,
    t2r_vptr_task_history_item = 0x32529E0,
    t2r_vptr_task_item = 0x3252B08,
    t2r_vptr_user_item = 0x3243BF0, // Entirely useless, maybe it's meant to be replaced by an actual user name? idk
    t2r_vptr_timer_item = 0x3117960,
    t2r_vptr_checkpoint_item = 0x322DB40,






    // The Last of Us Remake / The Last of Us Part I
    t1x_vptr_menu_base = 0x3938C78,

    t1x_vptr_menu = 0x3938BA8,
    t1x_vptr_dyn_menu = 0x38C40B8,
    t1x_vptr_weapon_upgrades_menu = 0x3932BB0,

    t1x_vptr_submenu_item = 0x3938A90,
    t1x_vptr_task_submenu_item = 0x39611F8,
    t1x_vptr_subtask_submenu_item = 0x39610E0,

    t1x_vptr_bool_item = 0x39380B0,
    t1x_vptr_exec_item = 0x3938860,
    t1x_vptr_alt_exec_item = 0x3938978,
    t1x_vptr_string_item = 0x3938630,
    t1x_vptr_string_list_item = 0x39382E0,
    t1x_vptr_float_item = 0x3938748,
    t1x_vptr_decimal_item = 0x3938510,
    t1x_vptr_separator_item = 0x3937D68,
    t1x_vptr_task_history_item = 0x3960EB0,
    t1x_vptr_task_item = 0x3960FC8,
    t1x_vptr_user_item = 0x39507E8,
    t1x_vptr_timer_item = 0x381C180,
    t1x_vptr_checkpoint_item = 0x39381C8,





    
    // Uncharted 4: A Thief's End Remastered
    uc4r_vptr_menu_base = 0xFFFFFFFFFFFFFFFF,

    uc4rx_vptr_menu = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_id_dyn_menu = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_id_unk_menu = 0xFFFFFFFFFFFFFFFF,
    
    uc4r_vptr_submenu_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_task_submenu_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_multiplayer_task_submenu_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_survival_task_submenu_item = 0xFFFFFFFFFFFFFFFF,
    
    uc4r_vptr_bool_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_exec_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_alt_exec_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_string_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_string_list_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_float_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_decimal_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_separator_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_task_history_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_task_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_user_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_timer_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_checkpoint_item = 0xFFFFFFFFFFFFFFFF,





    
    // Uncharted: The Lost Legacy Remastered
    tllr_vptr_menu_base = 0xFFFFFFFFFFFFFFFF,

    tllrx_vptr_menu = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_id_dyn_menu = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_id_unk_menu = 0xFFFFFFFFFFFFFFFF,
    
    tllr_vptr_submenu_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_task_submenu_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_noreturn_task_submenu_item = 0xFFFFFFFFFFFFFFFF,
    
    tllr_vptr_bool_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_exec_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_alt_exec_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_string_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_string_list_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_float_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_decimal_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_separator_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_task_history_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_task_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_user_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_timer_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_unk_item = 0xFFFFFFFFFFFFFFFF

} dmenu_vptr_t;




// simplified dmenu type enum boiling it down to structurally different items
typedef enum : char
{
    stype_menu_base, // The base instance containing all submenus
    stype_menu, // the menus themselves
    stype_submenu, // The items that load submenus
    stype_item // anything else

} dmenu_stype_t;




typedef enum : char
{
   menu_index_dev,
   menu_index_quick,
   menu_index_favorites

} dmenu_menu_index_t;









typedef struct dmenu_menu_s dmenu_menu_t;
typedef struct dmenu_item_s dmenu_item_t;

typedef struct dmenu_item_s // Size 0xB8 - 0xC8 depending on which derived class it is. do this properly later 
{
    void * vptr; // Item class vPtr/Virtual Table Pointer
    char unused1[0x8]; // 0x08 // Always zero
    char item_text_buffer[0x20]; // 32-byte buffer for the item text. If the string is too long, additional space is allocated instead and this is left unused. (it's also left null for separator lines) // 0x10
    char * item_text; // Item text pointer. Usually points to the item_text_buffer, but is redirected if the string is too long (or left null for separator lines) // 0x30
    dmenu_menu_t * parent_menu; // 0x38
    dmenu_item_t * next_item;
    const unsigned short horizontal_padding; 
    const unsigned short vertical_padding; 
    const unsigned short width; 
    const unsigned short height; 
    char unmapped0[0x8];
    unsigned long unmapped1;
    void * item_variable;
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour;
    unsigned int shortcut_info;
    char unmapped2[0x8];
    unsigned long unmapped3;
    char unmapped4[0x8];
    dmenu_menu_base_t * menu_base; // Points to the dmenu base entry [0x90] 
    char unmapped5[0x4];
    int selection_index;
    unsigned int item_count; //!
    unsigned int entry_event_handler_type_id; // unsure
    void * event_handler;
    unsigned long return_buffer; //!
    char padding1[0x10];

} dmenu_item_t;






typedef struct dmenu_menu_s // 0xE0
{
    void * vptr; // Item class vPtr/Virtual Table Pointer
    char unused1[0x8]; // Always zero
    char menu_name_buffer[0x20]; // Default buffer space for the item text, if it fits the buffer. 
    char * menu_name; // Pointer to the item's text; usually points to the item_text_buffer, but additional space is allocated and used instead if the string length is > 32. 
    dmenu_menu_t * parent_menu;
    dmenu_item_t * next_item; // Next item in the regular entries. seems to still be read here, though
    const unsigned short horizontal_padding;
    const unsigned short vertical_padding;
    const unsigned short width;
    const unsigned short height;
    char unmapped0[0x8];
    unsigned long unmapped1;
    void * unknown_struct_pointer;
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour;
    unsigned int shortcut_info;
    void * event_handler;
    unsigned long unmapped2;
    char unmapped3[0x8];
    dmenu_menu_base_t * menu_base; // Points to the dmenu base entry [0x90] 
    unsigned int unmapped4;
    int selection_index;
    unsigned int item_count; // The amount of items in the root of the menu
    unsigned int unk_filter_related_int; // Gets set to zero when removing all filter items
    char unknown_bool; // not quite sure exactly what it does yet. I've only got guesses
    const char unmapped5[0x7];
    dmenu_item_t * first_item; // Points to the first/top item in the menu [0xB0]
    unsigned int unmapped6;
    char menu_is_open;
    char unmapped7[0x3];
    unsigned long _filter_contents; // Something filter-related
    unsigned int unmapped8;
    int on_screen_item_count;
    unsigned long unmapped9;
    char unmapped10[0x4];
    void * unk_stack_thing;

} dmenu_menu_t;



typedef struct dmenu_menu_base_s // x >= 0x328
{
    void * vptr; // Item class vPtr/Virtual Table Pointer
    char unused1[0x8]; // Always zero
    char menu_base_name_buffer[0x20]; // Default buffer space for the menu base name, if it fits the buffer. 
    char * menu_base_name; // Pointer to the menu base's text; usually points to the menu_base_name_buffer, but additional space is allocated and used instead if the string length is > 32. 
    char unused2[0x30]; // Always zero
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour;
    unsigned int shortcut_info;
    char unmapped1[0x30]; // Unmapped
    dmenu_menu_t * root_menu; // The actual dmenu menu that gets populated with items and acts as this menu base's root submenu // 0xA8
    void * unknown_struct_pointer1; // Menu Options, maybe?
    char open_menu; // Open the menu
    char render_menu; // Render the menu when open
    char unmapped2[0x6];
    unsigned long unknown_long; // Gets changed every time the menu is opened or close, but it's different every time and doesn't look like a pointer //! check ghidra
    char unmapped3[0x240]; // Why on earth does it allocate so much space for the struct???
    unsigned int unk_count_ptr;
    void * unknown_struct_pointer2;
    char unmapped5[0x8];
    void * unknown_struct_pointer3;

} dmenu_menu_base_t;












int get_dmenu_entry_base_type(ndgame_t * ndgame, void * item);
char * get_dmenu_entry_type_as_string(long long type);


dmenu_item_t * find_item(ndgame_t * ndgame, const char * item_name, dmenu_menu_index_t parent_index);
dmenu_menu_t * find_menu(ndgame_t * ndgame, const char * menu_name, dmenu_menu_index_t parent_index);
dmenu_item_t * find_submenu(ndgame_t * ndgame, const char * submenu_name, dmenu_menu_index_t parent_index);

dmenu_menu_t * get_root_menu(ndgame_t * ndgame, dmenu_menu_index_t menu_base_index);



int load_menu_base(ndgame_t * ndgame, void * address, dmenu_menu_base_t * menu_base);
int load_menu(ndgame_t * ndgame, void * address, dmenu_menu_t * menu);
int load_item(ndgame_t * ndgame, void * address, dmenu_item_t * item);

void dump_menu_entries(ndgame_t * nd_game, dmenu_menu_base_t * menu);

int AppendItemToParent(ndgame_t * ndgame, dmenu_menu_t * parent_menu, dmenu_item_t * item);

int add_novis_option(ndgame_t * ndgame, dmenu_menu_index_t menu_base);
