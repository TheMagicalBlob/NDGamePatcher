

// Pointer to DMenu vitrual function table for DMenu components
typedef void * dmenu_vptr_t;


// simplified dmenu type enum boiling it down to structurally different items
typedef enum : char
{
    stype_menu_base,    // The base instance containing all submenus
    stype_menu,         // the menus themselves
    stype_itemsubmenu,  // The items that load submenus
    stype_item,         // anything else
    stype_separatorline // The separator lines used for spacing/formatting

} dmenu_stype_t;




// Indexing for the game's existing debug menus
typedef enum : char
{
   menu_index_dev,      // Dev Menu
   menu_index_quick,    // Quick Menu
   menu_index_favorites // Favorites Menu

} dmenu_menu_index_t;




typedef enum : int
{
    INVALID = -1,
    menu,
    unk1,
    unk2,
    toggle_item,
    unk4,
    unk5,
    unk6,
    unk7,
    unk8,
    separator_item

} dmenu_item_eh_type_t;












// DMenu component virtual pointers
typedef struct struct_dmenu_types_s
{
    dmenu_vptr_t menu_base; // The base for each debug menu

    dmenu_vptr_t menu; // The menus themselves which contain individual items
    dmenu_vptr_t dyn_menu; // Menus with dynamic contents that're cleared and repopulated every time they're opened
    dmenu_vptr_t weapon_upgrades_menu;

    dmenu_vptr_t item_submenu;
    dmenu_vptr_t task_item_submenu;
    dmenu_vptr_t subnode_task_item_submenu;

    dmenu_vptr_t base_component; // 0x??
    dmenu_vptr_t separator_item; // 0xA8
    dmenu_vptr_t bool_item; // 0xB8
    dmenu_vptr_t exec_item; // 0xC0
    dmenu_vptr_t alt_exec_item;
    dmenu_vptr_t string_item;
    dmenu_vptr_t string_list_item;
    dmenu_vptr_t decimal_item;
    dmenu_vptr_t task_history_item;
    dmenu_vptr_t task_item;
    dmenu_vptr_t user_item; // Entirely useless, I assume it's meant to be replaced by an actual username
    dmenu_vptr_t timer_item;
    dmenu_vptr_t checkpoint_item;
    dmenu_vptr_t float_item; // 0x158

} dmenu_types_t;







typedef struct dmenu_menu_s dmenu_menu_t;
typedef struct dmenu_item_s dmenu_item_t;

typedef struct dmenu_component_s 
{
    dmenu_vptr_t vptr;
    void * unknown_function_ptr;
    char item_text_buffer[0x20];
    char * item_text;
    dmenu_menu_t * parent_menu;
    dmenu_item_t * next_item;
    const unsigned short horizontal_padding;
    const unsigned short vertical_padding;
    const unsigned short width;
    const unsigned short height;
    char unmapped0[0x8];
    unsigned long unknown_item_index;
    void * item_variable;
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour;
    unsigned int shortcut_info;
    void * callback;
    unsigned long unmapped3;
    char unmapped4[0x8];
    dmenu_menu_base_t * menu_base; // Points to the dmenu base entry [0x90] 

} dmenu_component_t;




typedef struct dmenu_item_s 
{
    dmenu_vptr_t vptr; // Item class vPtr/Virtual Table Pointer
    void * unknown_function_ptr; // 0x08 // Always zero
    char item_text_buffer[0x20]; // 32-byte buffer for the item text. If the string is too long, additional space is allocated instead and this is left unused. (it's also left null for separator lines) // 0x10
    char * item_text; // Item text pointer. Usually points to the item_text_buffer, but is redirected if the string is too long (or left null for separator lines) // 0x30
    dmenu_menu_t * parent_menu; // The dmenu_menu_t that contains this item
    dmenu_item_t * next_item; // The proceeding item in the same submenu
    const unsigned short horizontal_padding; 
    const unsigned short vertical_padding; 
    const unsigned short width; 
    const unsigned short height; 
    char unmapped0[0x8];
    unsigned long unknown_item_index;
    void * item_variable;
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour;
    unsigned int shortcut_info;
    void * callback;
    unsigned long unmapped3;
    char unmapped4[0x8];
    dmenu_menu_base_t * menu_base; // Points to the dmenu base entry [0x90] 
    char unmapped5[0x4];
    int selection_index;
    unsigned int item_count; //!
    unsigned int event_handler_type_id; // unsure
    void * event_handler;
    unsigned long return_buffer;

} dmenu_item_t;







typedef struct dmenu_menu_s // 0xE0
{
    dmenu_vptr_t vptr; // Item class vPtr/Virtual Table Pointer
    void * unknown_function_ptr; // 0x08 // Always zero
    char menu_name_buffer[0x20]; // Default buffer space for the item text, if it fits the buffer. 
    char * menu_name; // Pointer to the item's text; usually points to the item_text_buffer, but additional space is allocated and used instead if the string length is > 32. 
    dmenu_menu_t * parent_menu;
    dmenu_item_t * next_item; // Next item in the regular entries. seems to still be read here, though
    const unsigned short horizontal_padding;
    const unsigned short vertical_padding;
    const unsigned short width;
    const unsigned short height;
    char unmapped0[0x8];
    unsigned long unknown_menu_array_index;
    void * menu_variable;
    unsigned int on_state_colour;
    unsigned int neutral_state_colour;
    unsigned int shortcut_info;
    void * callback;
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
    dmenu_vptr_t vptr; // Item class vPtr/Virtual Table Pointer
    char unused1[0x8]; // Always zero
    char menu_base_name_buffer[0x20]; // Default buffer space for the menu base name, if it fits the buffer. 
    char * name; // Pointer to the menu base's text; usually points to the menu_base_name_buffer, but additional space is allocated and used instead if the string length is > 32. 
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












dmenu_stype_t get_dmenu_entry_base_type(void * item);
const char * get_dmenu_entry_type_as_string(dmenu_vptr_t type);


dmenu_item_t * find_item(const char * item_name, dmenu_menu_index_t parent_index);
dmenu_menu_t * find_menu(const char * menu_name, dmenu_menu_index_t parent_index);
dmenu_item_t * find_submenu(const char * submenu_name, dmenu_menu_index_t parent_index);

dmenu_menu_t * get_root_menu(dmenu_menu_index_t menu_base_index);



int load_menu_base(void * address, dmenu_menu_base_t * menu_base);
int load_menu(void * address, dmenu_menu_t * menu);
int load_item(void * address, dmenu_item_t * item);

void dump_menu_entries();

int AppendItemToParent(dmenu_menu_t * parent_menu, dmenu_item_t * item);

int add_novis_option(dmenu_menu_index_t menu_base);
