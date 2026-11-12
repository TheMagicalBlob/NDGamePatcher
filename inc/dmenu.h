
//#
//## DMenu Structs and Enums
//#



// Pointer to DMenu vitrual function table for DMenu components
typedef void * dmenu_vptr_t;


// simplified dmenu type enum boiling it down to structurally different items
typedef enum DMENU_stype_e : char
{
    stype_menu_group,    // The base instance containing all submenus
    stype_menu,         // the menus themselves
    stype_itemsubmenu,  // The items that load submenus
    stype_item,         // anything else
    stype_separatorline // The separator lines used for spacing/formatting

} DMENU_stype_t;




// Indexing for the game's existing debug menus
typedef enum DMENU_menu_index_e : char
{
   menu_index_dev,      // Dev Menu
   menu_index_quick,    // Quick Menu
   menu_index_favorites // Favorites Menu

} DMENU_menu_index_t;




typedef enum DMENU_item_eh_type_e : unsigned
{
    DMENU_HandlerType_INVALID = __INT_MAX__,
    DMENU_HandlerType_ItemSubmenu = 0,
    DMENU_HandlerType_FloatItem,
    DMENU_HandlerType_DecimalItem,
    DMENU_HandlerType_ToggleItem,
    DMENU_HandlerType_StringListItem,
    DMENU_HandlerType_ExecItem,
    DMENU_HandlerType_UserItem,
    DMENU_HandlerType_StringItem,
    DMENU_HandlerType_UnknownItem_8,
    DMENU_HandlerType_SeparatorItem,
    DMENU_HandlerType_UnknownItem_10

} DMENU_item_eh_type_t;



// DMenu component virtual pointers
typedef struct DMenuTypes_s
{
    dmenu_vptr_t DMENU_Types_MenuGroup; // The base for each debug menu
    dmenu_vptr_t DMENU_Types_Menu; // The menus themselves which contain individual items
    dmenu_vptr_t DMENU_Types_DynMenu; // Menus with dynamic contents that're cleared and repopulated every time they're opened
    dmenu_vptr_t DMENU_Types_WeaponUpgradesMenu;
    dmenu_vptr_t DMENU_Types_ItemSubmenu;
    dmenu_vptr_t DMENU_Types_TaskItemSubmenu;
    dmenu_vptr_t DMENU_Types_SubnodeTaskItemSubmenu;
    dmenu_vptr_t DMENU_Types_BaseComponent; // 0x??
    dmenu_vptr_t DMENU_Types_SeparatorItem; // 0xA8
    dmenu_vptr_t DMENU_Types_ToggleItem; // 0xB8
    dmenu_vptr_t DMENU_Types_ExecItem; // 0xC0
    dmenu_vptr_t DMENU_Types_AltExecItem;
    dmenu_vptr_t DMENU_Types_StringItem;
    dmenu_vptr_t DMENU_Types_StringListItem;
    dmenu_vptr_t DMENU_Types_DecimalItem;
    dmenu_vptr_t DMENU_Types_TaskHistoryItem;
    dmenu_vptr_t DMENU_Types_TaskItem;
    dmenu_vptr_t DMENU_Types_UserItem; // Entirely useless, I assume it's meant to be replaced by an actual username
    dmenu_vptr_t DMENU_Types_TimerItem;
    dmenu_vptr_t DMENU_Types_CheckpointItem;
    dmenu_vptr_t DMENU_Types_FloatItem; // 0x158

} DMENU_Types_t;



typedef struct DMENU_EventHandlers_s
{
    void * DMENU_Handlers_MenuGroup; // The base for each debug menu
    void * DMENU_Handlers_Menu; // The menus themselves which contain individual items
    void * DMENU_Handlers_DynMenu; // Menus with dynamic contents that're cleared and repopulated every time they're opened
    void * DMENU_Handlers_WeaponUpgradesMenu;
    void * DMENU_Handlers_ItemSubmenu;
    void * DMENU_Handlers_TaskItemSubmenu;
    void * DMENU_Handlers_SubnodeTaskItemSubmenu;
    void * DMENU_Handlers_BaseComponent;
    void * DMENU_Handlers_SeparatorItem;
    void * DMENU_Handlers_ToggleItem;
    void * DMENU_Handlers_ExecItem;
    void * DMENU_Handlers_AltExecItem;
    void * DMENU_Handlers_StringItem;
    void * DMENU_Handlers_StringListItem;
    void * DMENU_Handlers_DecimalItem;
    void * DMENU_Handlers_TaskHistoryItem;
    void * DMENU_Handlers_TaskItem;
    void * DMENU_Handlers_UserItem;
    void * DMENU_Handlers_TimerItem;
    void * DMENU_Handlers_CheckpointItem;
    void * DMENU_Handlers_FloatItem;

} DMENU_EventHandlers_t;







typedef struct dmenu_menu_group_s dmenu_menu_group_t;
// Addresses of various functions and variables in the game process, assuming aslr has been removed
typedef struct DMENU_Addresses_s
{
    dmenu_menu_group_t * dev_menu_group;
    dmenu_menu_group_t * quick_menu_group;
    dmenu_menu_group_t * favorites_menu_group;

    // Function addresses
    void * f_Malloc; // Malloc
    void * f_Free;   // Free
    void * f_LookupSymbol; // Lookup a script symbol via a provided SID
    
    // Variable addresses
    byte * d_Novis; // Toggle usage of visibility info (culling)

} DMENU_Addresses_t;




// Struct for handling game-specific addresses and other variables. Assigned after attaching
// is in dmenu.h for laziness reasons (easiest solution to an issue)
typedef struct ndgame
{
    pid_t pid;
    int status; // Status reported by waitpid the last time the process stopped
    game_id_t game_id;

    char title_id[0xA];
    char app_ver[0xB]; // String representation
    char title[0x80];

    DMENU_Types_t Types; // vPtr's for DMenu component class vTables
    DMENU_EventHandlers_t EventHandlers;
    DMENU_Addresses_t Addrs;

} NdGame_t;









typedef struct dmenu_menu_s dmenu_menu_t;
typedef struct dmenu_item_s dmenu_item_t;

typedef struct dmenu_item_s 
{
    dmenu_vptr_t vPtr; // Item class vPtr/Virtual Table Pointer
    void * UnknownPtr; // 0x08 // Always zero
    char ItemText[0x20]; // 32-byte buffer for the item text. If the string is too long, additional space is allocated instead and this is left unused. (it's also left null for separator lines) // 0x10
    char * pItemText; // Item text pointer. Usually points to the item_text_buffer, but is redirected if the string is too long (or left null for separator lines) // 0x30
    dmenu_menu_t * Parent; // The dmenu_menu_t that contains this item
    dmenu_item_t * NextItem; // The proceeding item in the same submenu
    const unsigned short HorizontalPadding;
    const unsigned short VerticalPadding;
    const unsigned short Width; 
    const unsigned short Height;
    char Unmapped0[0x8];
    unsigned long UnknownItemArrayIndex;
    unsigned long ItemVariable;
    unsigned int OffStateColour;
    unsigned int OnStateColour;
    unsigned int UnknownInt;
    unsigned int ShortcutInfo;
    void * Callback;
    unsigned long Unmapped1;
    char Unmapped2[0x8];
    dmenu_menu_group_t * MenuGroup; // Points to the dmenu base entry [0x90] 
    char Unmapped3[0x4];
    int _SelectionIndex;
    unsigned int _ItemCount; //!
    DMENU_item_eh_type_t EventHandlerTypeID; // unsure
    void * EventHandler;
    unsigned long ReturnBuffer;
    unsigned long Unmapped4;

} dmenu_item_t;







typedef struct dmenu_menu_s // 0xE0
{
    dmenu_vptr_t vPtr; // Item class vPtr/Virtual Table Pointer
    void * UnknownPtr; // 0x08 // Always zero
    char MenuName[0x20]; // Default buffer space for the menu title, if it fits the buffer. 
    char * pMenuName; // Pointer to the item's text; usually points to the item_text_buffer, but additional space is allocated and used instead if the string length is > 32. 
    dmenu_menu_t * Parent;
    dmenu_item_t * NextItem; // Next item in the regular entries. seems to still be read here, though
    unsigned short HorizontalPadding;
    unsigned short VerticalPadding;
    unsigned short Width;
    unsigned short Height;
    char Unmapped0[0x8];
    unsigned long UnknownMenuArrayIndex;
    unsigned long MenuVariable;
    unsigned int OffStateColour;
    unsigned int OnStateColour;
    unsigned int UnknownInt0;
    unsigned int ShortcutInfo;
    void * Callback;
    unsigned long Unmapped1;
    char Unmapped2[0x8];
    dmenu_menu_group_t * MenuGroup; // Points to the dmenu base entry [0x90] 
    unsigned int Unmapped3;
    int SelectionIndex;
    unsigned int ItemCount; // The amount of items in the root of the menu
    unsigned int UnknownFilterRelatedInt; // Gets set to zero when removing all filter items
    char UnknownBool; // not quite sure exactly what it does yet. I've only got guesses
    const char Unmapped4[0x7];
    dmenu_item_t * FirstItem; // Points to the first/top item in the menu [0xB0]
    unsigned int Unmapped5;
    char _UnknownMenuBool;
    char Unmapped6[0x3];
    unsigned long _FilterContents; // Something filter-related
    unsigned int Unmapped7;
    int OnScreenItemCount;
    unsigned long Unmapped8;
    unsigned * pUnknownInt1[0x4];
    void * UnknownStackPtr;

} dmenu_menu_t;

// typedef struct dmenu_menu_s // 0xE0
// {
//     dmenu_vptr_t vptr; // Item class vPtr/Virtual Table Pointer
//     void * unknown_function_ptr; // 0x08 // Always zero
//     char menu_name_buffer[0x20]; // Default buffer space for the item text, if it fits the buffer. 
//     char * menu_name; // Pointer to the item's text; usually points to the item_text_buffer, but additional space is allocated and used instead if the string length is > 32. 
//     dmenu_menu_t * parent_menu;
//     dmenu_item_t * next_item; // Next item in the regular entries. seems to still be read here, though
//     const unsigned short horizontal_padding;
//     const unsigned short vertical_padding;
//     const unsigned short width;
//     const unsigned short height;
//     char unmapped0[0x8];
//     unsigned long unknown_menu_array_index;
//     unsigned long menu_variable;
//     unsigned int on_state_colour;
//     unsigned int unknown_int;
//     unsigned int shortcut_info;
//     void * callback;
//     unsigned long unmapped2;
//     char unmapped3[0x8];
//     dmenu_menu_group_t * menu_group; // Points to the dmenu base entry [0x90] 
//     unsigned int unmapped4;
//     int selection_index;
//     unsigned int item_count; // The amount of items in the root of the menu
//     unsigned int unk_filter_related_int; // Gets set to zero when removing all filter items
//     char unknown_bool; // not quite sure exactly what it does yet. I've only got guesses
//     const char unmapped5[0x7];
//     dmenu_item_t * first_item; // Points to the first/top item in the menu [0xB0]
//     unsigned int unmapped6;
//     char menu_is_open;
//     char unmapped7[0x3];
//     unsigned long _filter_contents; // Something filter-related
//     unsigned int unmapped8;
//     int on_screen_item_count;
//     unsigned long unmapped9;
//     char unmapped10[0x4];
//     void * unk_stack_thing;

// } dmenu_menu_t;



typedef struct dmenu_menu_group_s // x >= 0x328
{
    dmenu_vptr_t vptr; // Item class vPtr/Virtual Table Pointer
    char unused1[0x8]; // Always zero
    char menu_group_name_buffer[0x20]; // Default buffer space for the menu base name, if it fits the buffer. 
    char * name; // Pointer to the menu base's text; usually points to the menu_group_name_buffer, but additional space is allocated and used instead if the string length is > 32. 
    char unused2[0x30]; // Always zero
    unsigned int off_state_colour;
    unsigned int on_state_colour;
    unsigned int unknown_int;
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

} dmenu_menu_group_t;












DMENU_stype_t DMENU_GetBaseType(void * item);
const char * DMENU_GetTypeAsString(dmenu_vptr_t type);
int DMENU_CreateToggleItem(dmenu_menu_t * pParentMenu, dmenu_item_t ** pItem, const char * ItemName, void * ItemVariable, void * CustomEventHandler);


dmenu_item_t * find_item(const char * item_name, dmenu_menu_t * start_menu, char recurse);
dmenu_menu_t * find_menu(const char * menu_name, dmenu_menu_t * start_menu, char recurse);
dmenu_item_t * find_submenu(const char * submenu_name, dmenu_menu_t * start_menu, char recurse);

dmenu_menu_t * get_root_menu(DMENU_menu_index_t menu_group_index);



int load_menu_group(const dmenu_menu_group_t * address, dmenu_menu_group_t * menu_group);
int load_menu(const dmenu_menu_t * address, dmenu_menu_t * menu);
int load_item(const dmenu_item_t * address, dmenu_item_t * item);

void dump_menu_entries();

int AppendItemToParent(dmenu_menu_t * parent_menu, dmenu_item_t * item);

int patch_game();
