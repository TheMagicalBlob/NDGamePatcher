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

        case dmenu_id_dyn_menu:
        case dmenu_id_unk_menu:
        case dmenu_id_menu:
            return dmenu_basetype_menu;

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
    snprintf(retval, sizeof(retval), "_Unknown value read when getting dmenu item type: %lld", type);
    echo(retval);
    return "Unknown value";
}


dmenu_menu_base_t * load_menu_base(pid_t pid, void * address)
{
    static dmenu_menu_base_t menu_base;
    read_bytes(pid, sizeof(dmenu_menu_base_t), (void *) read_ulong(pid, (unsigned long *) address), &menu_base);

    return &menu_base;
}


dmenu_menu_t * load_menu(pid_t pid, void * address)
{
    static dmenu_menu_t menu;
    read_bytes(pid, sizeof(dmenu_menu_t), address, &menu);
    
    return &menu;
}


dmenu_item_t * load_item(pid_t pid, void * address)
{
    static dmenu_item_t item;
    read_bytes(pid, sizeof(dmenu_item_t), address, &item);

    return &item;
}





static void * find_dmenu_entry(pid_t pid, const char * expected_entry_name, dmenu_menu_base_t * parent_base, dmenu_item_base_types_t expected_base_type, size_t len)
{
    if (parent_base->menu == 0)
    {
        echo("ERROR: Empty menu base pointer provided.");
        return nullptr;
    }


    dmenu_menu_t menu;
    read_bytes(pid, sizeof(dmenu_menu_t), parent_base->menu, &menu);
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        return nullptr;
    }
    
    
    
    char next_is_menu = 0;

    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size

    dmenu_item_t item;
    char entry_name[0x80];
    
    while(entry_ptr != null)
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

            // if (base_type == expected_base_type && !strncmp(expected_entry_name, entry_name, len))
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


            // if (expected_base_type == base_type && !strncmp(expected_entry_name, entry_name, len))
            if (expected_base_type == base_type && !strcmp("Disable FPS", entry_name))
            {
                return entry_ptr;
            }


            no_text:
            if (type == dmenu_id_submenu_item || type == dmenu_id_task_submenu_item || type == dmenu_id_noreturn_task_submenu_item)
            {
                entry_ptr = (dmenu_item_t *) item.item_variable_buffer;

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




dmenu_item_t * find_item(pid_t pid, dmenu_menu_base_t * parent, const char * item_name)
{
    return find_dmenu_entry(pid, item_name, parent, dmenu_basetype_item, 0x80);
}

dmenu_menu_t * find_menu(pid_t pid, dmenu_menu_base_t * parent, const char * item_name)
{
    return find_dmenu_entry(pid, item_name, parent, dmenu_basetype_menu, strnlen(item_name, 0x80));
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
    
    void * entry_ptr;
    if((entry_ptr = menu.first_item) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).\n# Aborting.", get_dmenu_entry_type_as_string(*(unsigned long long *) &menu), &menu.item_text, entry_ptr);
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
    
    while(entry_ptr != null)
    {
        unsigned int type = read_ulong(pid, entry_ptr);
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
            read_bytes(pid, sizeof(dmenu_menu_t), entry_ptr, &menu);
            
            read_string(pid, menu.item_text, item_name);
            read_string(pid, (void *) read_ulong(pid, (unsigned long *) &menu.parent_menu->item_text), parent_name);

            echo(output_format, entry_ptr, padding, item_name, dtype, type, parent_name);
            
            // Read next item pointer
            entry_ptr = menu.first_item;
            next_is_menu = 0;
        }
        // Item
        else {
            read_bytes(pid, sizeof(dmenu_item_t), entry_ptr, &item);
            
            if (item.item_text == nullptr)
            {
                strncpy(item_name, "------------------------------------", 40);
            }
            else{
                read_string(pid, item.item_text, item_name);
            }



            read_string(pid, (void *) read_ulong(pid, (unsigned long *) &item.parent_menu->item_text), parent_name);

            echo(output_format, entry_ptr, padding, item_name, dtype, type, parent_name);
            
            if (type == dmenu_id_submenu_item || type == dmenu_id_task_submenu_item || type == dmenu_id_noreturn_task_submenu_item)
            {
                entry_ptr = (dmenu_item_t *) item.item_variable_buffer;

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
            echo(""); // just for formatting
        }
    }

    echo("\n# Scan finished. Unknown item types: %d", unknown_type_count);
}
