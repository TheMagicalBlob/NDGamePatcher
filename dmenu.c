#include "inc/include.h"
#include "inc/dmenu.h"
#include "inc/memory.h"
#include "inc/proc.h"
#ifdef DEBUG
#include "inc/debug.h"
#endif


extern ndgame_t * game;



// Read the vPtr at the start of the provided item, and return the simple type of the option to simplify type checks (i.e. checking for stype_item rather than 14 different types)
dmenu_stype_t get_dmenu_entry_base_type(void * item)
{
    void * chk;
    chk = (void *) read_ulong(game->pid, item);

    if (chk == game->vptrs.menu_base)
        return stype_menu_base;

    if (chk == game->vptrs.menu ||
        chk == game->vptrs.dyn_menu ||
        chk == game->vptrs.weapon_upgrades_menu)
        return stype_menu;

    if (chk == game->vptrs.item_submenu ||
        chk == game->vptrs.task_item_submenu ||
        chk == game->vptrs.subnode_task_item_submenu)
        return stype_itemsubmenu;

    if (chk == game->vptrs.base_component ||
        chk == game->vptrs.string_list_item ||
        chk == game->vptrs.toggle_item ||
        chk == game->vptrs.exec_item ||
        chk == game->vptrs.alt_exec_item ||
        chk == game->vptrs.float_item ||
        chk == game->vptrs.decimal_item ||
        chk == game->vptrs.separator_item ||
        chk == game->vptrs.string_item ||
        chk == game->vptrs.task_history_item ||
        chk == game->vptrs.task_item ||
        chk == game->vptrs.timer_item ||
        chk == game->vptrs.user_item ||
        chk == game->vptrs.checkpoint_item)
        return stype_item;


    return -1;
}








// [DEBUG-ONLY] Convert the name of the provided vptr to it's string representation, without the prefixed the game id and type name (assuming I haven't removed the latter)
const char * get_dmenu_entry_type_as_string(dmenu_vptr_t type)
{
    static char return_string[0x20];
    memset(return_string, 0, sizeof(return_string));
    strncpy(return_string, "unknown_type", strlen("unknown_type"));
    
#ifdef DEBUG
    int count;

    const char * names[] = 
    {
        "menu_base",

        "menu",
        "dyn_menu",
        "weapon_upgrades_menu",
        
        "item_submenu",
        "task_item_submenu",
        "subnode_task_item_submenu",
        
        "base_component",    
        "separator_item",
        "toggle_item",
        "exec_item",
        "alt_exec_item",
        "float_item",
        "decimal_item",
        "string_list_item",
        "task_history_item",
        "task_item",
        "user_item",
        "timer_item",
        "checkpoint_item",
        "string_item"
    };

    const dmenu_vptr_t types[] =
    {
        game->vptrs.menu_base,
        
        game->vptrs.menu,
        game->vptrs.dyn_menu,
        game->vptrs.weapon_upgrades_menu,
        
        game->vptrs.item_submenu,
        game->vptrs.task_item_submenu,
        game->vptrs.subnode_task_item_submenu,

        game->vptrs.base_component,
        game->vptrs.separator_item,
        game->vptrs.toggle_item,
        game->vptrs.exec_item,
        game->vptrs.alt_exec_item,
        game->vptrs.float_item,
        game->vptrs.decimal_item,
        game->vptrs.string_list_item,
        game->vptrs.task_history_item,
        game->vptrs.task_item,
        game->vptrs.user_item,
        game->vptrs.timer_item,
        game->vptrs.checkpoint_item,
        game->vptrs.string_item
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
static void * find_dmenu_component(const char * expected_component_name, dmenu_menu_t * start_menu, dmenu_stype_t expected_component_stype, char recurse)
{
    #ifdef DEBUG
    echo("find_dmenu_component");
    #endif
    __rerrv(start_menu == (dmenu_menu_t *) -1, "start_menu == -1", nullptr);


    int ret;
    dmenu_menu_t * menu;
    if (start_menu == nullptr)
    {
        dmenu_menu_index_t curr_base_index;
        dmenu_menu_base_t * parent_base;
    
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

        
        parent_base = *(&game->addrs.dev_menu_base + curr_base_index);
        if (parent_base == null)
        {
            echo("ERROR: Null parent base in ndgame struct at game->addrs.dev_menu_base + %d, aborting", curr_base_index);
            return nullptr;
        }


        menu = (dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &parent_base->root_menu);
        __rerrv(menu == (dmenu_menu_t *) -1, strbufbuild("error reading root menu from menu base #%d", curr_base_index), nullptr);
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

    dmenu_stype_t base_type;
    char entry_name[0xFF];
    const char * entry_name_ptr;
    
    next_is_menu = 0;
    while(entry_ptr != nullptr)
    {
        base_type = get_dmenu_entry_base_type(entry_ptr);
        
        entry_name_ptr = (const char *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) entry_ptr)->item_text);
        if ((void *) entry_name_ptr == nullptr)
        {
            goto _no_name;
        }
        

        ret = read_string(game->pid, entry_name_ptr, entry_name);
        if (ret == -1)
        {
            __rerrv(ret != sizeof(dmenu_menu_t), strbufbuild("ERROR: Failed reading name from component at 0x%llX when searching for component \"%s\", aborting.", entry_ptr, expected_component_name), nullptr);
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
            entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_menu_t *) entry_ptr)->first_item);
            next_is_menu = 0;
        }
        // Item
        else {
            if (base_type == stype_itemsubmenu && recurse > 0)
            {
                // Item Submenu
                entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) entry_ptr)->return_buffer);

                history[++depth] = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) entry_ptr)->next_item);

                next_is_menu = 1;
            }
            else{
                entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) entry_ptr)->next_item);
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
dmenu_item_t * find_item(const char * item_name, dmenu_menu_t * start_menu, char recurse)
{
    return find_dmenu_component(item_name, start_menu, stype_item, recurse);
}




// Get the address of the first menu in the desired parent menu for which the entry name (from the item_text ptr) matches the provided menu_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_menu_t * find_menu(const char * menu_name, dmenu_menu_t * start_menu, char recurse)
{
    return find_dmenu_component(menu_name, start_menu, stype_item, recurse);
}




// Get the address of the first submenu item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided submenu_name
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_item_t * find_submenu(const char * submenu_name, dmenu_menu_t * start_menu, char recurse)
{
    return find_dmenu_component(submenu_name, start_menu, stype_item, recurse);
}




// Get the root menu of the specified menu base without loading the base in to memory first
dmenu_menu_t * get_root_menu(dmenu_menu_index_t menu_base_index)
{
    // Why did I write it this way?!
    return ((dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &(* (dmenu_menu_base_t **) ((unsigned long) &game->addrs.dev_menu_base + menu_base_index * sizeof(dmenu_menu_base_t *)))->root_menu));
}








// Copy the dmenu_menu_base_t pointed to by the dmenu_menu_base_t * at the provided address in the traced process' memory into the provided menu base struct
int load_menu_base(const dmenu_menu_base_t * address, dmenu_menu_base_t * menu_base)
{
    return read_bytes(game->pid, sizeof(dmenu_menu_base_t), (byte *) address, (byte *) menu_base) != sizeof(dmenu_menu_base_t);
}




// Copy the dmenu_menu_t at the provided address in the traced process' memory into the provided menu struct
int load_menu(const dmenu_menu_t * address, dmenu_menu_t * menu)
{
    return read_bytes(game->pid, sizeof(dmenu_menu_t), (byte *) address, (byte *) menu) != sizeof(dmenu_menu_t);
}




// Copy the dmenu_item_t at the provided address in the traced process' memory into the provided item struct
int load_item(const dmenu_item_t * address, dmenu_item_t * item)
{
    return read_bytes(game->pid, sizeof(dmenu_item_t), (byte *) address, (byte *) item) != sizeof(dmenu_item_t);
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





//! write me
static void do_dump_menu_entries(dmenu_menu_base_t * parent_base)
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


    dmenu_menu_t menu;
    read_bytes(game->pid, sizeof(dmenu_menu_t), (byte *) parent_base->root_menu, (byte *) &menu);
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).\n# Aborting.", get_dmenu_entry_type_as_string(menu.vptr), menu.menu_name, entry_ptr);
        return;
    }



    // #ifdef DEBUG
    // SID known_types[0x15] = {0};
    // #endif

    
    
    
    int depth = 0;
    dmenu_menu_t * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char item_name[0x80];
    char parent_name[0x80];
    
    char next_is_menu = 0;
    unsigned short unknown_type_count = 0;
    
    dmenu_vptr_t type;
    dmenu_item_t item;
    const char * dtype;

    while(entry_ptr != null)
    {
        type = (dmenu_vptr_t) read_ulong(game->pid, entry_ptr);
        dtype = get_dmenu_entry_type_as_string(type);
        
        // Check for unknown dmenu component types
        if (*(unsigned long long *) dtype == 2336936577129475669) // just the start of the "Unknown value..." string read as an unsigned 64bit integer
        {
            unknown_type_count++;
        }
        
        

        // Menu
        if (next_is_menu)
        {
            bzero(&menu, sizeof(menu));
            load_menu(entry_ptr, &menu);
            
            read_string(game->pid, menu.menu_name, item_name);
            read_string(game->pid, (void *) read_ulong(game->pid, (unsigned long *) &menu.parent_menu->menu_name), parent_name);

            echo("0x%llX %s->%s (type: %s)", entry_ptr, parent_name, item_name, dtype);
            echo("\n"
                "vPtr: %llX\n"
                "Callback: %llX\n"
                "Variable: %llX\n",

                menu.vptr,
                menu.callback,
                menu.menu_variable
            );
            


            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            bzero(&item, sizeof(item));
            load_item(entry_ptr, &item);

            if (item.item_text)
            {
                read_string(game->pid, item.item_text, item_name);
            }
            else {
                strcpy(item_name, "--------------------------------");
            }

            read_string(game->pid, (void *) read_ulong(game->pid, (unsigned long *) &item.parent_menu->menu_name), parent_name);

            echo("0x%llX %s->%s (type: %s)", entry_ptr, parent_name, item_name, dtype);
            echo("\n"
                "vPtr: %llX\n"
                "Callback: %llX\n"
                "Variable: %llX\n"
                "EventHandler: %llX\n"
                "ItemType: %llX\n",

                item.vptr,
                item.callback,
                item.item_variable,
                item.event_handler,
                item.event_handler_type_id
            );
            

            

            // Handle ItemSubmenu components
            if (get_dmenu_entry_base_type(entry_ptr) == stype_itemsubmenu)
            {
                entry_ptr = (dmenu_item_t *) item.return_buffer;

                history[++depth] = (dmenu_menu_t *) item.next_item;
                
                next_is_menu = 1;
            }
            else{
                entry_ptr = item.next_item;
                next_is_menu = 0;
            } 
        }
        
        

        // Return from submenus
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
    dmenu_menu_base_t base;
    load_menu_base(game->addrs.dev_menu_base, &base);
    do_dump_menu_entries(&base);
    echo("Finished dumping Dev Menu entries\n\n\n");

    echo("Dumping Quick Menu entries.");
    load_menu_base(game->addrs.quick_menu_base, &base);
    do_dump_menu_entries(&base);
    echo("Finished dumping Quick Menu entries\n\n");
}














//#
//## Region for custom dmenu component creation functions //! write me a new region name
//#


//! write me
int CreateBaseComponent(/*dmenu_component_t*/ dmenu_item_t * item, const char * component_name)
{
    int ret;
    char * str_dest;
    int name_length;

    // Zero item in tracee's memory
    for (unsigned i = 0; i < sizeof(dmenu_item_t); i++)
    {
        ret = write_byte(game->pid, (byte *) item + i, (byte) 0);
        __rerrv(ret != 0, strbufbuild("Failed writing zero byte to struct in tracee's memory. (wrote %d out of %d bytes)", i, sizeof(dmenu_item_t)), 1);
    }

    ret = write_ulong(game->pid, (unsigned long *) &item->vptr, (unsigned long) game->vptrs.base_component);
    
    ret = write_uint(game->pid, &item->off_state_colour, (unsigned) 0xFFC8C8C8); // White
    ret = write_uint(game->pid, &item->on_state_colour,  (unsigned) 0xFF00FFFF); // Yellow
    
    ret = write_uint(game->pid, &item->unknown_int, (unsigned) 0xB0FFB0FF); // Value it always has, still not sure what this does. My original assumption appears to be wrong, though
    
    
    ret = write_uint(game->pid, (unsigned *) &item->_selection_index, 1);
    ret = write_uint(game->pid, (unsigned *) &item->_item_count, 1);


    if (component_name == NULL)
    {
        echo("Unnamed item created");
        return 0;
    }


    name_length = strlen(component_name);
    if (name_length < 0x20)
    {
        str_dest = (char *) &item->item_text_buffer;
        ret = write_string(game->pid, str_dest, component_name);
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

    ret = write_ulong(game->pid, (unsigned long *) &item->item_text, (unsigned long) str_dest);

    echo("%s created", component_name);
    return 0;
}








//! write me
int CreateToggleItem(dmenu_item_t * item, const char * item_name, void * variable, void * alternate_event_handler)
{
    int ret;
    // Create component base
    ret = CreateBaseComponent(item, "Disable All Visibility");
    __rerrv(ret != 0, "Failed creating base component for novis option, aborting.", 1);
    
    // Default to game's basic toggle item event handler if none is specified
    if (alternate_event_handler == nullptr)
    {
        //! TODO: FINISH THIS- this only works for tlou2's latest patch 
        alternate_event_handler = (void *) 0x0114d100; // (0x0114d100 == Basic bool event handler for Tlou2 Remastered 02.001.010)
    }

    echo("Base component created, adding other crap");
    
    // Item Class/Type (vTable pointer)
    ret |= write_ulong(game->pid, (unsigned long *) &item->vptr, (unsigned long) game->vptrs.toggle_item);


    // Event Handler type
    ret |= write_uint(game->pid, (unsigned *) &item->event_handler_type_id, (unsigned) toggle_item);
    
    // Event Handler
    ret |= write_ulong(game->pid, (unsigned long *) &item->event_handler, (unsigned long) alternate_event_handler);

    // Item Variable
    ret |= write_ulong(game->pid, (unsigned long *) &item->item_variable, (unsigned long) variable);

    // Item return buffer
    ret |= write_ulong(game->pid, (unsigned long *) &item->return_buffer, (unsigned long) NULL);
    ret |= write_byte(game->pid, (byte *) &item->return_buffer, read_byte(game->pid, (byte *) &item->item_variable));
    

    return ret;
}








//! write me
int AppendItemToParent(dmenu_menu_t * parent_menu, dmenu_item_t * item)
{
    dmenu_item_t
        **next_item_p = &parent_menu->first_item,
        *current_item = NULL,
        *prev_item
    ;


    // get the last item in the menu, if it has any contents
    do {
        prev_item = current_item;
        current_item = (dmenu_item_t *) read_ulong(game->pid, (unsigned long *) next_item_p);
        __rerrv(current_item == (dmenu_item_t *) -1 && errno != 0, "ERROR: Failed reading next item when trying to find last item in menu", 1);

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
int PrependItemToParent(dmenu_menu_t * parent_menu, dmenu_item_t * item)
{
    int ret;
    dmenu_item_t * first_item;

    first_item = (dmenu_item_t *) read_ulong(game->pid, (unsigned long *) &parent_menu->first_item);
    __rerrv(first_item == (dmenu_item_t *) -1 && errno != 0, "PrependItemToPArent Failed first read", 1);

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
int InsertItemInParent(dmenu_menu_t * parent_menu, dmenu_item_t * item, const char * target_item_name, char prepend)
{
    return 1;

    dmenu_item_t * target_item;
    dmenu_menu_t * menu_buf;
    dmenu_menu_base_t * base;

    menu_buf = parent_menu;
    do {
        base = (dmenu_menu_base_t *) read_ulong(game->pid, (unsigned long *) &menu_buf->menu_base);

        if (base != (dmenu_menu_base_t *) nullptr)
        {
            break;
        }

        menu_buf = (dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &menu_buf->parent_menu);
        __rerrv(menu_buf == (dmenu_menu_t *) nullptr, "Failed to find menu base for parent menu when inserting item. (all parent menus have null bases, and we've run out of parents!!)", 1);

    } while (1);





    target_item = (dmenu_item_t *) find_dmenu_component(target_item_name, parent_menu, stype_item, false);
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
int add_novis_option(dmenu_menu_t * menu)
{
    if (find_item("Disable All Visibility", menu, false) > 0)
    {
        echo("Novis already added to specified menu base");
        return 0;
    }
    echo("Adding new novis option");


    // Allocate space for new menu entry
    dmenu_item_t * item;
    item = (dmenu_item_t *) proc_malloc(align(sizeof(dmenu_item_t)));
    __rerrv(!item, "Error allocating space for novis option", 1);
    
    CreateToggleItem(item, "Disable All Visibility", game->addrs.dat.novis, nullptr);
    
    echo("Prepending item to parent menu");
    return AppendItemToParent(menu, item);
}

