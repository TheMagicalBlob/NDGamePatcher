#include "inc/memory.h"
#include "inc/dmenu.h"




int get_dmenu_entry_base_type(pid_t pid, void * item)
{
    unsigned long long chk = (unsigned long long) read_ulong(pid, item);

    switch (chk)
    {
        case dmenu_id_menu_base:
            return dmenu_basetype_menu_base;

        case dmenu_id_bool_item:
        case dmenu_id_exec_item:
        case dmenu_id_alt_exec_item:
        case dmenu_id_float_item:
        case dmenu_id_decimal_item:
        case dmenu_id_separator_item:
        case dmenu_id_string_list_item:
        case dmenu_id_string_item:
        case dmenu_id_task_history_item:
        case dmenu_id_task_item:
        case dmenu_id_timer_item:
        case dmenu_id_user_item:
        case dmenu_id_unk_item:
            return dmenu_basetype_item;
        
        case dmenu_id_submenu_item:
        case dmenu_id_task_submenu_item:
        case dmenu_id_noreturn_task_submenu_item:
            return dmenu_basetype_submenu;

        case dmenu_id_menu:
            return dmenu_basetype_menu;
        
        case dmenu_id_dyn_menu:
            return dmenu_basetype_dyn_menu;

        case dmenu_id_unk_menu:
            return dmenu_basetype_unk_menu;

        default:
            echo("Unknown value read when getting dmenu item type: %lld", chk);
            return -1;
    }
}



char * get_dmenu_entry_type_as_string(long long type)
{
    switch (type)
    {
        case dmenu_id_bool_item: return "bool_item";
        case dmenu_id_exec_item: return "exec_item";
        case dmenu_id_alt_exec_item: return "alt_exec_item";
        case dmenu_id_float_item: return "float_item";
        case dmenu_id_decimal_item: return "decimal_item";
        case dmenu_id_separator_item: return "separator_item";
        case dmenu_id_string_list_item: return "string_list_item";
        case dmenu_id_task_history_item: return "task_history_item";
        case dmenu_id_task_item: return "task_item";
        case dmenu_id_user_item: return "user_item";
        case dmenu_id_timer_item: return "timer_item";
        case dmenu_id_unk_item: return "unk_item";

        case dmenu_id_string_item: return "string_item";
        
        case dmenu_id_submenu_item: return "submenu_item";
        case dmenu_id_task_submenu_item: return "task_submenu_item";
        case dmenu_id_noreturn_task_submenu_item: return "noreturn_task_submenu_item";

        case dmenu_id_menu_base: return "menu_base";

        case dmenu_id_menu: return "menu";

        case dmenu_id_dyn_menu: return "dyn_menu";

        case dmenu_id_unk_menu: return "unk_menu";
    }


    char retval[0x40];
    snprintf(retval, sizeof(retval), "Unknown value read when getting dmenu item type: %lld", type);
    echo(retval);
    return "Unknown value";
}




void * find_dmenu_entry(pid_t pid, const char * entry_name, dmenu_menu_base_t * parent_base, dmenu_type_ids_t item_type)
{
    if (parent_base->menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided.");
        return;
    }


    dmenu_menu_t menu;
    read_bytes(pid, sizeof(dmenu_menu_t), parent_base->menu, &menu);
    
    dmenu_item_t * item_ptr;
    if((item_ptr = menu.first_item) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).", get_dmenu_entry_type_as_string(*(unsigned long long *) &menu), &menu.item_text, item_ptr);
    }
    
    
    
    
    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char item_name[0x80];
    
    char next_is_menu = 0;
    
    dmenu_item_t item;
    
    while(item_ptr != null)
    {
        int type = read_uint(pid, item_ptr);

        bzero(&item, sizeof(item));
        bzero(&menu, sizeof(menu));
        
        // Menu
        if (next_is_menu)
        {
            read_bytes(pid, sizeof(dmenu_menu_t), item_ptr, &menu);
            
            read_string(pid, menu.item_text, item_name);

            // Read next item pointer
            item_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            read_bytes(pid, sizeof(dmenu_item_t), item_ptr, &item);
            
            if (item.item_text == nullptr)
            {
                strncpy(item_name, "------------------------------------", 40);
            }
            else{
                read_string(pid, item.item_text, item_name);
            }



            if (type == dmenu_id_submenu_item || type == dmenu_id_task_submenu_item || type == dmenu_id_noreturn_task_submenu_item)
            {
                item_ptr = (dmenu_item_t *) item.item_variable_buffer;

                history[++depth] = item.next_item;

                next_is_menu = 1;
            }
            else{
                item_ptr = item.next_item;
                next_is_menu = 0;
            }
        }
        
        


        while (item_ptr == nullptr && depth >= 0)
        {
            item_ptr = history[depth];
            history[depth--] = nullptr;
        
            next_is_menu = 0;
        }
    }

    echo("Unablr to find item \"%s\"", item_name);
}




dmenu_item_t * find_item(pid_t pid, dmenu_menu_base_t * parent, const char * item_name)
{
    return nullptr;
}

dmenu_item_t * find_menu(pid_t pid, dmenu_menu_base_t * parent, const char * item_name)
{
    return nullptr;
}




void dump_menu_entries(pid_t pid, dmenu_menu_base_t * parent_base)
{
    if (parent_base->menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided.");
        return;
    }


    dmenu_menu_t menu;
    read_bytes(pid, sizeof(dmenu_menu_t), parent_base->menu, &menu);
    
    dmenu_item_t * item_ptr;
    if((item_ptr = menu.first_item) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).\n# Aborting.", get_dmenu_entry_type_as_string(*(unsigned long long *) &menu), &menu.item_text, item_ptr);
    }
    
    
    
    
    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char * output_format = "0x%llX %sItem: %s [Type: %s (%d) | Parent: %s]";
    char padding[0x40]; // used for indentation during output
    
    char item_name[0x80];
    char parent_name[0x80];
    
    char next_is_menu = 0;
    int unknown_type_count = 0;
    
    dmenu_item_t item;
    
    while(item_ptr != null)
    {
        int type = read_uint(pid, item_ptr);
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
            read_bytes(pid, sizeof(dmenu_menu_t), item_ptr, &menu);
            
            read_string(pid, menu.item_text, item_name);
            read_string(pid, (void *) read_ulong(pid, &menu.parent_menu->item_text), parent_name);

            echo(output_format, item_ptr, padding, item_name, dtype, type, parent_name);
            
            // Read next item pointer
            item_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            read_bytes(pid, sizeof(dmenu_item_t), item_ptr, &item);
            
            if (item.item_text == nullptr)
            {
                strncpy(item_name, "------------------------------------", 40);
            }
            else{
                read_string(pid, item.item_text, item_name);
            }



            read_string(pid, (void *) read_ulong(pid, &item.parent_menu->item_text), parent_name);

            echo(output_format, item_ptr, padding, item_name, dtype, type, parent_name);
            
            if (type == dmenu_id_submenu_item || type == dmenu_id_task_submenu_item || type == dmenu_id_noreturn_task_submenu_item)
            {
                item_ptr = (dmenu_item_t *) item.item_variable_buffer;

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
                item_ptr = item.next_item;
                next_is_menu = 0;
            }
            
        }
        
        


        while (item_ptr == nullptr && depth >= 0)
        {
            item_ptr = history[depth];
            history[depth--] = nullptr;
        
            *(padding + depth * 4) = 0; // truncate padding

            next_is_menu = 0;
            echo(""); // just for formatting
        }
    }

    echo("\n# Scan finished. Unknown item types: %d", unknown_type_count);
}
