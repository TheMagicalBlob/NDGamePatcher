#include "include.h"

#define T2_FREE 0x02a8f680
#define T2_MALLOC 0x02a8f690


typedef struct dmenu_menu_base dmenu_menu_base_t;
#define T2DMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t *) 0x6b63998
#define T2DMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t *) 0x6b64670
#define T2DMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t *) 0x6b639a0



// Just the function array pointer at the start of each dmenu struct read as an unsigned 64bit integer
typedef enum dmenu_type_ids : u_long
{
    dmenu_id_menu_base = 0x322E688,

    dmenu_id_menu = 0x322E5B0,
    dmenu_id_dyn_menu = 0x31B5248, // slightly larger than normal menus, I think these are ones generated when opened. //! check when headache gone
    dmenu_id_unk_menu = 0x32280B0,

    dmenu_id_submenu_item = 0x322E488,
    dmenu_id_task_submenu_item = 0x3252D58,
    dmenu_id_noreturn_task_submenu_item = 0x3252C30,

    dmenu_id_bool_item = 0x322DA18,
    dmenu_id_exec_item = 0x322E238,
    dmenu_id_alt_exec_item = 0x322E360,
    dmenu_id_string_item = 0x322DFE8,
    dmenu_id_string_list_item = 0x322DC68,
    dmenu_id_float_item = 0x322E110,
    dmenu_id_decimal_item = 0x322DEB8,
    dmenu_id_separator_item = 0x322D6A0,
    dmenu_id_task_history_item = 0x32529E0,
    dmenu_id_task_item = 0x3252B08,
    dmenu_id_user_item = 0x3243BF0, // Entirely useless, maybe it's meant to be replaced by an actual user name? idk
    dmenu_id_timer_item = 0x3117960,
    dmenu_id_unk_item = 0x322DB40

} dmenu_type_ids_t;


// simplified dmenu type enum boiling it down to structurally different items
typedef enum dmenu_item_base_types : char
{
    dmenu_basetype_menu_base, // The base instance containing all menu
    dmenu_basetype_menu, // the menus themselves, whether it be the "Quick Menu" menu or "Rendering" submenu
    // dmenu_basetype_dyn_menu, // generated ones- first item is in a diff spot //! what?
    // dmenu_basetype_unk_menu, // idk
    dmenu_basetype_submenu, // The items that load submenus
    dmenu_basetype_item // anything else

} dmenu_item_base_types_t;


typedef struct dmenu_menu dmenu_menu_t;
typedef struct dmenu_item dmenu_item_t;


typedef struct dmenu_item // Size 0xB8/0x?? (seems to vary, don't think there's one universal dmenu_item struct- needs checking)
{
    void * function_array; // 0x00
    char unused1[0x8]; // 0x08 // Always zero
    char item_text_buffer[0x20]; // 0x10
    char * item_text; // 0x30 // Just Points 32 bytes above? I assume it gets used to redirect strings that are too long for the item_text_buffer
    dmenu_menu_t * parent_menu; // 0x38
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



typedef struct dmenu_menu // 0xE0
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



typedef struct dmenu_menu_base // 0x328
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




int t2_dcall(pid_t pid);

int get_dmenu_entry_base_type(pid_t pid, void * item);
char * get_dmenu_entry_type_as_string(long long type);

dmenu_menu_base_t * load_menu_base(pid_t pid, void * address);
dmenu_menu_t * load_menu(pid_t pid, void * address);
dmenu_item_t * load_item(pid_t pid, void * address);


dmenu_item_t * find_item(pid_t pid, dmenu_menu_base_t * parent, const char * item_name);
dmenu_menu_t * find_menu(pid_t pid, dmenu_menu_base_t * parent, const char * item_name);
void dump_menu_entries(pid_t pid, dmenu_menu_base_t * menu);

dmenu_menu_base_t * load_menu_base(pid_t pid, void * address);
dmenu_menu_t * load_menu(pid_t pid, void * address);
dmenu_item_t * load_item(pid_t pid, void * address);

