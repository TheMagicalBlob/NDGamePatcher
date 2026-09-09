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
static void * find_dmenu_component(const char * expected_entry_name, dmenu_stype_t expected_base_type, dmenu_menu_index_t parent_index)
{
    if (parent_index < 0 || parent_index > 2)
    {
        echo("Invalid parent menu index provided, defaulting to dev menu");
        parent_index = 0;
    }
    
    #ifdef DEBUG
    echo("find_dmenu_component");
    #endif

    int ret;
    dmenu_menu_base_t parent_base;
    ret = load_menu_base(*(&game->addrs.dev_menu_base + parent_index), &parent_base);
    __rerrv(ret != 0, strbufbuild("ERROR: Failed loading menu base when preparing to find dmenu component \"%s\", aborting.", expected_entry_name), nullptr);

    if (parent_base.root_menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided. (%s)", parent_base.name ? parent_base.name : (char *) &parent_base.menu_base_name_buffer);
        return nullptr;
    }
    
    
    dmenu_menu_t menu;
    ret = read_bytes(game->pid, sizeof(dmenu_menu_t), (byte *) parent_base.root_menu, (byte *) &menu);
    __rerrv(ret != sizeof(dmenu_menu_t), strbufbuild("ERROR: Failed reading root menu from menu base \"%s\", aborting.", expected_entry_name), nullptr);
    

    char parent_name[0xFF];
    bzero(parent_name, sizeof(parent_name));
    

    ret = read_string(game->pid, menu.menu_name, parent_name);
    __rerrv(ret == -1, strbufbuild("ERROR: Failed reading name of root menu from menu base \"%s\", aborting.", expected_entry_name), nullptr);
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        echo("menu %s contains no items, aborting", parent_name);
        return nullptr;
    }
    
    echo("Searching %s for %s", parent_name, expected_entry_name);
    
    
    char next_is_menu = 0;

    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    char entry_name[0xFF];
    
    dmenu_stype_t base_type;
    

    
    int depth = 0;
    const char * entry_name_ptr;
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
            __rerrv(ret != sizeof(dmenu_menu_t), strbufbuild("ERROR: Failed reading name from component at 0x%llX when searching for component \"%s\", aborting.", entry_ptr, expected_entry_name), nullptr);
            break;
        }

        // Check for matching base type and component name
        if (expected_base_type == base_type && !strncmp(expected_entry_name, entry_name, strlen(entry_name)))
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
            if (base_type == stype_itemsubmenu)
            {
                entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) entry_ptr)->return_buffer);

                history[++depth] = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) entry_ptr)->next_item);

                next_is_menu = 1;
            }
            else{
                entry_ptr = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) entry_ptr)->next_item);
                next_is_menu = 0;
            }
        }
        
        

        while (entry_ptr == nullptr && depth >= 0)
        {
            entry_ptr = history[depth];
            history[depth--] = nullptr;
        
            next_is_menu = 0;
        }
    }


    return nullptr;
}








// Get the address of the first item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided item_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_item_t * find_item(const char * item_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(item_name, stype_item, parent_index);
}




// Get the address of the first menu in the desired parent menu for which the entry name (from the item_text ptr) matches the provided menu_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_menu_t * find_menu(const char * menu_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(menu_name, stype_menu, parent_index);
}




// Get the address of the first submenu item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided submenu_name
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_item_t * find_submenu(const char * submenu_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(submenu_name, stype_itemsubmenu, parent_index);
}




// Get the root menu of the specified menu base without loading the base in to memory first
dmenu_menu_t * get_root_menu(dmenu_menu_index_t menu_base_index)
{
    return ((dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &(* (dmenu_menu_base_t **) ((unsigned long) &game->addrs.dev_menu_base + menu_base_index * sizeof(dmenu_menu_base_t *)))->root_menu));
}








// Copy the dmenu_menu_base_t pointed to by the dmenu_menu_base_t * at the provided address in the traced process' memory into the provided menu base struct
int load_menu_base(void * address, dmenu_menu_base_t * menu_base)
{
    return read_bytes(game->pid, sizeof(dmenu_menu_base_t), (void *) address, (byte *) menu_base);
}




// Copy the dmenu_menu_t at the provided address in the traced process' memory into the provided menu struct
int load_menu(void * address, dmenu_menu_t * menu)
{
    return read_bytes(game->pid, sizeof(dmenu_menu_t), address, (byte *) menu);
}




// Copy the dmenu_item_t at the provided address in the traced process' memory into the provided item struct
int load_item(void * address, dmenu_item_t * item)
{
    return read_bytes(game->pid, sizeof(dmenu_item_t), address, (byte *) item);
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



    #ifdef DEBUG
    // SID known_types[0x15] = {0};
    #endif

    
    
    
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







// STILL UNFINISHED!! Treats the item as though it's in the payload's memory, which it isn't
dmenu_item_t * CreateBaseComponent(/*dmenu_component_t*/ dmenu_item_t * entry, const char * component_name)
{
    return nullptr;

    char * __dest;
    int name_length;

    bzero(&entry, sizeof(entry));

    entry->vptr = game->vptrs.base_component;
    entry->off_state_colour = 0xffc8c8c8;
    entry->on_state_colour = 0xff00ffff;
    entry->unknown_int = 0xffb0ffb0;

    if (component_name == NULL)
    {
        echo("Unnamed item created");
        return nullptr;
    }


    name_length = strlen(component_name);
    if (name_length < 0x20)
    {
        __dest = entry->item_text_buffer;
        strcpy(__dest, component_name);
    }
    else {
        if (name_length < 0xFF)
        {
            name_length = 0xFF;
        }

        __dest = (char *) proc_malloc(name_length + 1);
        __rerrv(__dest == NULL, "ERROR: Unable to allocate space for large component name!!!", nullptr);
        
        write_string(game->pid, __dest, component_name);
        __dest[name_length] = 0;
    }

    entry->item_text = __dest;

    echo("%s created", component_name);
}









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
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }

    // Assign parent menu and set next item
    write_ulong(game->pid, (unsigned long *) &item->next_item, 0);
    write_ulong(game->pid, (unsigned long *) &item->parent_menu, (u_long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint(game->pid, (unsigned int *) &parent_menu->item_count, read_uint(game->pid, &parent_menu->item_count) + 1);
    write_uint(game->pid, (unsigned int *) &parent_menu->unknown_bool, 1);
    
    char _menu_is_open;
    _menu_is_open = read_byte(game->pid, (byte *) &parent_menu->menu_is_open);

    if (_menu_is_open)
    {
        echo("menu is open");
        void * update_menu;
        if (!(update_menu = (void *) read_ulong(game->pid, (unsigned long *) read_ulong(game->pid, (unsigned long *) &item->vptr) + 0x18)))
        {
            echo("Unable to read item's vPtr!!");
            return 1;
        }
        
        
        struct reg args;

        ret = proc_set_args(&args, (uintptr_t) item, 0, 0, 0, 0, 0, 0);
        if (ret)
        {
            echo("ERROR @ Line: %d", __LINE__ - 3);
            return 1;
        }


        ret = proc_call(update_menu, nullptr, &args);
        if (ret)
        {
            echo("ERROR @ Line: %d", __LINE__ - 3);
            return 1;
        }
    }
    echo("menu not open");

    return 0;
}










int PrependItemToParent(dmenu_menu_t * parent_menu, dmenu_item_t * item)
{
    echo("Unimplemented function \"%s\" @ line %d in %s called prematurely.", __FUNCTION__, __LINE__, __FILE__);
    return 1;
}











//! TODO: Write the base menu item creation function and move the optimization to that. I don't want bzero being optimized out.
int add_novis_option(dmenu_menu_index_t menu_base)
{
    dmenu_item_t * check;
    if ((check = find_item("Disable All Visibility", menu_base)))
    {
        echo("Novis already added to specified menu base");
        return 0;
    }
    echo("Adding new novis option");


    // Allocate space for new menu entry
    void * mem;
    mem = proc_malloc(align(sizeof(dmenu_item_t)));
    __rerrv(!mem, "Error allocating space for novis option", 1);

    echo("Allocated mem for new entry");
    proc_wait(false);




    dmenu_item_t item;
    bzero(&item, sizeof(dmenu_item_t));

    item.vptr = game->vptrs.toggle_item;

    item.event_handler_type_id = 3; // Bool item
    item.event_handler = (void *) 0x0114d100; // Bool event handler
    item.return_buffer = read_byte(game->pid, (byte *) game->addrs.dat.novis);

    item.off_state_colour = 0xB0FFB0FF; // White
    item.on_state_colour  = 0xEF00FFFF; // Yellow
    item.unknown_int      = 0xB0FFB0FF; // Value it always has, still not sure what this does. My original assumption appears to be wrong, though
    item._selection_index = 1;
    item._item_count      = 1;

    
    strcpy(item.item_text_buffer, "Disable All Visibility");

    item.item_text = (char *) &((dmenu_item_t *) mem)->item_text_buffer;

    item.item_variable = game->addrs.dat.novis;

    
    echo("Writing local item to allocated memory space");
    int ret;
    ret = write_bytes(game->pid, sizeof(dmenu_item_t), mem, (byte *) &item);
    __rerrv(ret != sizeof(dmenu_item_t), "Error writing new menu option to allocated memory, aborting novis option addition.", 1);


    echo("Appending item to parent menu");
    return AppendItemToParent(get_root_menu(menu_base), mem);
}

