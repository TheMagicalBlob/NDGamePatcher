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





// dmenu_item_t * find_item(pid_t pid, dmenu_menu_base_t * parent)
dmenu_item_t * find_item(pid_t pid, dmenu_menu_base_t * parent, const char * item_name)
{
    dmenu_menu_base_t menu_base;
    read_bytes(pid, sizeof(menu_base), parent, &menu_base);
    
    if (menu_base.menu == NULL)
    {
        echo("ERROR: menu base %s's menu pointer was null. cannot find item.", &menu_base.item_text_buffer);
        return nullptr;
    }

    dmenu_menu_t current_menu;
    read_bytes(pid, sizeof(dmenu_menu_t), menu_base.menu, &current_menu);
    if (current_menu.first_item == NULL)
    {
        echo("ERROR: menu %s's initial item pointer was null. cannot find item.", &current_menu.item_text_buffer);
        return nullptr;
    }


    char * string_pointer = null; //! CHANGE TO A POINTER TO THE ITEM ITSELF ONCE DONE TESTING
    void * next_item;
    dmenu_item_t current_item;
    
    next_item = current_menu.first_item;
    do {
        if (next_item == nullptr)
        {
            echo("ERROR: null pointer for next item.");
            return nullptr;
        }



        int base_type = get_dmenu_entry_base_type(pid, next_item);

        if (base_type == dmenu_basetype_menu || base_type == dmenu_basetype_dyn_menu)
        {
            read_bytes(pid, sizeof(dmenu_menu_t), next_item, &current_menu);

            string_pointer = &current_menu.item_text_buffer;


            next_item = current_menu.first_item;
            bzero(&current_menu, sizeof(current_menu));
            continue;
        }


        if (base_type == dmenu_basetype_item)
        {
            item:
            read_bytes(pid, sizeof(dmenu_item_t), next_item, &current_item);

            string_pointer = &current_item.item_text_buffer;

            next_item = current_item.next_item;
            bzero(&current_item, sizeof(current_item));
            continue;
        }


        if (base_type == dmenu_basetype_submenu)
        {
            echo("WARNING: Item is submenu entry item. I forget whether these are mapped correctly rn");
            goto item;
        } 

    }
    while (!strcmp(string_pointer, item_name));

    echo("FOUND IT BIHH (@%llX)", string_pointer);

    return string_pointer;
}




void dump_menu_entries(pid_t pid, dmenu_menu_base_t * menu)
{
    if (menu->menu != 0)
    {
        dmenu_menu_t current_menu;
        read_bytes(pid, sizeof(dmenu_menu_t), menu->menu, &current_menu);
        
        dmenu_item_t * item_ptr;
        if((item_ptr = current_menu.first_item) == 0)
        {
            echo("ERROR:\n# %s has no entries (%d).\n# Aborting.", &current_menu.item_text, item_ptr);
        }
        
        
        
        int unknown_type_count = 0;
        
        void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
        
        int depth = 0;
        char next_item_should_be_submenu = 0;
        
        dmenu_item_t item;
        dmenu_menu_t menu;
        
        while(item_ptr != null)
        {
            // TODO: Handle empty sumbenus
            int type = read_uint(pid, item_ptr);
            char * dtype = get_dmenu_entry_type_as_string(type);

            if (((unsigned long long) ( *(unsigned long long *) dtype)) == 2336936577129475669)
            {
                unknown_type_count++;
            }
            
            
            
            echo("Type: %d (%s)", type, dtype);
            echo("Address: %llx", item_ptr);

            if (next_item_should_be_submenu) // Submenu
            {
                bzero(&item, sizeof(item));
                read_bytes(pid, sizeof(dmenu_menu_t), item_ptr, &menu);
                
                echo("Submenu Name: %s (item count: %d)", read_string(pid, menu.item_text), menu.item_count);
                
                item_ptr = menu.first_item;
                next_item_should_be_submenu = 0;



                if (type != dmenu_id_menu && type != dmenu_id_dyn_menu && type != dmenu_id_unk_menu)
                {
                    echo("WARNING: read type does not match known menu types. (%lld)", type);
                }
            }
            else {
                bzero(&menu, sizeof(menu));
                read_bytes(pid, sizeof(dmenu_item_t), item_ptr, &item);


                char * str;
                if (item.item_text == nullptr)
                {
                    str = "------------------------------------";
                }
                else{
                    str = read_string(pid, item.item_text);
                }
                


                echo("Entry Name: %s", str);
                
                if (type == dmenu_id_submenu_item || type == dmenu_id_task_submenu_item)
                {
                    item_ptr = (dmenu_item_t *) item.item_variable_buffer;

                    history[++depth] = item.next_item;
                    next_item_should_be_submenu = 1;

                }
                else{
                    item_ptr = item.next_item;
                    next_item_should_be_submenu = 0;
                }
                
            }

            while (item_ptr == nullptr && depth >= 0)
            {
                echo("Returning to parent menu (depth %d)", depth);

                item_ptr = history[depth];
                history[depth--] = nullptr;

                next_item_should_be_submenu = 0;
            }

            echo("");
        }

        echo("\n# Scan finished. Unknown item types: %d", unknown_type_count);
    }
    else{
        echo("ERROR: Menu pointer was still null.\n");
    }
}
