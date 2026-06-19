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

#define printf(s, ...) klog_printf(s, __VA_ARGS__)



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




int echo(char * str)
{
    size_t len = strlen(str);
    char msg[len + 2];
    len = snprintf(msg, sizeof(msg), "%s\n", str);

    puts(msg);
    klog_printf(msg);

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
            return;
        }

        echo("Successfully set permissions.");
        
        if ((status = attach(pid)) != 0)
        {
            return;
        }
        notify("Attached to game process.");


        waitpid(pid, 0, 0);
        notify("Process should be suspended, running IO tests.");

        


        

        dmenu_menu_base_t quick_menu_base;
        read_bytes(pid, sizeof(dmenu_menu_base_t), (void *) read_ulong(pid, DMENU_QUICK_MENU_BASE_PTR), &quick_menu_base);

        printf("Check: %s\n", &quick_menu_base.item_text_buffer);
        printf("menu_base.Menu ptr val: %llx\n", quick_menu_base.menu);

        if (quick_menu_base.menu != 0)
        {
            dmenu_menu_t quick_menu;
            read_bytes(pid, sizeof(dmenu_menu_t), quick_menu_base.menu, &quick_menu);
            
            if (strcmp(quick_menu.item_text_buffer, "Quick Menu"))
            {
                printf("ERROR: Unexpected item text (%s != %s)", quick_menu.item_text_buffer, "Quick Menu");
            }
            printf("\n# Quick Menu:\n# Count %d\n", quick_menu.item_count);



            int i, item_count;
            dmenu_item_t * item_ptr;
            dmenu_item_t item;
            for(i = 0, item_count += quick_menu.item_count; i < item_count; i++)
            {
                if((item_ptr = quick_menu.first_item) == 0)
                {
                    printf("Skipping empty submenu %s with no entries (%d)\n", &quick_menu.item_text, item_ptr);
                    continue;
                }
                
                read_bytes(pid, sizeof(item), item_ptr, &item);

                printf("Item: %s\n", item.item_text_buffer);
            }
        }
        else{
            echo("ERROR: Menu pointer was still null.");
        }


        detach(pid);
    }
    else {
        notify("Application Title ID does not match expected value of [PPSA10662]\nTitle ID: %s\nApp ID: %d\n", app_info.title_id, app_info.app_id);
    }
}






int main() {
    // Set payload name
    syscall(SYS_thr_set_name, -1, MODULE_NAME);

    int pid, len;
    if ((pid = find_pid(MODULE_NAME)) > 0) // Kill any previous instance
    {
        printf("\nKilling old instance (pid: %d)... ", pid);
        len = strlen(MODULE_NAME) + 28;

        if (kill(pid, SIGKILL))
        {
            echo("sigkill err");
            return -1;
        }
        
        char msg[len];
        snprintf(msg, sizeof(msg), "Killed previous %s instance", MODULE_NAME);
        
        echo(msg);
        notify(msg);
        return 0;
    }

    

    // Construct and print start message
    len = strlen(MODULE_NAME) + 23;
    char msg[len];
    snprintf(msg, sizeof(msg), "Loaded new %s instance", MODULE_NAME);

    notify(msg);
    echo(msg);


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

