#include "inc/include.h"
#include "inc/dmenu.h"
#include "inc/memory.h"
#include "inc/proc.h"

extern NdGame_t * game;



// Read the vPtr at the start of the provided item, and return the simple type of the option to simplify type checks (i.e. checking for stype_item rather than 14 different types)
DMENU_stype_t get_DMENU_entry_base_type(void * item)
{
    void * chk;
    chk = (void *) read_ulong(game->pid, item);

    if (chk == game->Types.DMENU_Types_MenuGroup)
        return stype_menu_group;
    
    if (chk == game->Types.DMENU_Types_Menu ||
        chk == game->Types.DMENU_Types_DynMenu ||
        chk == game->Types.DMENU_Types_WeaponUpgradesMenu)
        return stype_menu;

    if (chk == game->Types.DMENU_Types_ItemSubmenu ||
        chk == game->Types.DMENU_Types_TaskItemSubmenu ||
        chk == game->Types.DMENU_Types_SubnodeTaskItemSubmenu)
        return stype_itemsubmenu;

    if (chk == game->Types.DMENU_Types_BaseComponent ||
        chk == game->Types.DMENU_Types_SeparatorItem ||
        chk == game->Types.DMENU_Types_ToggleItem ||
        chk == game->Types.DMENU_Types_ExecItem ||
        chk == game->Types.DMENU_Types_AltExecItem ||
        chk == game->Types.DMENU_Types_StringItem ||
        chk == game->Types.DMENU_Types_StringListItem ||
        chk == game->Types.DMENU_Types_DecimalItem ||
        chk == game->Types.DMENU_Types_TaskHistoryItem ||
        chk == game->Types.DMENU_Types_TaskItem ||
        chk == game->Types.DMENU_Types_UserItem ||
        chk == game->Types.DMENU_Types_TimerItem ||
        chk == game->Types.DMENU_Types_CheckpointItem ||
        chk == game->Types.DMENU_Types_FloatItem)
        return stype_item;


    return -1;
}








// [DEBUG-ONLY] Convert the name of the provided vptr to it's string representation, without the prefixed the game id and type name (assuming I haven't removed the latter)
const char * get_DMENU_entry_type_as_string(DMENU_vptr_t type)
{
    static char return_string[0x20];
    memset(return_string, 0, sizeof(return_string));
    strncpy(return_string, "unknown_type", strlen("unknown_type"));
    
#ifdef DEBUG
    int count;

    const char * names[] = 
    {
        "MenuGroup",
        "Menu",
        "DynMenu",
        "WeaponUpgradesMenu",
        "ItemSubmenu",
        "TaskItemSubmenu",
        "SubnodeTaskItemSubmenu",
        "BaseComponent",    
        "SeparatorItem",
        "ToggleItem",
        "ExecItem",
        "AltExecItem",
        "StringItem",
        "StringListItem",
        "DecimalItem",
        "TaskHistoryItem",
        "TaskItem",
        "UserItem",
        "TimerItem",
        "CheckpointItem",
        "FloatItem"
    };

    const DMENU_vptr_t types[] =
    {
        game->Types.DMENU_Types_MenuGroup,
        game->Types.DMENU_Types_Menu,
        game->Types.DMENU_Types_DynMenu,
        game->Types.DMENU_Types_WeaponUpgradesMenu,
        game->Types.DMENU_Types_ItemSubmenu,
        game->Types.DMENU_Types_TaskItemSubmenu,
        game->Types.DMENU_Types_SubnodeTaskItemSubmenu,
        game->Types.DMENU_Types_BaseComponent,
        game->Types.DMENU_Types_SeparatorItem,
        game->Types.DMENU_Types_ToggleItem,
        game->Types.DMENU_Types_ExecItem,
        game->Types.DMENU_Types_AltExecItem,
        game->Types.DMENU_Types_StringItem,
        game->Types.DMENU_Types_StringListItem,
        game->Types.DMENU_Types_DecimalItem,
        game->Types.DMENU_Types_TaskHistoryItem,
        game->Types.DMENU_Types_TaskItem,
        game->Types.DMENU_Types_UserItem,
        game->Types.DMENU_Types_TimerItem,
        game->Types.DMENU_Types_CheckpointItem,
        game->Types.DMENU_Types_FloatItem
    };
    
    count = sizeof(types) / 8;
    for(int i = 0; i < count; i++)
    {
        if (type != types[i])
        {
            continue;
        }
        
        if(!names[i])
        {
            strncpy(return_string, "(null)", sizeof(return_string));
            break;
        }
        strncpy(return_string, names[i], sizeof(return_string));
        break;
    }
#endif

    return return_string;
}








// Recursively check every item and submenu for an item matching the specified dmenu simple type and component name
static void * find_DMENU_component(const char * expected_component_name, DMENU_menu_t * start_menu, DMENU_stype_t expected_component_stype, char recurse)
{
    #ifdef DEBUG
    echo("find_DMENU_component");
    #endif
    __rerrv(start_menu == (DMENU_menu_t *) -1, "start_menu == -1", nullptr);


    int ret;
    DMENU_menu_t * menu;
    if (start_menu == nullptr)
    {
        DMENU_menu_index_t curr_base_index;
        DMENU_menu_group_t * parent_base;
    
        curr_base_index = 0;
        if (0)
        {
    _recheck:
            if (curr_base_index >= 2)
            {
                #ifdef DEBUG
                echo("Didn't find component %s in Menu #%d; checking next menu", expected_component_name, curr_base_index);
                #endif
                return nullptr;
            }

            curr_base_index++;
        }

        
        parent_base = *(&game->Addrs.dev_menu_group + curr_base_index);
        if (parent_base == null)
        {
            echo("ERROR: Null parent base in ndgame struct at game->Addrs.dev_menu_group + %d, aborting", curr_base_index);
            return nullptr;
        }


        menu = (DMENU_menu_t *) read_ulong(game->pid, (unsigned long *) &parent_base->root_menu);
        __rerrv(menu == (DMENU_menu_t *) -1, strbufbuild("error reading root menu from menu base #%d", curr_base_index), nullptr);
        if (menu == 0)
        {
            echo("ERROR: Null menu base doesn't even have a root menu! Aborting.");
            return nullptr;
        }
    }
    else{
        menu = start_menu;
    }
    

    

    

    void * entry_ptr;

    entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &menu->first_item);

#ifdef DEBUG
    // Read current menu name before checking for an empty menu
    char parent_name[0xFF];
    ret = read_string(game->pid, (const char *) read_ulong(game->pid, (unsigned long *) &menu->menu_name), parent_name);
    __rerrv(ret == -1, strbufbuild("ERROR: Failed reading name of root menu from menu base \"%s\", aborting.", expected_component_name), nullptr);
    if (entry_ptr == nullptr)
    {
        echo("%s contains no items, aborting", parent_name);
        return nullptr;
    }

    echo("Searching %s for %s", parent_name, expected_component_name);
#else
    if(entry_ptr == 0)
    {
        return nullptr;
    }
#endif

    
    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size

    char next_is_menu;

    DMENU_stype_t base_type;
    char entry_name[0xFF];
    const char * entry_name_ptr;
    
    next_is_menu = 0;
    while(entry_ptr != nullptr)
    {
        base_type = get_DMENU_entry_base_type(entry_ptr);
        
        entry_name_ptr = (const char *) read_ulong(game->pid, (unsigned long *) &((DMENU_item_t *) entry_ptr)->item_text);
        if ((void *) entry_name_ptr == nullptr)
        {
            goto _no_name;
        }
        

        ret = read_string(game->pid, entry_name_ptr, entry_name);
        if (ret == -1)
        {
            __rerrv(ret != sizeof(DMENU_menu_t), strbufbuild("ERROR: Failed reading name from component at 0x%llX when searching for component \"%s\", aborting.", entry_ptr, expected_component_name), nullptr);
            break;
        }

        // Check for matching base type and component name
        if (expected_component_stype == base_type && !strncmp(expected_component_name, entry_name, strlen(entry_name)))
        {
            return entry_ptr;
        }

        
        _no_name:
        // Menu
        if (next_is_menu)
        {
            // Read next item pointer
            entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((DMENU_menu_t *) entry_ptr)->first_item);
            next_is_menu = 0;
        }
        // Item
        else {
            if (base_type == stype_itemsubmenu && recurse > 0)
            {
                // Item Submenu
                entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((DMENU_item_t *) entry_ptr)->return_buffer);

                history[++depth] = (void *) read_ulong(game->pid, (unsigned long *) &((DMENU_item_t *) entry_ptr)->next_item);

                next_is_menu = 1;
            }
            else{
                entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((DMENU_item_t *) entry_ptr)->next_item);
                next_is_menu = 0;
            }
        }
        
        if (recurse < 1)
        {
            continue;
        }


        while (entry_ptr == nullptr && depth >= 0)
        {
            entry_ptr = history[depth];
            history[depth--] = nullptr;
        
            next_is_menu = 0;
        }
    }



    goto _recheck;
}








// Get the address of the first item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided item_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
DMENU_item_t * find_item(const char * item_name, DMENU_menu_t * start_menu, char recurse)
{
    return find_DMENU_component(item_name, start_menu, stype_item, recurse);
}




// Get the address of the first menu in the desired parent menu for which the entry name (from the item_text ptr) matches the provided menu_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
DMENU_menu_t * find_menu(const char * menu_name, DMENU_menu_t * start_menu, char recurse)
{
    return find_DMENU_component(menu_name, start_menu, stype_item, recurse);
}




// Get the address of the first submenu item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided submenu_name
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
DMENU_item_t * find_submenu(const char * submenu_name, DMENU_menu_t * start_menu, char recurse)
{
    return find_DMENU_component(submenu_name, start_menu, stype_item, recurse);
}




// Get the root menu of the specified menu base without loading the base in to memory first
DMENU_menu_t * get_root_menu(DMENU_menu_index_t menu_group_index)
{
    // Why did I write it this way?!
    return ((DMENU_menu_t *) read_ulong(game->pid, (unsigned long *) &(* (DMENU_menu_group_t **) ((unsigned long) &game->Addrs.dev_menu_group + menu_group_index * sizeof(DMENU_menu_group_t *)))->root_menu));
}








// Copy the DMENU_menu_group_t pointed to by the DMENU_menu_group_t * at the provided address in the traced process' memory into the provided menu base struct
int load_menu_group(const DMENU_menu_group_t * address, DMENU_menu_group_t * menu_group)
{
    return read_bytes(game->pid, sizeof(DMENU_menu_group_t), (byte *) address, (byte *) menu_group) != sizeof(DMENU_menu_group_t);
}




// Copy the DMENU_menu_t at the provided address in the traced process' memory into the provided menu struct
int load_menu(const DMENU_menu_t * address, DMENU_menu_t * menu)
{
    return read_bytes(game->pid, sizeof(DMENU_menu_t), (byte *) address, (byte *) menu) != sizeof(DMENU_menu_t);
}




// Copy the DMENU_item_t at the provided address in the traced process' memory into the provided item struct
int load_item(const DMENU_item_t * address, DMENU_item_t * item)
{
    return read_bytes(game->pid, sizeof(DMENU_item_t), (byte *) address, (byte *) item) != sizeof(DMENU_item_t);
}






// Try and scan the current game process for all known dmenu types after getting the dev and quick menu base pointer addresses
// Returns: non-zero on failure- otherwise, zero 
static int try_dump_menu_entry_types()
{
    return 1;
    //! TODO:
    // 1 cycle through menu types, printing details on first instance of each type
    // 2 Take these entries (combed to avoid obvious game-specific options) and create this option to do the following

    // 3 Search for each item string, saving the type of each one found.
    // 3a at the end, if any exist- print all found unknown types and all unknown item names for manual assignment (also print the item name of the items these unknown types were found in this run)
}



static DMENU_vptr_t known_types[0x16] = {0};
static int foundTypes = 0;

//! Unoptimized menu entry info dumper. May fail if partially-stripped menus are added, like the SpawnAvObjects menu in T2R 02.001.010 with it's "next_entry" pointer being for some reason set to an address pointing to unrelated volatile memory 
static void do_dump_menu_entries(DMENU_menu_group_t * parent_base)
{
    // Null menu base pointer
    if (parent_base == 0)
    {
        echo("ERROR: Empty menu base pointer provided.");
        return;
    }
    if (parent_base->root_menu == 0) // Handle empty menus
    {
        if (parent_base->name)
        {
            echo("Provided menu base %s has no menus.", parent_base->name);
        }
        else {
            echo("Provided menu base (unnamed) has no menus.");
        }
        return;
    }


    
    DMENU_menu_t menu;
    read_bytes(game->pid, sizeof(DMENU_menu_t), (byte *) parent_base->root_menu, (byte *) &menu);
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).\n# Aborting.", get_DMENU_entry_type_as_string(menu.vptr), menu.menu_name, entry_ptr);
        return;
    }




    
    int depth = 0;
    DMENU_menu_t * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char item_name[0x80];
    char parent_name[0x80];
    
    char next_is_menu = 0;
    unsigned short unknown_type_count = 0;
    
    const SID unknownTypeSID = (SID) 0x9E3E4FA85A926262ul;
    
    char newItem = 0;

    DMENU_vptr_t type;
    DMENU_item_t item;
    const char * dtype;

    while(entry_ptr != null)
    {
        type = (DMENU_vptr_t) read_ulong(game->pid, entry_ptr);
        if ((long) type == -1) // no need to check errno, as it shouldn't be reading -1 either
        {
            entry_ptr = nullptr;
            
            AssertPushBufferedMsg("Failed to read item's vptr, or it was actually negative. Skipping item at 0x%llX, and returning from it's containing menu", entry_ptr);
            __errg((long) type == -1, NULL, _cont);
        }

        dtype = get_DMENU_entry_type_as_string(type);
        newItem = (char) 0x1;


        // Check for unknown dmenu component types
        if (encode_sid(dtype) == unknownTypeSID)
        {
            unknown_type_count++;
            newItem = (char) 0x0;
        }
        else {
            int i;
            for (i = 0; i < foundTypes; i++)
            {
                if (known_types[i] == type)
                {
                    newItem = (char) 0x0;
                    break;
                }
            }

            if (newItem)
            {
                known_types[foundTypes++] = type;
            }
        }


        // Menu
        if (next_is_menu)
        {
            bzero(&menu, sizeof(menu));
            load_menu(entry_ptr, &menu);

            if (newItem)
            {
                read_string(game->pid, menu.menu_name, item_name);
                read_string(game->pid, (void *) read_ulong(game->pid, (unsigned long *) &menu.parent_menu->menu_name), parent_name);

                echo("\n"
                    "0x%llX %s->%s (type: %s)\n"
                    
                    "vPtr: %llX\n"
                    "Callback: %llX\n",


                    entry_ptr, parent_name, item_name, dtype,

                    menu.vptr,
                    menu.callback
                );
            }
            


            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            bzero(&item, sizeof(item));
            load_item(entry_ptr, &item);

            if (newItem)
            {
                if (item.item_text)
                {
                    read_string(game->pid, item.item_text, item_name);
                }
                else {
                    strcpy(item_name, "--------------------------------");
                }
    
                read_string(game->pid, (void *) read_ulong(game->pid, (unsigned long *) &item.parent_menu->menu_name), parent_name);


                echo("\n"
                    "0x%llX %s->%s (type: %s)\n"

                    "vPtr: %llX\n"
                    "Callback: %llX\n"
                    "EventType: %llX\n"
                    "EventHandler: %llX\n",
                    
                    entry_ptr, parent_name, item_name, dtype,

                    item.vptr,
                    item.callback,
                    item.event_handler_type_id,
                    item.event_handler
                );
            }
            

            // Handle ItemSubmenu components
            if (get_DMENU_entry_base_type(entry_ptr) == stype_itemsubmenu)
            {
                entry_ptr = (DMENU_item_t *) item.return_buffer;

                history[++depth] = (DMENU_menu_t *) item.next_item;
                
                next_is_menu = 1;
            }
            else {
                entry_ptr = item.next_item;
                next_is_menu = 0;
            }
        }
        

        // Return from submenus
    _cont:
        while (entry_ptr == nullptr && depth >= 0)
        {
            entry_ptr = history[depth];
            history[depth--] = nullptr;
        
            next_is_menu = 0;
        }
    }

    echo("\n# Scan finished. Unknown item types: %d", unknown_type_count);
}




//! write me
void dump_menu_entries()
{
    echo("\n\nDumping Dev Menu entries.");
    DMENU_menu_group_t base;
    load_menu_group(game->Addrs.dev_menu_group, &base);
    do_dump_menu_entries(&base);
    echo("Finished dumping Dev Menu entries\n\n\n");

    echo("Dumping Quick Menu entries.");
    load_menu_group(game->Addrs.quick_menu_group, &base);
    do_dump_menu_entries(&base);
    echo("Finished dumping Quick Menu entries\n\n");
}














//#
//## Region for custom dmenu component creation functions //! write me a new region name
//#


//! write me
static int CreateBaseComponent(DMENU_item_t * item, const char * component_name)
{
    int ret;
    char * str_dest;
    int name_length;
    
    ret = 0;
    // Zero item in tracee's memory
    for (unsigned i = 0; i < sizeof(DMENU_item_t); i++)
    {
        ret |= write_byte(game->pid, (byte *) item + i, (byte) 0);
        __errg(ret != 0, strbufbuild("Failed writing zero byte to struct in tracee's memory. (wrote %d out of %d bytes)", i, sizeof(DMENU_item_t)), __break);
    }


__break:
    ret |= write_ulong(game->pid, (unsigned long *) &item->vptr, (unsigned long) game->Types.DMENU_Types_BaseComponent);
    
    ret |= write_uint(game->pid, &item->off_state_colour, (unsigned) 0xFFC8C8C8); // White
    ret |= write_uint(game->pid, &item->on_state_colour,  (unsigned) 0xFF00FFFF); // Yellow
    
    ret |= write_uint(game->pid, &item->unknown_int, (unsigned) 0xB0FFB0FF); // Value it always has, still not sure what this does. My original assumption appears to be wrong, though
    
    
    ret |= write_uint(game->pid, (unsigned *) &item->_item_count, 1);
    ret |= write_uint(game->pid, (unsigned *) &item->_selection_index, 1);


    if (component_name == NULL)
    {
        component_name = "unnamed";
        echo("Unnamed item created");
    }
    else {
        name_length = strlen(component_name);
        if (name_length < 0x20)
        {
            str_dest = (char *) &item->item_text_buffer;
            ret |= write_string(game->pid, str_dest, component_name);
        }
        else {
            if (name_length < 0xFF)
            {
                name_length = 0xFF;
            }

            str_dest = (char *) proc_malloc(name_length + 1);
            __rerrv(str_dest == NULL, "ERROR: Unable to allocate space for large component name!!!", 1);
            
            write_string(game->pid, str_dest, component_name);
            str_dest[name_length] = 0;
        }

        ret |= write_ulong(game->pid, (unsigned long *) &item->item_text, (unsigned long) str_dest);

        echo("%s created", component_name);
    }

    // Check for and report the number of failed processes
    AssertPushBufferedMsg("%d write processes failed when creating base component \"%s\".", component_name);
    __err(ret != 0,  nullptr);

    return ret != 0;
}








//! write me
static int CreateNonBaseComponent(DMENU_item_t * pComponent, const char * ItemName, void * ItemVariable, DMENU_vptr_t ItemType, void * ItemEventHandler, void * DefaultEventHandler)
{
    int ret;
    // Create component base
    ret = CreateBaseComponent(pComponent, ItemName);
    AssertPushBufferedMsg("Failed creating base component for %s, aborting.", ItemName);
    __rerrv(ret != 0, nullptr, 1);
    echo("Base component created, adding other crap");
    

    // Default to game's basic toggle item event handler if none is specified
    if (ItemEventHandler == nullptr)
    {
        //! TODO: FINISH THIS- this only works for tlou2's latest patch. finish the menu item dumper function we're gonna use to grab these
        ItemEventHandler = (void *) game->EventHandlers.DMENU_Handlers_ToggleItem;
    }
    
    // Item Class/Type (vTable pointer)
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->vptr, (unsigned long) game->Types.DMENU_Types_ToggleItem);


    // Event Handler type
    ret |= write_uint(game->pid, (unsigned *) &pComponent->event_handler_type_id, (unsigned) DMENU_HandlerType_ToggleItem);
    
    // Event Handler
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->event_handler, (unsigned long) ItemEventHandler);

    // Item Variable
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->item_variable, (unsigned long) ItemVariable);

    // Item return buffer
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->return_buffer, (unsigned long) NULL);
    ret |= write_byte(game->pid, (byte *) &pComponent->return_buffer, read_byte(game->pid, (byte *) &pComponent->item_variable));
    
    
    // Check for and report the number of failed processes
    AssertPushBufferedMsg("%d write processes failed when creating %s \"%s\".", get_DMENU_entry_type_as_string(ItemType), ItemName);
    __err(ret != 0, nullptr);

    return ret;
}








//! write me
int CreateToggleItem(DMENU_item_t * pItem, const char * ItemName, void * ItemVariable, void * ItemAlternateEventHandler)
{
    return CreateNonBaseComponent(pItem, ItemName, ItemVariable, game->Types.DMENU_Types_ToggleItem, ItemAlternateEventHandler, game->EventHandlers.DMENU_Handlers_ToggleItem) != 0;
}








//! write me
int CreateItemSubmenu(DMENU_item_t * pItemSubmenu, const char * SubmenuName, DMENU_menu_t * pParentMenu, DMENU_menu_t * pNewSubmenu)
{
    return CreateNonBaseComponent(pItemSubmenu, SubmenuName, (void *) pParentMenu, game->Types.DMENU_Types_ItemSubmenu, pNewSubmenu, NULL) != 0;
}







//! write me
int CreateMenu(DMENU_item_t * pItem, const char * MenuName, DMENU_menu_t * pMenu, void * ItemAlternateEventHandler)
{
    return CreateNonBaseComponent(pItem, MenuName, (void *) pMenu, game->Types.DMENU_Types_Menu, ItemAlternateEventHandler, game->EventHandlers.DMENU_Handlers_ToggleItem) != 0;
}








// Shorthand for allocating the memory for, creating, and appending a new DMENU_item_t to the provided parent menu
// Sets the outvar pItem to the pointer of the new menu entry, unless proc_malloc fails
// returns zero on success, and 1 on failure.
int DMENU_CreateToggleItem(DMENU_menu_t * pParentMenu, DMENU_item_t ** pItem, const char * ItemName, void * ItemVariable, void * CustomEventHandler)
{
    int ret;
    char menu_name[0xFF];

    if (find_item(ItemName, pParentMenu, false) > 0)
    {
        echo("Item \"%s\" already added to specified menu base", ItemName);
        return 0;
    }

    ret = read_string(game->pid, (const char *) &pParentMenu->menu_name, (char *) menu_name);
    __rerrv(ret == -1, "Error creating and adding new item; read_string returned error when reading name of parent menu", 1)

    echo("Adding %s to %s", ItemName, menu_name);


    // Allocate space for new menu entry
    DMENU_item_t * item;
    item = (DMENU_item_t *) proc_malloc(align(sizeof(DMENU_item_t)));
    __rerrv(!item, "Error allocating space for item", 1);

    *pItem = item;
    CreateToggleItem(item, ItemName, ItemVariable, CustomEventHandler);

    
    echo("Prepending item to parent menu");
    return AppendItemToParent(pParentMenu, item);
} 

















//#
//## Region for dmenu manipulation funcs like appending items or clearing menus //! write me a new region name
//#

//! write me
int AppendItemToParent(DMENU_menu_t * parent_menu, DMENU_item_t * item)
{
    DMENU_item_t
        **next_item_p = &parent_menu->first_item,
        *current_item = NULL,
        *prev_item
    ;


    // get the last item in the menu, if it has any contents
    do {
        prev_item = current_item;
        current_item = (DMENU_item_t *) read_ulong(game->pid, (unsigned long *) next_item_p);
        __rerrv(current_item == (DMENU_item_t *) -1 && errno != 0, "ERROR: Failed reading next item when trying to find last item in menu", 1);

        next_item_p = &current_item->next_item;
        
    } while (current_item != NULL && errno == 0);
    
    if (prev_item == NULL)
    {
        // Default to the menu's first item if it has no entries
        next_item_p = &parent_menu->first_item;
    }
    else{
        next_item_p = &prev_item->next_item;
    }


    
    int ret;
    ret = write_ulong(game->pid, (unsigned long *) next_item_p, (unsigned long) item);
    __rerrv(ret != 0, "ERROR: unable to write item pointer to game memory, aborting append function. ", 1);

    // Assign parent menu and set next item
    write_ulong(game->pid, (unsigned long *) &item->next_item, 0);
    write_ulong(game->pid, (unsigned long *) &item->parent_menu, (u_long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint(game->pid, (unsigned int *) &parent_menu->item_count, read_uint(game->pid, &parent_menu->item_count) + 1);
    write_uint(game->pid, (unsigned int *) &parent_menu->unknown_bool, 1);
    



    char _menu_is_open;
    _menu_is_open = (char) read_byte(game->pid, (byte *) &parent_menu->menu_is_open);

    if (_menu_is_open)
    {
        echo("updating menu");
        
        void * update_menu;
        update_menu = (void *) read_ulong(game->pid, (unsigned long *) read_ulong(game->pid, (unsigned long *) &item->vptr) + 0x18);
        if (!update_menu)
        {
            echo("Unable to read item's vPtr!!");
            return 1;
        }


        struct reg args;
        ret = proc_set_args(&args, (uintptr_t) item, 0, 0, 0, 0, 0, 0);
        __rerrv(ret != 0, "ERROR: Unable to set args for call to menu update function.", 1)
        
        
        ret = proc_call(update_menu, nullptr, &args);
        __rerrv(ret != 0, "ERROR: Failed calling menu update function.", 1)
    }

    return 0;
}








//! write me
int PrependItemToParent(DMENU_menu_t * parent_menu, DMENU_item_t * item)
{
    int ret;
    DMENU_item_t * first_item;

    first_item = (DMENU_item_t *) read_ulong(game->pid, (unsigned long *) &parent_menu->first_item);
    __rerrv(first_item == (DMENU_item_t *) -1 && errno != 0, "PrependItemToPArent Failed first read", 1);

    // Assign parent menu and set next item
    if (first_item != NULL)
    {
        write_ulong(game->pid, (unsigned long *) &item->next_item, (unsigned long) first_item);
    }
    write_ulong(game->pid, (unsigned long *) &parent_menu->first_item, (unsigned long) item);
    write_ulong(game->pid, (unsigned long *) &item->parent_menu, (unsigned long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint(game->pid, (unsigned int *) &parent_menu->item_count, read_uint(game->pid, &parent_menu->item_count) + 1);
    write_uint(game->pid, (unsigned int *) &parent_menu->unknown_bool, 1);
    



    char _menu_is_open;
    _menu_is_open = (char) read_byte(game->pid, (byte *) &parent_menu->menu_is_open);

    if (_menu_is_open)
    {
        echo("updating menu");
        
        void * update_menu;
        update_menu = (void *) read_ulong(game->pid, (unsigned long *) read_ulong(game->pid, (unsigned long *) &item->vptr) + 0x18);
        if (!update_menu)
        {
            echo("Unable to read item's vPtr!!");
            return 1;
        }
        

        struct reg args;
        ret = proc_set_args(&args, (uintptr_t) item, 0, 0, 0, 0, 0, 0);
        __rerrv(ret != 0, "ERROR: Unable to set args for call to menu update function.", 1)
        
        
        ret = proc_call(update_menu, nullptr, &args);
        __rerrv(ret != 0, "ERROR: Failed calling menu update function.", 1)
    }

    return 0;
}








//! write me
int InsertItemInParent(DMENU_menu_t * parent_menu, DMENU_item_t * item, const char * target_item_name, char prepend)
{
    return 1;

    DMENU_item_t * target_item;
    DMENU_menu_t * menu_buf;
    DMENU_menu_group_t * base;

    menu_buf = parent_menu;
    do {
        base = (DMENU_menu_group_t *) read_ulong(game->pid, (unsigned long *) &menu_buf->menu_group);

        if (base != (DMENU_menu_group_t *) nullptr)
        {
            break;
        }

        menu_buf = (DMENU_menu_t *) read_ulong(game->pid, (unsigned long *) &menu_buf->parent_menu);
        __rerrv(menu_buf == (DMENU_menu_t *) nullptr, "Failed to find menu base for parent menu when inserting item. (all parent menus have null bases, and we've run out of parents!!)", 1);

    } while (1);





    target_item = (DMENU_item_t *) find_DMENU_component(target_item_name, parent_menu, stype_item, false);
    if (target_item == nullptr)
    {
        if (errno)
        {
            echo("ERROR: failed finding dmenu component \"%s\". (error: %s)", target_item_name, strerror(errno));
            return 1;
        }
    }

    //! TODO: Write this function
    if (prepend)
    {
        // Read the parent menu of the found item, and get the preceeding item to the target.

        return 0;
    }


    
    return 0;
}
















//#
//## Region for custom menu patch functions //! write me a new region name
//#

//! TODO: Write the base menu item creation function and move the optimization to that. I don't want bzero being optimized out.
int patch_game()
{
    DMENU_item_t * itemBuf;
    DMENU_menu_t * parentMenu;

    itemBuf = nullptr;
    parentMenu = (DMENU_menu_t *) read_ulong(game->pid, (unsigned long *) &game->Addrs.favorites_menu_group->root_menu);
    DMENU_CreateToggleItem(parentMenu, &itemBuf, "Disable All Visibility", game->Addrs.d_Novis, nullptr);

    return 0;
}

