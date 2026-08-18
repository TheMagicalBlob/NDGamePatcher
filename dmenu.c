#include "inc/memory.h"
#include "inc/dmenu.h"
#include "inc/proc.h"



int get_dmenu_entry_base_type(ndgame_t * ndgame, void * item)
{
    // echo("int get_dmenu_entry_base_type(ndgame_t * ndgame, void * item)");
    unsigned long long chk = (unsigned long long) read_ulong(ndgame->pid, item);
    // echo("behehee");

    switch (chk)
    {
        case t2r_typeid_menu_base:
            return simple_typeid_menu_base;

        case t2r_typeid_dyn_menu:
        case t2r_typeid_weapon_upgrades_menu:
        case t2r_typeid_id_menu:
            return simple_typeid_menu;

        case t2r_typeid_submenu_item:
        case t2r_typeid_task_submenu_item:
        case t2r_typeid_noreturn_task_submenu_item:
            return simple_typeid_submenu;

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
            





        case t1x_typeid_menu_base:
            return simple_typeid_menu_base;

        case t1x_typeid_dyn_menu:
        case t1x_typeid_weapon_upgrades_menu:
        case t1x_typeid_id_menu:
            return simple_typeid_menu;

        case t1x_typeid_submenu_item:
        case t1x_typeid_task_submenu_item:
        case t1x_typeid_subtask_submenu_item:
            return simple_typeid_submenu;

        case t1x_typeid_bool_item:
        case t1x_typeid_exec_item:
        case t1x_typeid_alt_exec_item:
        case t1x_typeid_float_item:
        case t1x_typeid_decimal_item:
        case t1x_typeid_separator_item:
        case t1x_typeid_string_item:
        case t1x_typeid_string_list_item:
        case t1x_typeid_task_history_item:
        case t1x_typeid_task_item:
        case t1x_typeid_timer_item:
        case t1x_typeid_user_item:
            return simple_typeid_item;

        
            


        /*
        case uc4r_typeid_menu_base:
            return simple_typeid_menu_base;

        case uc4r_typeid_dyn_menu:
        case uc4r_typeid_unk_menu:
        case uc4r_typeid_id_menu:
            return simple_typeid_menu;

        case uc4r_typeid_submenu_item:
        case uc4r_typeid_task_submenu_item:
        case uc4r_typeid_noreturn_task_submenu_item:
            return simple_typeid_submenu;

        case uc4r_typeid_bool_item:
        case uc4r_typeid_exec_item:
        case uc4r_typeid_alt_exec_item:
        case uc4r_typeid_float_item:
        case uc4r_typeid_decimal_item:
        case uc4r_typeid_separator_item:
        case uc4r_typeid_string_list_item:
        case uc4r_typeid_string_item:
        case uc4r_typeid_task_history_item:
        case uc4r_typeid_task_item:
        case uc4r_typeid_timer_item:
        case uc4r_typeid_user_item:
        case uc4r_typeid_unk_item:
            return simple_typeid_item;
        
            




        case tllr_typeid_menu_base:
            return simple_typeid_menu_base;
        
        
        case tllr_typeid_dyn_menu:
        case tllr_typeid_unk_menu:
        case tllr_typeid_id_menu:
            return simple_typeid_menu;
        
        case tllr_typeid_submenu_item:
        case tllr_typeid_task_submenu_item:
        case tllr_typeid_noreturn_task_submenu_item:
            return simple_typeid_submenu;
        
        case tllr_typeid_bool_item:
        case tllr_typeid_exec_item:
        case tllr_typeid_alt_exec_item:
        case tllr_typeid_float_item:
        case tllr_typeid_decimal_item:
        case tllr_typeid_separator_item:
        case tllr_typeid_string_list_item:
        case tllr_typeid_string_item:
        case tllr_typeid_task_history_item:
        case tllr_typeid_task_item:
        case tllr_typeid_timer_item:
        case tllr_typeid_user_item:
        case tllr_typeid_unk_item:
            return simple_typeid_item;
        */


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
        case t1x_typeid_checkpoint_item:
            return "checkpoint_item";


    
        case t2r_typeid_string_item:
        case t1x_typeid_string_item:
            return "string_item";
    
    
        case t2r_typeid_submenu_item:
        case t1x_typeid_submenu_item:
            return "submenu_item";
    
        case t2r_typeid_task_submenu_item:
        case t1x_typeid_task_submenu_item:
            return "task_submenu_item";
    
        case t2r_typeid_noreturn_task_submenu_item:
        case t1x_typeid_subtask_submenu_item:
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

        case t2r_typeid_menu_base:
        case t1x_typeid_menu_base:
            return "menu_base";
    }


    // echo("Unknown type provided when trying to get dmenu entry type as string: %llX", type);
    return "Unknown Type";
}








static void * find_dmenu_component(ndgame_t * nd_game, const char * expected_entry_name, dmenu_simple_type_t expected_base_type, dmenu_menu_index_t parent_index)
{
    dmenu_menu_base_t parent_base;
    dmenu_menu_base_t * p_parent_base;

    if (parent_index < 0 || parent_index > 2)
    {
        echo("Invalid parent menu index provided, defaulting to dev menu");
        parent_index = 0;
    }

    // Determine which menu base pointer to use
    switch(parent_index)
    {
        case 0:
            p_parent_base = nd_game->dev_menu_base;
            break;

        case 1:
            p_parent_base = nd_game->quick_menu_base;
            break;
        
        case 2:
            p_parent_base = nd_game->favorites_menu_base;
            break;
    }


    load_menu_base(nd_game, p_parent_base, &parent_base);

    if (parent_base.root_menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided. (%s)", parent_base.menu_base_name ? parent_base.menu_base_name : (char *) &parent_base.menu_base_name_buffer);
        return nullptr;
    }
    
    
    dmenu_menu_t menu;
    read_bytes(nd_game->pid, sizeof(dmenu_menu_t), parent_base.root_menu, &menu);
    
    char parent_name[0x40];
    void * entry_ptr;

    bzero(parent_name, 0x40);
    read_string(nd_game->pid, menu.menu_name, parent_name);

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
    
    dmenu_simple_type_t base_type;
    

    
    int depth = 0, ret;
    while(entry_ptr != nullptr)
    {
        base_type = get_dmenu_entry_base_type(nd_game, entry_ptr);
        
        // Menu
        if (next_is_menu)
        {
            ret = read_bytes(nd_game->pid, sizeof(dmenu_menu_t), entry_ptr, &menu);
            if (ret)
            {
                break;
            }

            ret = read_string(nd_game->pid, menu.menu_name, entry_name);
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
            read_bytes(nd_game->pid, sizeof(dmenu_item_t), entry_ptr, &item);
            
            if (item.item_text == nullptr)
            {
                goto no_text;
            }
            
            read_string(nd_game->pid, item.item_text, entry_name);


            if (expected_base_type == base_type && !strncmp(expected_entry_name, entry_name, strlen(entry_name)))
            {
                return entry_ptr;
            }


            no_text:
            if (base_type == simple_typeid_submenu)
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


    return nullptr;
}








// Get the address of the first item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided item_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_item_t * find_item(ndgame_t * nd_game, const char * item_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(nd_game, item_name, simple_typeid_item, parent_index);
}



// Get the address of the first menu in the desired parent menu for which the entry name (from the item_text ptr) matches the provided menu_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_menu_t * find_menu(ndgame_t * nd_game, const char * menu_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(nd_game, menu_name, simple_typeid_menu, parent_index);
}



// Get the address of the first submenu item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided submenu_name
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_item_t * find_submenu(ndgame_t * nd_game, const char * submenu_name, dmenu_menu_index_t parent_index)
{
    return find_dmenu_component(nd_game, submenu_name, simple_typeid_submenu, parent_index);
}








// Copy the dmenu_menu_base_t pointed to by the dmenu_menu_base_t * at the provided address in the traced process' memory into the provided menu base struct
int load_menu_base(ndgame_t * ndgame, void * address, dmenu_menu_base_t * menu_base)
{
    return read_bytes(ndgame->pid, sizeof(dmenu_menu_base_t), (void *) read_ulong(ndgame->pid, (unsigned long *) address), menu_base);
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








void dump_menu_entries(ndgame_t * ndgame, dmenu_menu_base_t * parent_base)
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
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char * output_format = "0x%llX %s%s->%s  [Type: %s, vPtr: %llX, EventHandler: 0x%llX (id?: %d)]";
    char padding[0x40]; // used for indentation during output
    
    char item_name[0x80];
    char parent_name[0x80];
    
    char next_is_menu = 0;
    int unknown_type_count = 0;
    
    dmenu_item_t item;
    
    while(entry_ptr != null)
    {
        dmenu_vptr_t type = (dmenu_vptr_t) read_ulong(ndgame->pid, entry_ptr);
        char * dtype = get_dmenu_entry_type_as_string(type);
        

        // Check for unknown dmenu component types
        if (((unsigned long long) ( *(unsigned long long *) dtype)) == 2336936577129475669) // just the start of the "Unknown value..." string read as an unsigned 64bit integer
        {
            unknown_type_count++;
        }
        

        bzero(&item, sizeof(item));
        bzero(&menu, sizeof(menu));
        
        // Menu
        if (next_is_menu)
        {
            load_menu(ndgame, entry_ptr, &menu);
            
            read_string(ndgame->pid, menu.menu_name, item_name);
            read_string(ndgame->pid, (void *) read_ulong(ndgame->pid, (unsigned long *) &menu.parent_menu->menu_name), parent_name);

            echo(output_format, entry_ptr, padding, parent_name, item_name, dtype, type, -1, -1);
            
            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            load_item(ndgame, entry_ptr, &item);

            if (item.item_text)
            {
                read_string(ndgame->pid, item.item_text, item_name);
            }
            else {
                strcpy(item_name, "--------------------------------");
            }

            read_string(ndgame->pid, (void *) read_ulong(ndgame->pid, (unsigned long *) &item.parent_menu->menu_name), parent_name);

            echo(output_format, entry_ptr, padding, parent_name, item_name, dtype, type, item.event_handler, item.entry_event_handler_type_id);

            
            if (get_dmenu_entry_base_type(ndgame, entry_ptr) == simple_typeid_submenu)
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






//! TODO: Write the base menu item creation function and move the optimization to that. I don't want bzero being optimized out.
int add_novis_option(ndgame_t * ndgame, dmenu_menu_index_t menu_base)
{
    echo("Checking for already-present novis option.");
    dmenu_item_t * check;
    if ((check = find_item(ndgame, "Disable All Visibility", menu_base)))
    {
        echo("Novis already added to specified menu base");
        return 0;
    }
    echo("Option not found, adding new optiom");


    int ret;
    struct reg regs;
    ret = proc_get_regs(ndgame->pid, &regs);
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }



    // set malloc arguments
    ret = set_args(&regs, align(sizeof(dmenu_item_t)), 0, 0, 0, 0, 0, 0);
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }

    // call malloc
    void * mem;
    ret = proc_call(ndgame, ndgame->func_malloc, (unsigned long *) &mem, &regs);
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }

    proc_wait(ndgame, false);








    dmenu_item_t item;
    bzero(&item, sizeof(dmenu_item_t));

    * (unsigned int *) &item = t2r_typeid_bool_item;


    item.entry_event_handler_type_id = 3; // Bool item
    item.event_handler = (void *) 0x0114d100; // Bool event handler
    item.item_event_buffer = read_byte(ndgame->pid, ndgame->dat_novis);

    item.off_state_colour = 0xEFFFFFFF; // White
    item.on_state_colour = 0xEF00FFFF;  // Yellow
    item.unknown_state_colour_1 = 0xFF0000FF; // Red
    item.unknown_state_colour_2 = 0xFF00FF00; // Green
    item.selection_index = 1;
    item.item_count = 1;

    
    strcpy(item.item_text_buffer, "Disable All Visibility");

    item.item_text = (char *) &((dmenu_item_t *) mem)->item_text_buffer;

    item.item_variable = ndgame->dat_novis;

    echo("Writing new item to allocated spot 0x%llX", mem);

    ret = write_bytes(ndgame->pid, sizeof(dmenu_item_t), mem, &item);
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }


    ret = proc_get_regs(ndgame->pid, &regs);
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }

    
    uintptr_t one;
    one = (uintptr_t) ((dmenu_menu_t *) read_ulong(ndgame->pid, (unsigned long *) &((dmenu_menu_base_t *) read_ulong(ndgame->pid, (unsigned long *) T2RDMENU_QUICK_MENU_BASE_PTR))->root_menu));


    set_args(&regs, one, (uintptr_t) mem, 0, 0, 0, 0, 0);

    ret = proc_call(ndgame, (void *) 0x10e7fe0, NULL, &regs);
    if (ret)
    {
        echo("ERROR @ Line: %d", __LINE__ - 3);
        return 1;
    }


    return 0;
}

