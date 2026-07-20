#include "inc/memory.h"
#include "inc/dmenu.h"



int get_dmenu_entry_base_type(pid_t pid, void * item)
{
    unsigned long long chk = (unsigned long long) read_ulong(pid, item);

    switch (chk)
    {
        case t2r_typeid_menu_base:
            return simple_typeid_menu_base;

        case t2r_typeid_bool_item:
        case t2r_typeid_exec_item:
        case t2r_typeid_alt_exec_item:
        case t2r_typeid_float_item:
        case t2r_typeid_decimal_item:
        case t2r_typeid_separator_item:
        case t2r_typeid_string_list_item:
        case t2r_typeid_string_item:
        case t2r_typeid_task_history_item:
        case t2r_typeid_task_item:
        case t2r_typeid_timer_item:
        case t2r_typeid_user_item:
        case t2r_typeid_checkpoint_item:
            return simple_typeid_item;
        
        case t2r_typeid_submenu_item:
        case t2r_typeid_task_submenu_item:
        case t2r_typeid_noreturn_task_submenu_item:
            return simple_typeid_submenu;

        case t2r_typeid_dyn_menu:
        case t2r_typeid_weapon_upgrades_menu:
        case t2r_typeid_id_menu:
            return simple_typeid_menu;

            




        case t1x_typeid_menu_base:
            return simple_typeid_menu_base;

        case t1x_typeid_bool_item:
        case t1x_typeid_exec_item:
        case t1x_typeid_alt_exec_item:
        case t1x_typeid_float_item:
        case t1x_typeid_decimal_item:
        case t1x_typeid_separator_item:
        case t1x_typeid_string_list_item:
        case t1x_typeid_task_history_item:
        case t1x_typeid_task_item:
        case t1x_typeid_timer_item:
        case t1x_typeid_user_item:
            return simple_typeid_item;

        case t1x_typeid_submenu_item:
        case t1x_typeid_task_submenu_item:
            return simple_typeid_submenu;

        case t1x_typeid_dyn_menu:
        case t1x_typeid_weapon_upgrades_menu:
        case t1x_typeid_id_menu:
            return simple_typeid_menu;

        
            



        // case uc4r_typeid_menu_base:
        //     return simple_typeid_menu_base;

        // case uc4r_typeid_bool_item:
        // case uc4r_typeid_exec_item:
        // case uc4r_typeid_alt_exec_item:
        // case uc4r_typeid_float_item:
        // case uc4r_typeid_decimal_item:
        // case uc4r_typeid_separator_item:
        // case uc4r_typeid_string_list_item:
        // case uc4r_typeid_string_item:
        // case uc4r_typeid_task_history_item:
        // case uc4r_typeid_task_item:
        // case uc4r_typeid_timer_item:
        // case uc4r_typeid_user_item:
        // case uc4r_typeid_unk_item:
        //     return simple_typeid_item;
        
        // case uc4r_typeid_submenu_item:
        // case uc4r_typeid_task_submenu_item:
        // case uc4r_typeid_noreturn_task_submenu_item:
        //     return simple_typeid_submenu;

        // case uc4r_typeid_dyn_menu:
        // case uc4r_typeid_unk_menu:
        // case uc4r_typeid_id_menu:
        //     return simple_typeid_menu;

        
            



        // case tllr_typeid_menu_base:
        //     return simple_typeid_menu_base;
        
        // case tllr_typeid_bool_item:
        // case tllr_typeid_exec_item:
        // case tllr_typeid_alt_exec_item:
        // case tllr_typeid_float_item:
        // case tllr_typeid_decimal_item:
        // case tllr_typeid_separator_item:
        // case tllr_typeid_string_list_item:
        // case tllr_typeid_string_item:
        // case tllr_typeid_task_history_item:
        // case tllr_typeid_task_item:
        // case tllr_typeid_timer_item:
        // case tllr_typeid_user_item:
        // case tllr_typeid_unk_item:
        //     return simple_typeid_item;
        
        // case tllr_typeid_submenu_item:
        // case tllr_typeid_task_submenu_item:
        // case tllr_typeid_noreturn_task_submenu_item:
        //     return simple_typeid_submenu;
        
        // case tllr_typeid_dyn_menu:
        // case tllr_typeid_unk_menu:
        // case tllr_typeid_id_menu:
        //     return simple_typeid_menu;


        default:
            // echo("Unknown value read when getting dmenu item type: %lld", chk);
            return -1;
    }
}



char * get_dmenu_entry_type_as_string(long long type)
{
    switch (type)
    {
        case t2r_typeid_bool_item:
        case t1x_typeid_bool_item:
            return "bool_item";
        
        case t2r_typeid_exec_item:
        case t1x_typeid_exec_item:
            return "exec_item";
    
        case t2r_typeid_alt_exec_item:
        case t1x_typeid_alt_exec_item:
            return "alt_exec_item";
    
        case t2r_typeid_float_item:
        case t1x_typeid_float_item:
            return "float_item";
    
        case t2r_typeid_decimal_item:
        case t1x_typeid_decimal_item:
            return "decimal_item";
    
        case t2r_typeid_separator_item:
        case t1x_typeid_separator_item:
            return "separator_item";
    
        case t2r_typeid_string_list_item:
        case t1x_typeid_string_list_item:
            return "string_list_item";
    
        case t2r_typeid_task_history_item:
        case t1x_typeid_task_history_item:
            return "task_history_item";
    
        case t2r_typeid_task_item:
        case t1x_typeid_task_item:
            return "task_item";
    
        case t2r_typeid_user_item:
        case t1x_typeid_user_item:
            return "user_item";
    
        case t2r_typeid_timer_item:
        case t1x_typeid_timer_item:
            return "timer_item";
    
        case t2r_typeid_checkpoint_item:
            return "checkpoint_item";


    
        case t2r_typeid_string_item:
            return "string_item";
    
    
        case t2r_typeid_submenu_item:
        case t1x_typeid_submenu_item:
            return "submenu_item";
    
        case t2r_typeid_task_submenu_item:
        case t1x_typeid_task_submenu_item:
            return "task_submenu_item";
    
        case t2r_typeid_noreturn_task_submenu_item:
            return "noreturn_task_submenu_item";

    

        case t2r_typeid_id_menu:
        case t1x_typeid_id_menu:
            return "menu";

        case t2r_typeid_dyn_menu:
        case t1x_typeid_dyn_menu:
            return "dyn_menu";

        case t2r_typeid_weapon_upgrades_menu:
        case t1x_typeid_weapon_upgrades_menu:
            return "weapon_upgrades_menu";

        // case t1x_typeid_menu_base:
        // case t1x_typeid_menu_base:
            return "menu_base";
    }


    // echo("Unknown type provided when trying to get dmenu entry type as string: %llX", type);
    return "Unknown Type";
}




// Copy the dmenu_menu_base_t at the provided address in the traced process' memory into the provided menu base struct
int load_menu_base(pid_t pid, void * address, dmenu_menu_base_t * menu_base)
{
    return read_bytes(pid, sizeof(dmenu_menu_base_t), (void *) read_ulong(pid, (unsigned long *) address), menu_base);
}


// Copy the dmenu_menu_t at the provided address in the traced process' memory into the provided menu struct
int load_menu(pid_t pid, void * address, dmenu_menu_t * menu)
{
    return read_bytes(pid, sizeof(dmenu_menu_t), address, menu);
}


// Copy the dmenu_item_t at the provided address in the traced process' memory into the provided item struct
int load_item(pid_t pid, void * address, dmenu_item_t * item)
{
    return read_bytes(pid, sizeof(dmenu_item_t), address, item);
}





static void * find_dmenu_entry(pid_t pid, const char * expected_entry_name, dmenu_menu_base_t * parent_base, dmenu_simple_type_t expected_base_type, size_t len)
{
    if (parent_base->menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided.");
        return nullptr;
    }


    char parent_name[0x40];
    read_string(pid, parent_base->item_text, parent_name);
    echo("Loading from parent \"%s\"", parent_name);

    dmenu_menu_t menu;
    read_bytes(pid, sizeof(dmenu_menu_t), parent_base->menu, &menu);
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        echo("menu contains no items, aborting");
        return nullptr;
    }
    
    
    bzero(parent_name, 0x40);
    read_string(pid, menu.item_text, parent_name);
    echo("Searching menu \"%s\"", parent_name);
    
    
    char next_is_menu = 0;

    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size

    dmenu_item_t item;
    char entry_name[0x80];
    
    while(entry_ptr != nullptr)
    {
        int base_type = get_dmenu_entry_base_type(pid, entry_ptr);
        unsigned int type = read_ulong(pid, entry_ptr);
        
        
        bzero(&item, sizeof(item));
        bzero(&menu, sizeof(menu));
        
        // Menu
        if (next_is_menu)
        {
            read_bytes(pid, sizeof(dmenu_menu_t), entry_ptr, &menu);
            
            read_string(pid, menu.item_text, entry_name);

            if (expected_base_type == base_type && !strncmp(expected_entry_name, entry_name, len))
            {
                return entry_ptr;
            }


            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            read_bytes(pid, sizeof(dmenu_item_t), entry_ptr, &item);
            
            if (item.item_text == nullptr)
            {
                goto no_text;
            }
            
            read_string(pid, item.item_text, entry_name);


            if (expected_base_type == base_type && !strncmp(expected_entry_name, entry_name, len))
            {
                return entry_ptr;
            }


            no_text:
            if (type == t2r_typeid_submenu_item || type == t2r_typeid_task_submenu_item || type == t2r_typeid_noreturn_task_submenu_item)
            {
                entry_ptr = (dmenu_item_t *) item.item_event_buffer;

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


    echo("Unable to find item \"%s\"", expected_entry_name);
    return nullptr;
}




// Get the address of the first item for which the item name (from the item_text ptr) matches the provided item_name 
dmenu_item_t * find_item(pid_t pid, dmenu_menu_base_t * parent, const char * item_name)
{
    return find_dmenu_entry(pid, item_name, parent, simple_typeid_item, 0x80);
}

// Get the address of the first menu for which the item name (from the item_text ptr) matches the provided menu_name 
dmenu_menu_t * find_menu(pid_t pid, dmenu_menu_base_t * parent, const char * menu_name)
{
    return find_dmenu_entry(pid, menu_name, parent, simple_typeid_menu, 0x80);
}

// Get the address of the first submenu item for which the item name (from the item_text ptr) matches the provided submenu_name
dmenu_item_t * find_submenu(pid_t pid, dmenu_menu_base_t * parent, const char * submenu_name)
{
    return find_dmenu_entry(pid, submenu_name, parent, simple_typeid_submenu, 0x80);
}



void dump_menu_entries(nd_game_t nd_game, dmenu_menu_base_t * parent_base)
{
    if (parent_base->menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided.");
        return;
    }


    dmenu_menu_t menu;
    read_bytes(nd_game.pid, sizeof(dmenu_menu_t), parent_base->menu, &menu);
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).\n# Aborting.", get_dmenu_entry_type_as_string(*(unsigned long long *) &menu), menu.item_text, entry_ptr);
    }
    
    
    
    
    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char * output_format = "0x%llX %sItem: %s [Type: %s (%lX) | Parent: %s]";
    char padding[0x40]; // used for indentation during output
    
    char item_name[0x80];
    char parent_name[0x80];
    
    char next_is_menu = 0;
    int unknown_type_count = 0;
    
    dmenu_item_t item;
    
    while(entry_ptr != null)
    {
        dmenu_typeid_t type = (dmenu_typeid_t) read_ulong(nd_game.pid, entry_ptr);

        char * dtype = get_dmenu_entry_type_as_string(type);
        

        if (((unsigned long long) ( *(unsigned long long *) dtype)) == 2336936577129475669) // just the start of the "Unknown value..." string read as an unsigned 64bit integer
        {
            unknown_type_count++;
        }
        

        bzero(&item, sizeof(item));
        bzero(&menu, sizeof(menu));
        
        // Menu
        if (next_is_menu)
        {
            read_bytes(nd_game.pid, sizeof(dmenu_menu_t), entry_ptr, &menu);
            
            read_string(nd_game.pid, menu.item_text, item_name);
            read_string(nd_game.pid, (void *) read_ulong(nd_game.pid, (unsigned long *) &menu.parent_menu->item_text), parent_name);

            echo(output_format, entry_ptr, padding, item_name, dtype, type, parent_name);
            
            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            read_bytes(nd_game.pid, sizeof(dmenu_item_t), entry_ptr, &item);
            
            if (item.item_text == nullptr)
            {
                strncpy(item_name, "------------------------------------", 40);
            }
            else{
                read_string(nd_game.pid, item.item_text, item_name);
            }



            read_string(nd_game.pid, (void *) read_ulong(nd_game.pid, (unsigned long *) &item.parent_menu->item_text), parent_name);

            echo(output_format, entry_ptr, padding, item_name, dtype, type, parent_name);
            
            if (get_dmenu_entry_base_type(nd_game.pid, entry_ptr) == simple_typeid_submenu)
            {
                entry_ptr = (dmenu_item_t *) item.item_event_buffer;

                history[++depth] = item.next_item;
                
                for(int i = 0;;) // Write padding
                {
                    *(padding + i++) = ' ';

                    if (i >= depth * 4)
                    {
                        *(padding + i) = 0;
                        break;
                    }
                }

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
        
            *(padding + depth * 4) = 0; // truncate padding

            next_is_menu = 0;
        }
    }

    echo("\n# Scan finished. Unknown item types: %d", unknown_type_count);
}




int t2_example_call(pid_t pid)
{
    int ret;
    unsigned long long check; 
    struct reg args;
    if ((ret = get_regs(pid, &args)) != 0)
    {
        echo("Unable to get register values, aborting 'cause something's clearly wrong.");
        return ret;
    }



    dmenu_menu_base_t * fav = (dmenu_menu_base_t *) read_ulong(pid, (u_long *) T2RDMENU_FAVORITES_MENU_BASE_PTR);
    args.r_rdi = (long long) &fav->open_menu;
    args.r_rsi = (long long) !read_byte(pid, &fav->open_menu);

    if ((ret = proc_call(pid, (unsigned long *) 0x16c7bf0, (unsigned long *) &check, &args)) != 0)
    {
        echo("ERROR: %s call failed! (ret: %d)", __FUNCTION__);
    }

    return ret;
}





void add_novis_option(nd_game_t * nd_game)
{
    dmenu_item_t * item_ptr;
    item_ptr = find_item(nd_game->pid, &nd_game->dev_menu_base, "Draw World Axes");
    if (!item_ptr)
    {
        if ((item_ptr = find_item(nd_game->pid, &nd_game->dev_menu_base, "novis")))
        {
            //! testing thing, remove
            {
                echo("\nTemporary address check %llX | %llX\n", read_ulong(nd_game->pid, (unsigned long *) &item_ptr->item_variable_ptr), (unsigned long) nd_game->novis_address);
            }

            echo("novis already present");
            return;
        }

        echo("Unable to find item to replace with novis.");
        return;
    }


    dmenu_item_t item;
    load_item(nd_game->pid, item_ptr, &item);

    write_string(nd_game->pid, item.item_text, "Enable novis");
    write_ulong(nd_game->pid, (unsigned long *) &item_ptr->item_variable_ptr, (unsigned long) nd_game->novis_address);

    echo("Novis option added, hopefully");
}
