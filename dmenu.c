#include "inc/memory.h"
#include "inc/dmenu.h"
#include "inc/proc.h"


// Read the vPtr at the start of the provided item, and return the simple type of the option to simplify type checks (i.e. checking for stype_item rather than 14 different types)
int get_dmenu_entry_base_type(ndgame_t * ndgame, void * item)
{
    unsigned long long chk = (unsigned long long) read_ulong(ndgame->pid, item);

    switch (chk)
    {
        case t2r_vptr_menu_base:
            return stype_menu_base;

        case t2r_vptr_dyn_menu:
        case t2r_vptr_weapon_upgrades_menu:
        case t2r_vptr_menu:
            return stype_menu;

        case t2r_vptr_item_submenu:
        case t2r_vptr_task_item_submenu:
        case t2r_vptr_noreturn_task_item_submenu:
            return stype_itemsubmenu;

        case t2r_vptr_base_component:
        case t2r_vptr_bool_item:
        case t2r_vptr_exec_item:
        case t2r_vptr_alt_exec_item:
        case t2r_vptr_float_item:
        case t2r_vptr_decimal_item:
        case t2r_vptr_separator_item:
        case t2r_vptr_string_list_item:
        case t2r_vptr_string_item:
        case t2r_vptr_task_history_item:
        case t2r_vptr_task_item:
        case t2r_vptr_timer_item:
        case t2r_vptr_user_item:
        case t2r_vptr_checkpoint_item:
            return stype_item;
            





        case t1x_vptr_menu_base:
            return stype_menu_base;

        case t1x_vptr_dyn_menu:
        case t1x_vptr_weapon_upgrades_menu:
        case t1x_vptr_menu:
            return stype_menu;

        case t1x_vptr_item_submenu:
        case t1x_vptr_task_item_submenu:
        case t1x_vptr_subtask_item_submenu:
            return stype_itemsubmenu;

        case t1x_vptr_base_component:
        case t1x_vptr_bool_item:
        case t1x_vptr_exec_item:
        case t1x_vptr_alt_exec_item:
        case t1x_vptr_float_item:
        case t1x_vptr_decimal_item:
        case t1x_vptr_separator_item:
        case t1x_vptr_string_item:
        case t1x_vptr_string_list_item:
        case t1x_vptr_task_history_item:
        case t1x_vptr_task_item:
        case t1x_vptr_timer_item:
        case t1x_vptr_user_item:
            return stype_item;

        
            


        /*
        case uc4r_vptr_menu_base:
            return stype_menu_base;

        case uc4r_vptr_dyn_menu:
        case uc4r_vptr_unk_menu:
        case uc4r_vptr_id_menu:
            return stype_menu;

        case uc4r_vptr_item_submenu:
        case uc4r_vptr_task_item_submenu:
        case uc4r_vptr_noreturn_task_item_submenu:
            return stype_itemsubmenu;

        case uc4r_vptr_bool_item:
        case uc4r_vptr_exec_item:
        case uc4r_vptr_alt_exec_item:
        case uc4r_vptr_float_item:
        case uc4r_vptr_decimal_item:
        case uc4r_vptr_separator_item:
        case uc4r_vptr_string_list_item:
        case uc4r_vptr_string_item:
        case uc4r_vptr_task_history_item:
        case uc4r_vptr_task_item:
        case uc4r_vptr_timer_item:
        case uc4r_vptr_user_item:
        case uc4r_vptr_unk_item:
            return stype_item;
        
            




        case tllr_vptr_menu_base:
            return stype_menu_base;
        
        
        case tllr_vptr_dyn_menu:
        case tllr_vptr_unk_menu:
        case tllr_vptr_id_menu:
            return stype_menu;
        
        case tllr_vptr_item_submenu:
        case tllr_vptr_task_item_submenu:
        case tllr_vptr_noreturn_task_item_submenu:
            return stype_itemsubmenu;
        
        case tllr_vptr_bool_item:
        case tllr_vptr_exec_item:
        case tllr_vptr_alt_exec_item:
        case tllr_vptr_float_item:
        case tllr_vptr_decimal_item:
        case tllr_vptr_separator_item:
        case tllr_vptr_string_list_item:
        case tllr_vptr_string_item:
        case tllr_vptr_task_history_item:
        case tllr_vptr_task_item:
        case tllr_vptr_timer_item:
        case tllr_vptr_user_item:
        case tllr_vptr_unk_item:
            return stype_item;
        */


        default:
            // echo("Unknown value read when getting dmenu item type: %lld", chk);
            return -1;
    }
}








//! TODO: Wait, I thought returning pointers to local strings was a no-no
// Convert the name of the provided vptr to it's string representation, without the prefixed the game id and type name (assuming I haven't removed the latter)
const char * get_dmenu_entry_type_as_string(dmenu_vptr_t type)
{
#ifdef DEBUG
    switch (type)
    {
        case t2r_vptr_base_component:
        case t1x_vptr_base_component:
            return "base_component";

        case t2r_vptr_bool_item:
        case t1x_vptr_bool_item:
            return "bool_item";
        
        case t2r_vptr_exec_item:
        case t1x_vptr_exec_item:
            return "exec_item";
    
        case t2r_vptr_alt_exec_item:
        case t1x_vptr_alt_exec_item:
            return "alt_exec_item";
    
        case t2r_vptr_float_item:
        case t1x_vptr_float_item:
            return "float_item";
    
        case t2r_vptr_decimal_item:
        case t1x_vptr_decimal_item:
            return "decimal_item";
    
        case t2r_vptr_separator_item:
        case t1x_vptr_separator_item:
            return "separator_item";
    
        case t2r_vptr_string_list_item:
        case t1x_vptr_string_list_item:
            return "string_list_item";
    
        case t2r_vptr_task_history_item:
        case t1x_vptr_task_history_item:
            return "task_history_item";
    
        case t2r_vptr_task_item:
        case t1x_vptr_task_item:
            return "task_item";
    
        case t2r_vptr_user_item:
        case t1x_vptr_user_item:
            return "user_item";
    
        case t2r_vptr_timer_item:
        case t1x_vptr_timer_item:
            return "timer_item";
    
        case t2r_vptr_checkpoint_item:
        case t1x_vptr_checkpoint_item:
            return "checkpoint_item";


    
        case t2r_vptr_string_item:
        case t1x_vptr_string_item:
            return "string_item";
    
    
        case t2r_vptr_item_submenu:
        case t1x_vptr_item_submenu:
            return "item_submenu";
    
        case t2r_vptr_task_item_submenu:
        case t1x_vptr_task_item_submenu:
            return "task_item_submenu";
    
        case t2r_vptr_noreturn_task_item_submenu:
        case t1x_vptr_subtask_item_submenu:
            return "noreturn_task_item_submenu";

    

        case t2r_vptr_menu:
        case t1x_vptr_menu:
            return "menu";

        case t2r_vptr_dyn_menu:
        case t1x_vptr_dyn_menu:
            return "dyn_menu";

        case t2r_vptr_weapon_upgrades_menu:
        case t1x_vptr_weapon_upgrades_menu:
            return "weapon_upgrades_menu";

        case t2r_vptr_menu_base:
        case t1x_vptr_menu_base:
            return "menu_base";
    }
    
    // echo("Unknown type provided when trying to get dmenu entry type as string: %llX", type);
    return "Unknown Type";
#else
    return " ";
#endif
}







// Recursively check every item and submenu for an item matching the specified dmenu simple type and component name
static void * find_dmenu_component(ndgame_t * ndgame, const char * expected_entry_name, dmenu_stype_t expected_base_type, dmenu_menu_index_t parent_index)
{
    
    if (parent_index < 0 || parent_index > 2)
    {
        echo("Invalid parent menu index provided, defaulting to dev menu");
        parent_index = 0;
    }
    
    
    
    dmenu_menu_base_t parent_base;
    load_menu_base(ndgame, *(&ndgame->dev_menu_base + parent_index), &parent_base);

    if (parent_base.root_menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided. (%s)", parent_base.menu_base_name ? parent_base.menu_base_name : (char *) &parent_base.menu_base_name_buffer);
        return nullptr;
    }
    
    
    dmenu_menu_t menu;
    read_bytes(ndgame->pid, sizeof(dmenu_menu_t), parent_base.root_menu, &menu);
    
    char parent_name[0x40];
    void * entry_ptr;

    bzero(parent_name, 0x40);
    read_string(ndgame->pid, menu.menu_name, parent_name);

    if((entry_ptr = menu.first_item) == 0)
    {
        echo("menu %s contains no items, aborting", parent_name);
        return nullptr;
    }
    
    echo("Searching %s for %s", parent_name, expected_entry_name);
    
    
    char next_is_menu = 0;

    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    dmenu_item_t item;
    char entry_name[0x80];
    
    dmenu_stype_t base_type;
    

    
    int depth = 0, ret;
    while(entry_ptr != nullptr)
    {
        base_type = get_dmenu_entry_base_type(ndgame, entry_ptr);
        
        // Menu
        if (next_is_menu)
        {
            ret = read_bytes(ndgame->pid, sizeof(dmenu_menu_t), entry_ptr, &menu);
            if (ret)
            {
                break;
            }

            ret = read_string(ndgame->pid, menu.menu_name, entry_name);
            if (!ret)
            {
                break;
            }


            if (expected_base_type == base_type && !strncmp(expected_entry_name, entry_name, strlen(entry_name)))
            {
                return entry_ptr;
            }


            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            read_bytes(ndgame->pid, sizeof(dmenu_item_t), entry_ptr, &item);
            
            if (item.item_text == nullptr)
            {
                goto no_text;
            }
            
            read_string(ndgame->pid, item.item_text, entry_name);


            if (expected_base_type == base_type && !strncmp(expected_entry_name, entry_name, strlen(entry_name)))
            {
                return entry_ptr;
            }


            no_text:
            if (base_type == stype_itemsubmenu)
            {
                entry_ptr = (dmenu_item_t *) item.return_buffer;

                history[++depth] = item.next_item;

                next_is_menu = 1;
            }
            else{
                entry_ptr = item.next_item;
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
dmenu_item_t * find_item(ndgame_t * nd_game, const char * item_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(nd_game, item_name, stype_item, parent_index);
}




// Get the address of the first menu in the desired parent menu for which the entry name (from the item_text ptr) matches the provided menu_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_menu_t * find_menu(ndgame_t * nd_game, const char * menu_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(nd_game, menu_name, stype_menu, parent_index);
}




// Get the address of the first submenu item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided submenu_name
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_item_t * find_submenu(ndgame_t * nd_game, const char * submenu_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(nd_game, submenu_name, stype_itemsubmenu, parent_index);
}




// Get the root menu of the specified menu base without loading the base in to memory first
dmenu_menu_t * get_root_menu(ndgame_t * ndgame, dmenu_menu_index_t menu_base_index)
{
    return ((dmenu_menu_t *) read_ulong(ndgame->pid, (unsigned long *) &(* (dmenu_menu_base_t **) ((unsigned long) &ndgame->dev_menu_base + menu_base_index * sizeof(dmenu_menu_base_t *)))->root_menu));
}








// Copy the dmenu_menu_base_t pointed to by the dmenu_menu_base_t * at the provided address in the traced process' memory into the provided menu base struct
int load_menu_base(ndgame_t * ndgame, void * address, dmenu_menu_base_t * menu_base)
{
    return read_bytes(ndgame->pid, sizeof(dmenu_menu_base_t), (void *) address, menu_base);
}




// Copy the dmenu_menu_t at the provided address in the traced process' memory into the provided menu struct
int load_menu(ndgame_t * ndgame, void * address, dmenu_menu_t * menu)
{
    return read_bytes(ndgame->pid, sizeof(dmenu_menu_t), address, menu);
}




// Copy the dmenu_item_t at the provided address in the traced process' memory into the provided item struct
int load_item(ndgame_t * ndgame, void * address, dmenu_item_t * item)
{
    return read_bytes(ndgame->pid, sizeof(dmenu_item_t), address, item);
}








static void do_dump_menu_entries(ndgame_t * ndgame, dmenu_menu_base_t * parent_base)
{
    if (parent_base->root_menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided.");
        return;
    }


    dmenu_menu_t menu;
    read_bytes(ndgame->pid, sizeof(dmenu_menu_t), parent_base->root_menu, &menu);
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).\n# Aborting.", get_dmenu_entry_type_as_string(*(unsigned long long *) &menu), menu.menu_name, entry_ptr);
    }




    
    
    
    int depth = 0;
    char padding[0x40] = { 0x20 }; // used for indentation during output

    dmenu_menu_t * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char item_name[0x80];
    char parent_name[0x80];
    
    char next_is_menu = 0;
    unsigned short unknown_type_count = 0;
    
    dmenu_item_t item;
    dmenu_vptr_t type;
    const char * dtype;

    while(entry_ptr != null)
    {
        
        
        dtype = get_dmenu_entry_type_as_string(type);
        // Check for unknown dmenu component types
        if (((unsigned long long) ( *(unsigned long long *) dtype)) == 2336936577129475669) // just the start of the "Unknown value..." string read as an unsigned 64bit integer
        {
            unknown_type_count++;
        }
        
        
        
        type = (dmenu_vptr_t) read_ulong(ndgame->pid, entry_ptr);

        // Menu
        if (next_is_menu)
        {
            bzero(&menu, sizeof(menu));
            load_menu(ndgame, entry_ptr, &menu);
            
            read_string(ndgame->pid, menu.menu_name, item_name);
            read_string(ndgame->pid, (void *) read_ulong(ndgame->pid, (unsigned long *) &menu.parent_menu->menu_name), parent_name);

            echo("0x%llX %s%s->%s (type: %s)", entry_ptr, padding, parent_name, item_name, dtype);
            echo(
                "%s[\n%s  vPtr: %llX\n%s  Unknown Pointer: 0x%llX\n%s  Unknown Bool: 0x%llX\n%s  Menu Callback: 0x%llX\n%s  Menu Variable: 0x%llX\n%s  Menu Index: 0x%llX\n%s]\n",
                padding, padding, type, padding, menu.unknown_function_ptr, padding, menu.unknown_bool, padding, menu.callback, padding, menu.menu_variable, padding, menu.unknown_menu_array_index, padding);

            
            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            bzero(&item, sizeof(item));
            load_item(ndgame, entry_ptr, &item);

            if (item.item_text)
            {
                read_string(ndgame->pid, item.item_text, item_name);
            }
            else {
                strcpy(item_name, "--------------------------------");
            }

            read_string(ndgame->pid, (void *) read_ulong(ndgame->pid, (unsigned long *) &item.parent_menu->menu_name), parent_name);

            echo("0x%llX %s%s->%s (type: %s)", entry_ptr, padding, parent_name, item_name, dtype);
            echo(
                "%s[\n%s  vPtr: %llX\n%s  Unknown Pointer: 0x%llX\n%s  ItemOrEH TypeID: 0x%llX\n%s  Event Handler: 0x%llX\n%s  Item Callback: 0x%llX\n%s  Item Variable: 0x%llX\n%s  Item Index: 0x%llX\n%s]\n",
                padding, padding, type, padding, item.unknown_function_ptr, padding, item.event_handler_type_id, padding, item.event_handler, padding, item.callback, padding, item.item_variable, padding, item.unknown_item_index, padding);


            

            // Handle ItemSubmenu components
            if (get_dmenu_entry_base_type(ndgame, entry_ptr) == stype_itemsubmenu)
            {
                entry_ptr = (dmenu_item_t *) item.return_buffer;

                history[++depth] = (dmenu_menu_t *) item.next_item;
                
                next_is_menu = 1;


                 // Write padding for output formatting
                for(int i = 0;;)
                {
                    *(padding + i++) = ' ';

                    if (i >= depth * 4)
                    {
                        *(padding + i) = 0; // Terminate new padding string
                        break;
                    }
                }
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
        
            *(padding + depth * 4) = 0; // truncate padding

            next_is_menu = 0;
        }
    }

    echo("\n# Scan finished. Unknown item types: %d", unknown_type_count);
}




void dump_menu_entries(ndgame_t * ndgame)
{
    echo("\n\nDumping Dev Menu entries.");
    dmenu_menu_base_t base;
    load_menu_base(ndgame, ndgame->dev_menu_base, &base);
    do_dump_menu_entries(ndgame, &base);
    echo("Finished dumping Dev Menu entries\n\n\n");

    echo("Dumping Quick Menu entries.");
    load_menu_base(ndgame, ndgame->quick_menu_base, &base);
    do_dump_menu_entries(ndgame, &base);
    echo("Finished dumping Quick Menu entries\n\n");
}








int AppendItemToParent(ndgame_t * ndgame, dmenu_menu_t * parent_menu, dmenu_item_t * item)
{
    dmenu_item_t
        **next_item_p = &parent_menu->first_item,
        *current_item = NULL,
        *prev_item
    ;



    // get the last item in the menu, if it has any contents
    do {
        prev_item = current_item;
        current_item = (dmenu_item_t *) read_ulong(ndgame->pid, (unsigned long *) next_item_p);
        next_item_p = &current_item->next_item;
        
    } while (current_item != NULL);
    
    if (prev_item == NULL)
    {
        // Default to the menu's first item if it has no entries
        next_item_p = &parent_menu->first_item;
    }
    else{
        next_item_p = &prev_item->next_item;
    }


    
    int ret;
    ret = write_ulong(ndgame->pid, (unsigned long *) next_item_p, (unsigned long) item);
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }

    // Assign parent menu and set next item
    write_ulong(ndgame->pid, (unsigned long *) &item->next_item, 0);
    write_ulong(ndgame->pid, (unsigned long *) &item->parent_menu, (u_long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint(ndgame->pid, (unsigned int *) &parent_menu->item_count, read_uint(ndgame->pid, &parent_menu->item_count) + 1);
    write_uint(ndgame->pid, (unsigned int *) &parent_menu->unknown_bool, 1);
    
    char _menu_is_open;
    _menu_is_open = read_byte(ndgame->pid, (byte *) &parent_menu->menu_is_open);

    if (_menu_is_open)
    {
        echo("menu is open");
        void * update_menu;
        if (!(update_menu = (void *) read_ulong(ndgame->pid, (unsigned long *) read_ulong(ndgame->pid, (unsigned long *) &item->vptr) + 0x18)))
        {
            echo("Unable to read item's vPtr!!");
            return 1;
        }
        
        
        struct reg args;

        ret = proc_set_args(ndgame, &args, (uintptr_t) item, 0, 0, 0, 0, 0, 0);
        if (ret)
        {
            echo("ERROR @ Line: %d", __LINE__ - 3);
            return 1;
        }


        ret = proc_call(ndgame, update_menu, nullptr, &args);
        if (ret)
        {
            echo("ERROR @ Line: %d", __LINE__ - 3);
            return 1;
        }
    }
    echo("menu not open");

    return 0;
}















//! TODO: Write the base menu item creation function and move the optimization to that. I don't want bzero being optimized out.
int add_novis_option(ndgame_t * ndgame, dmenu_menu_index_t menu_base)
{
    dmenu_item_t * check;
    if ((check = find_item(ndgame, "Disable All Visibility", menu_base)))
    {
        echo("Novis already added to specified menu base");
        return 0;
    }
    echo("Adding new novis optiom");

    void * mem;    
    mem = proc_malloc(ndgame, align(sizeof(dmenu_item_t)));
    __rerr(!mem, "Error allocating space for novis option", 1);
    proc_wait(ndgame, false);





    dmenu_item_t item;
    bzero(&item, sizeof(dmenu_item_t));

    * (unsigned long *) &item = t2r_vptr_bool_item;


    item.event_handler_type_id = 3; // Bool item
    item.event_handler = (void *) 0x0114d100; // Bool event handler
    item.return_buffer = read_byte(ndgame->pid, (byte *) ndgame->dat_novis);

    item.off_state_colour = 0xB0FFB0FF; // White
    item.on_state_colour = 0xEF00FFFF;  // Yellow
    item.neutral_state_colour = 0xB0FFB0FF; // Red
    item.selection_index = 1;
    item.item_count = 1;

    
    strcpy(item.item_text_buffer, "Disable All Visibility");

    item.item_text = (char *) &((dmenu_item_t *) mem)->item_text_buffer;

    item.item_variable = ndgame->dat_novis;



    int ret;
    ret = write_bytes(ndgame->pid, sizeof(dmenu_item_t), mem, &item);
    __rerr(ret, "Error writing new menu option to allocated memory, aborting novis option addition.", 1);

    return AppendItemToParent(ndgame, get_root_menu(ndgame, menu_base), mem);
}

