/* Copyright (C) 2023 John Törnblom

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.  */

#include "inc/memory.h"
#include "inc/dmenu.h"

#define MODULE_NAME "t2p.prx"


void log_message(char * base_string, ...);
#define printf(s, ...) { /*klog_printf(s, __VA_ARGS__);*/ log_message(s, __VA_ARGS__); }



typedef struct notify_request {
    char unused[45];
    char message[3075];
} notify_request_t;



typedef struct app_info
{
    uint32_t app_id;
    uint64_t unknown1;
    char     title_id[14];
    char     unknown2[0x3c];

} app_info_t;




extern int sceKernelSendNotificationRequest(int, notify_request_t*, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);

static int logfile;

int echo(char * str)
{
    size_t len = strlen(str);
    char msg[len + 2];
    len = snprintf(msg, sizeof(msg), "%s\n", str);

    printf(msg, "");

    return len;
}



void notify(const char * message, ...)
{
    va_list list;
    notify_request_t request = { };

    // Build string with variable arguments
    va_start(list, message);
    vsnprintf(request.message, sizeof(request.message) - 1, message, list);
    va_end(list);
    
    sceKernelSendNotificationRequest(0, &request, sizeof(request), 0); // Not sure what the first and last arg are for. One may be notification type though
    
    
    printf("[Notification]: %s\n", request.message);
}


void log_message(char * base_string, ...)
{
    va_list args;
    va_start(args, base_string);

    char message[4000];
    vsnprintf(message, sizeof(message), base_string, args);
    
    va_end(args);


    if (logfile < 0) { // failed to create it, don't wanna retry every print
        return;
    }

    if (logfile == null)
    {
        logfile = open("/data/blog.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (logfile < 0)
        {
            printf("Unable to open logfile, maybe error code: %s", strerror(logfile));
        }
    }


    // Write the message to the log file
    write(logfile, message, strlen(message));
}



static int get_dmenu_item_type(pid_t pid, void * item)
{
    unsigned long long chk = (unsigned long long) read_ulong(pid, item);
    switch (chk)
    {
        case bool_item:
        case exec_item:
        case alt_exec_item:
        case float_item:
        case decimal_item:
        case separator_item:
        case string_list_item:
        case string_item:
            return dmenu_item;
        
        case submenu_item:
            return dmenu_submenu;

        case menu_base:
            return dmenu_base;


        case menu:
            return dmenu_menu;
        
        case dyn_menu:
            return dmenu_dyn_menu;

            
        case unk1_menu:
            return dmenu_unk1_menu;

        default:
            printf("Unknown value read when getting dmenu item type: %lld", chk);
            return -1;
    }
}



char * debug_get_entry_type(long long type)
{
    switch (type)
    {
        case bool_item: return "bool_item";
        case exec_item: return "exec_item";
        case alt_exec_item: return "alt_exec_item";
        case float_item: return "float_item";
        case decimal_item: return "decimal_item";
        case separator_item: return "separator_item";
        case string_list_item: return "string_list_item";

        case string_item: return "string_item";
        
        case submenu_item: return "submenu_item";

        case menu_base: return "menu_base";

        case menu: return "menu";

        case dyn_menu: return "dyn_menu";

        case unk1_menu: return "unk1_menu";
    }


    char retval[0x40];
    snprintf(retval, sizeof(retval), "Unknown value read when getting dmenu item type: %lld", type);
    echo(retval);
    return "Unknown value";
}





// static dmenu_item_t * find_item(pid_t pid, dmenu_menu_base_t * parent)
static char * find_item(pid_t pid, dmenu_menu_base_t * parent, const char * item_name)
{
    dmenu_menu_base_t menu_base;
    read_bytes(pid, sizeof(menu_base), parent, &menu_base);
    
    if (menu_base.menu == NULL)
    {
        printf("ERROR: menu base %s's menu pointer was null. cannot find item.", &menu_base.item_text_buffer);
        return nullptr;
    }

    dmenu_menu_t current_menu;
    read_bytes(pid, sizeof(dmenu_menu_t), menu_base.menu, &current_menu);
    if (current_menu.first_item == NULL)
    {
        printf("ERROR: menu %s's initial item pointer was null. cannot find item.", &current_menu.item_text_buffer);
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



        int type = get_dmenu_item_type(pid, next_item);

        if (type == dmenu_menu || type == dmenu_dyn_menu)
        {
            read_bytes(pid, sizeof(dmenu_menu_t), next_item, &current_menu);

            string_pointer = current_menu.item_text_buffer;


            next_item = current_menu.first_item;
            bzero(&current_menu, sizeof(current_menu));
            continue;
        }


        if (type == dmenu_item)
        {
            item:
            read_bytes(pid, sizeof(dmenu_item_t), next_item, &current_item);

            string_pointer = current_item.item_text_buffer;



            next_item = current_item.next_item;
            bzero(&current_item, sizeof(current_item));
            continue;
        }


        if (type == dmenu_submenu)
        {
            echo("WARNING: Item is submenu entry item. I forget whether these are mapped correctly rn");
            goto item;
        } 

    }
    while (!strcmp(string_pointer, item_name));

    printf("FOUND IT BIHH (@%llX)", string_pointer);

    return string_pointer;
}




static void dump_menu_entries(pid_t pid, dmenu_menu_base_t * menu)
{
    if (menu->menu != 0)
    {
        dmenu_menu_t current_menu;
        read_bytes(pid, sizeof(dmenu_menu_t), menu->menu, &current_menu);
        
        dmenu_item_t * item_ptr;
        if((item_ptr = current_menu.first_item) == 0)
        {
            printf("ERROR:\n# %s has no entries (%d).\n# Aborting.", &current_menu.item_text, item_ptr);
        }
        
        
        
        dmenu_item_t item;
        dmenu_menu_t menu;
        int i, item_count;
        
        
        int unknown_type_count = 0;
        void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
        unsigned depth = 0;

        char next_item_should_be_submenu = 0;

        for(i = 0, item_count = current_menu.item_count; i < item_count; i++)
        {
            // TODO: Handle empty sumbenus
            int type = read_uint(pid, item_ptr);
            char * dtype = debug_get_entry_type(type);

            if (((unsigned long long) (* (unsigned long long *) dtype)) == 2336936577129475669)
            {
                unknown_type_count++;
            }
            
            
            
            printf("Type: %d (%s)\n", type, dtype);
            printf("Item: %d \\ %d\n", i, item_count);
            printf("Address: %llx\n", item_ptr);

            if (next_item_should_be_submenu) // Submenu
            {
                bzero(&item, sizeof(item));
                read_bytes(pid, sizeof(dmenu_menu_t), item_ptr, &menu);
                
                printf("Submenu Name: %s (item count: %d)\n", read_string(pid, menu.item_text), menu.item_count);
                
                item_ptr = menu.first_item;
                item_count += menu.item_count + 1; // Iirc the header item isn't counted, so we add one extra to crawl through the whole menu
                next_item_should_be_submenu = 0;



                if (type != 52618672 && type != 52122184 && type != 52592816)
                {
                    printf("WARNING: read type does not match known menu types. (%lld)", type);
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
                


                printf("Entry Name: %s\n", str);
                
                if (type == submenu_item)
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

            if (item_ptr == nullptr)
            {
                item_ptr = history[depth];
                history[depth--] = nullptr;
            }

            echo("");
        }

        printf("\n#SCAN FINISHED:\nUnknown item types: %d\n", unknown_type_count);
    }
    else{
        echo("ERROR: Menu pointer was still null.");
    }
}







static void ptracetst(int module_pid)
{
    int pid;
    
    // Check for any running eboot.bin //! remember to check for the other possible T2R executable names
getPid:
    pid = find_pid("eboot.bin");


    if (pid < 0)
    {
        echo("Unable to find eboot, waiting a few seconds before retrying");
        sleep(7);
        goto getPid;
    }

    char msg[0x40];
    snprintf(msg, sizeof(msg), "Found eboot.bin as pid %d, getting title id...\n", pid); 
    notify(msg);


    app_info_t app_info;
    sceKernelGetAppInfo(pid, &app_info);

    if (!strncmp(app_info.title_id, "PPSA10662", 10))
    {
        int status;
        printf("Status:\n# Game is the last of us part II.\n# Title ID: %s\n# App ID: %d\n", app_info.title_id, app_info.app_id);

        if ((status = set_dbg_perms()) != 0)
        {
            close(logfile);
            return;
        }

        echo("Successfully set permissions.");
        
        if ((status = attach(pid)) != 0)
        {
            close(logfile);
            return;
        }
        notify("Attached to game process.");


        waitpid(pid, 0, 0);
        notify("Process should be suspended, running IO tests.");

        

        
        echo("==================================\n"
             "#  PRINTING QUICK MENU CONTENTS  #\n"
             "==================================");
        dmenu_menu_base_t quick_menu_base;
        read_bytes(pid, sizeof(dmenu_menu_base_t), (void *) read_ulong(pid, DMENU_QUICK_MENU_BASE_PTR), &quick_menu_base);
        dump_menu_entries(pid, &quick_menu_base);
        

        
        echo("\n\n================================\n"
                 "|  PRINTING DEV MENU CONTENTS  |\n"
                 "================================");

        dmenu_menu_base_t dev_menu_base;
        read_bytes(pid, sizeof(dmenu_menu_base_t), (void *) read_ulong(pid, DMENU_DEV_MENU_BASE_PTR), &dev_menu_base);
        dump_menu_entries(pid, &dev_menu_base);
        

        detach(pid);
    }
    else {
        notify("Application Title ID does not match expected value of [PPSA10662]\nTitle ID: %s\nApp ID: %d\n", app_info.title_id, app_info.app_id);
    }

    close(logfile);
}






int main() {
    int pid, len;
    while ((pid = find_pid(MODULE_NAME)) > 0) // Kill any previous instance
    {
        printf("\nKilling old instance (pid: %d)... ", pid);
        len = ALIGN(strlen(MODULE_NAME) + 28);

        if (kill(pid, SIGKILL))
        {
            printf("sigkill err%s", "\n");
            return -1;
        }
        
        char msg[len];
        snprintf(msg, sizeof(msg), "Killed previous %s instance\n", MODULE_NAME);
        notify(msg);
        
        printf("Killed previous %s instance\n", MODULE_NAME);
        return 0;
    }
    
    // Set payload name
    syscall(SYS_thr_set_name, -1, MODULE_NAME);
    

    // Construct and print start message
    len = strlen(MODULE_NAME) + 23;
    char msg[len];
    snprintf(msg, sizeof(msg), "Loaded new %s instance", MODULE_NAME);

    notify(msg);


    // Payload start
    ptracetst(getpid());
}






// extern int32_t module_start(size_t argc, const void* args)
// {
//     main();
//     return 0;
// }

// extern int32_t module_stop(size_t argc, const void* args)
// {
//     return 0;
// }

