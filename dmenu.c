#include "inc/include.h"
#include "inc/dmenu.h"
#include "inc/memory.h"
#include "inc/proc.h"

extern NdGame_t * game;



// Read the vPtr at the start of the provided item, and return the simple type of the option to simplify type checks (i.e. checking for stype_item rather than 14 different types)
DMENU_stype_t DMENU_GetBaseType(void * item)
{
    void * chk;
    chk = (void *) read_ulong(game->pid, item);

    if (chk == game->Types.DMENU_Types_MenuGroup)
        return stype_menu_group;
    
    if (chk == game->Types.DMENU_Types_Menu ||
        chk == game->Types.DMENU_Types_DynMenu ||
        chk == game->Types.DMENU_Types_WeaponUpgradesMenu)
        return stype_menu;

    if (chk == game->Types.DMENU_Types_ItemSubmenu ||
        chk == game->Types.DMENU_Types_TaskItemSubmenu ||
        chk == game->Types.DMENU_Types_SubnodeTaskItemSubmenu)
        return stype_itemsubmenu;

    if (chk == game->Types.DMENU_Types_BaseComponent ||
        chk == game->Types.DMENU_Types_SeparatorItem ||
        chk == game->Types.DMENU_Types_ToggleItem ||
        chk == game->Types.DMENU_Types_ExecItem ||
        chk == game->Types.DMENU_Types_AltExecItem ||
        chk == game->Types.DMENU_Types_StringItem ||
        chk == game->Types.DMENU_Types_StringListItem ||
        chk == game->Types.DMENU_Types_DecimalItem ||
        chk == game->Types.DMENU_Types_TaskHistoryItem ||
        chk == game->Types.DMENU_Types_TaskItem ||
        chk == game->Types.DMENU_Types_UserItem ||
        chk == game->Types.DMENU_Types_TimerItem ||
        chk == game->Types.DMENU_Types_CheckpointItem ||
        chk == game->Types.DMENU_Types_FloatItem)
        return stype_item;


    return -1;
}








// [DEBUG-ONLY] Convert the name of the provided vptr to it's string representation, without the prefixed the game id and type name (assuming I haven't removed the latter)
const char * DMENU_GetTypeAsString(dmenu_vptr_t type)
{
    static char return_string[0x20];
    memset(return_string, 0, sizeof(return_string));
    strncpy(return_string, "unknown_type", strlen("unknown_type"));
    
#ifdef DEBUG
    int count;

    const char * names[] = 
    {
        "MenuGroup",
        "Menu",
        "DynMenu",
        "WeaponUpgradesMenu",
        "ItemSubmenu",
        "TaskItemSubmenu",
        "SubnodeTaskItemSubmenu",
        "BaseComponent",    
        "SeparatorItem",
        "ToggleItem",
        "ExecItem",
        "AltExecItem",
        "StringItem",
        "StringListItem",
        "DecimalItem",
        "TaskHistoryItem",
        "TaskItem",
        "UserItem",
        "TimerItem",
        "CheckpointItem",
        "FloatItem"
    };

    const dmenu_vptr_t types[] =
    {
        game->Types.DMENU_Types_MenuGroup,
        game->Types.DMENU_Types_Menu,
        game->Types.DMENU_Types_DynMenu,
        game->Types.DMENU_Types_WeaponUpgradesMenu,
        game->Types.DMENU_Types_ItemSubmenu,
        game->Types.DMENU_Types_TaskItemSubmenu,
        game->Types.DMENU_Types_SubnodeTaskItemSubmenu,
        game->Types.DMENU_Types_BaseComponent,
        game->Types.DMENU_Types_SeparatorItem,
        game->Types.DMENU_Types_ToggleItem,
        game->Types.DMENU_Types_ExecItem,
        game->Types.DMENU_Types_AltExecItem,
        game->Types.DMENU_Types_StringItem,
        game->Types.DMENU_Types_StringListItem,
        game->Types.DMENU_Types_DecimalItem,
        game->Types.DMENU_Types_TaskHistoryItem,
        game->Types.DMENU_Types_TaskItem,
        game->Types.DMENU_Types_UserItem,
        game->Types.DMENU_Types_TimerItem,
        game->Types.DMENU_Types_CheckpointItem,
        game->Types.DMENU_Types_FloatItem
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
static void * DMENU_FindComponent(const char * expected_component_name, dmenu_menu_t * start_menu, DMENU_stype_t expected_component_stype, char recurse)
{
    __rerrv(start_menu < (dmenu_menu_t *) 1, nullptr, "Invalid start menu pointer provided (0x%llX)", start_menu)


    int ret;
    dmenu_menu_t * menu;
    if (start_menu == nullptr)
    {
        DMENU_menu_index_t curr_base_index;
        dmenu_menu_group_t * parent_group;
    
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

        
        parent_group = *(&game->Addrs.dev_menu_group + curr_base_index);
        if (parent_group == null)
        {
            echo("ERROR: Null parent base in ndgame struct at game->Addrs.dev_menu_group + %d, aborting", curr_base_index);
            return nullptr;
        }


        menu = (dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &parent_group->root_menu);
        __rerrv(menu == (dmenu_menu_t *) -1, nullptr, "error reading root menu from menu base #%d", curr_base_index)
        if (menu == 0)
        {
            echo("ERROR: Null menu base doesn't even have a root menu! Aborting.");
            return nullptr;
        }
    }
    else{
        menu = start_menu;
    }
    

    

    

    void * pItem;

    pItem = (void *) read_ulong(game->pid, (unsigned long *) &menu->FirstItem);

#ifdef DEBUG
    // Read current menu name before checking for an empty menu
    char parent_name[0xFF];
    ret = read_string(game->pid, (const char *) read_ulong(game->pid, (unsigned long *) &menu->pMenuName), parent_name);
    __rerrv(ret == -1, nullptr, "ERROR: Failed reading name of root menu from menu base \"%s\", aborting.", expected_component_name)
    if (pItem == nullptr)
    {
        echo("%s contains no items, aborting", parent_name);
        return nullptr;
    }

    echo("Searching %s for %s", parent_name, expected_component_name);
#else
    if(pItem == 0)
    {
        return nullptr;
    }
#endif

    
    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size

    char next_is_menu;

    DMENU_stype_t base_type;
    char entry_name[0xFF];
    const char * entry_name_ptr;
    
    next_is_menu = 0;
    while(pItem != nullptr)
    {
        base_type = DMENU_GetBaseType(pItem);
        
        entry_name_ptr = (const char *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) pItem)->pItemText);
        if ((void *) entry_name_ptr == nullptr)
        {
            goto _no_name;
        }
        

        ret = read_string(game->pid, entry_name_ptr, entry_name);
        if (ret == -1)
        {
            __rerrv(ret != sizeof(dmenu_menu_t), nullptr, "ERROR: Failed reading name from component at 0x%llX when searching for component \"%s\", aborting.", pItem, expected_component_name)
            break;
        }

        // Check for matching base type and component name
        if (expected_component_stype == base_type && !strncmp(expected_component_name, entry_name, strlen(entry_name)))
        {
            return pItem;
        }

        
        _no_name:
        // Menu
        if (next_is_menu)
        {
            // Read next item pointer
            pItem = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_menu_t *) pItem)->FirstItem);
            next_is_menu = 0;
        }
        // Item
        else {
            if (base_type == stype_itemsubmenu && recurse > 0)
            {
                // Item Submenu
                pItem = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) pItem)->ReturnBuffer);

                history[++depth] = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) pItem)->NextItem);

                next_is_menu = 1;
            }
            else{
                pItem = (void *) read_ulong(game->pid, (unsigned long *) &((dmenu_item_t *) pItem)->NextItem);
                next_is_menu = 0;
            }
        }
        
        if (recurse < 1)
        {
            continue;
        }


        while (pItem == nullptr && depth >= 0)
        {
            pItem = history[depth];
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
    return DMENU_FindComponent(item_name, start_menu, stype_item, recurse);
}




// Get the address of the first menu in the desired parent menu for which the entry name (from the item_text ptr) matches the provided menu_name 
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_menu_t * find_menu(const char * menu_name, dmenu_menu_t * start_menu, char recurse)
{
    return DMENU_FindComponent(menu_name, start_menu, stype_item, recurse);
}




// Get the address of the first submenu item in the desired parent menu for which the entry name (from the item_text ptr) matches the provided submenu_name
// Parent indexes:
// <=0 == Dev Menu
// 1 == Quick Menu
// >=2 == Favorites Menu
dmenu_item_t * find_submenu(const char * submenu_name, dmenu_menu_t * start_menu, char recurse)
{
    return DMENU_FindComponent(submenu_name, start_menu, stype_item, recurse);
}




// Get the root menu of the specified menu base without loading the base in to memory first
dmenu_menu_t * get_root_menu(DMENU_menu_index_t menu_group_index)
{
    // Why did I write it this way?!
    return ((dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &(* (dmenu_menu_group_t **) ((unsigned long) &game->Addrs.dev_menu_group + menu_group_index * sizeof(dmenu_menu_group_t *)))->root_menu));
}








// Copy the dmenu_menu_group_t pointed to by the dmenu_menu_group_t * at the provided address in the traced process' memory into the provided menu base struct
int load_menu_group(const dmenu_menu_group_t * address, dmenu_menu_group_t * menu_group)
{
    return read_bytes(game->pid, sizeof(dmenu_menu_group_t), (byte *) address, (byte *) menu_group) != sizeof(dmenu_menu_group_t);
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








// do_dump_menu_entries-specific struct
typedef struct {
    char Name[0xFF];       // Component name
    char ParentName[0xFF]; // Component's parent name
    dmenu_vptr_t Type;     // Component "type" (class vPtr)
    char dType[0x40];      // Component Type as String 
    dmenu_item_t * pItem;  // Pointer to item itself in the tracee's memory
    unsigned long ReturnBuffer; // Item event handler return value storage //!
    unsigned long ItemVariable; // Item tag


    // Detailed-Scanner reserved
    size_t CheckCount;            // The amount of items to be checking
    unsigned long * CheckOffsets; // Offsets of the things being checked in the menu/item struct
    unsigned long * CheckBuffers; // Storage for check values
    const char ** CheckNames;      // What it says on the can.
    
} DMENU_scanner_t;

// do_dump_menu_entries-specific function
static void PrintScannerContents(DMENU_scanner_t Scanner[], size_t ScannerCount, const char * MenuGroupName)
{
    if (MenuGroupName == nullptr)
    {
        MenuGroupName = "<unnamed>";
    }


    echo("");
    for (unsigned i = 0; i < ScannerCount; i++)
    {
        echo(
            "\n# /%s/%s/%s\n# {\n#     0x%llX\n#     Type: 0x%llX (%s)\n#     Item Variable: 0x%llX\n#     Return Buffer: 0x%llX\n# }",

            MenuGroupName, Scanner[i].ParentName, Scanner[i].Name, (unsigned long long) Scanner[i].pItem, (unsigned long long) Scanner[i].Type, Scanner[i].dType, Scanner[i].ItemVariable, Scanner[i].ReturnBuffer
        );

        if (Scanner[i].CheckCount > 0)
        {
            char scannerOut[0x1FC] = {"# DR Scanner:"};

            for (unsigned j = 0; j < Scanner[i].CheckCount; j++)
            {
                if (Scanner[i].CheckBuffers[j] != 0)
                {
                    snprintf(&scannerOut[strnlen(scannerOut, sizeof(scannerOut) - 1)], sizeof(scannerOut), "\n# %s: 0x%llX = 0x%llX", Scanner[i].CheckNames[j], (unsigned long long) Scanner[i].CheckOffsets[j], (unsigned long long) Scanner[i].CheckBuffers[j]);
                }
            }
            echo("%s\n", (char *) scannerOut);
        }
        else{
            echo("\n");
        }
    }
    echo("");
}

//! Unoptimized menu entry info dumper. May fail if partially-stripped menus are added, like the SpawnAvObjects menu in T2R 02.001.010 with it's "next_entry" pointer being for some reason set to an address pointing to unrelated volatile memory 
static void do_dump_menu_entries(dmenu_menu_group_t * parent_group)
{
    //#
    //## Basic null checks
    //#
    if (parent_group == 0) // Null menu group pointer
    {
        echo("ERROR: Empty menu base pointer provided.");
        return;
    }
    if (parent_group->root_menu == 0) // Empty menu group
    {
        if (parent_group->name)
        {
            echo("Provided menu base %s has no menus.", parent_group->name);
        }
        else {
            echo("Provided menu base (unnamed) has no menus.");
        }
        return;
    }

    char parentName[0x40] = {"Error"};
    __rerr(read_string(game->pid, (const char *) parent_group->name, parentName) == -1, "Failed to read menu group name before dumping menu entries, aborting. (pMenuGroup: 0x%llX)", parent_group);
    
    

    

    //#
    //## Begin dumper by reading root menu of provided menu group
    //#
    echo("Dumping %s contents.", parentName);
    
    dmenu_menu_t menu;
    read_bytes(game->pid, sizeof(dmenu_menu_t), (byte *) parent_group->root_menu, (byte *) &menu);
    
    void * pItem;
    if((pItem = menu.FirstItem) == 0)
    {
        echo("ERROR:\n# %s %s has no entries (%d).\n# Aborting.", DMENU_GetTypeAsString(menu.vPtr), menu.MenuName, pItem);
        return;
    }




    
    //#
    //## General Menu-Crawling Variables
    //#
    int depth = 0;
    dmenu_menu_t * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char nextIsMenu = 0;
    char newItem = 0;
    
    dmenu_vptr_t type;
    dmenu_item_t item;
    const char * dtype;
    
    // Encoded SID of "unknown_type"
    const SID unknownTypeSID = (SID) 0x9E3E4FA85A926262ul;
    




    //#
    //## Component Scanner Variables
    //#
    #define maxComponentCount 0x40
    DMENU_scanner_t * pCurrComponent; // Scanner ref to avoid having a bunch of if/else statements checking for unknown values.

    //## Basic type checks
    unsigned knownTypeCount = 0;
    DMENU_scanner_t knownComponents[maxComponentCount] = {0};
    
    unsigned unknownTypeCount = 0;
    DMENU_scanner_t unknownComponents[maxComponentCount] = {0};


    //## Reversing checks
    unsigned foundTestMenuCount = 0;
    DMENU_scanner_t foundTestMenus[maxComponentCount] = {0};
    
    unsigned foundTestItemCount = 0;
    DMENU_scanner_t foundTestItems[maxComponentCount] = {0};
    




    //#
    //## Setup detailed scanner's current config
    //#
    
    // Array of offsets in the item and/or menu structures
    unsigned long CheckOffsets[] = {
        __offsetof(dmenu_item_t, ItemVariable),
        __offsetof(dmenu_item_t, Unmapped1)
    };
    const char * CheckNames[] = {
        "ItemVariable",
        "Unmapped1"
    };
    // Amount of offsets added to the CheckOffsets array
    size_t CheckCount = sizeof(CheckOffsets) / sizeof(unsigned long);

    
    
    DMENU_scanner_t * scnInitArr[0x2] = { foundTestMenus, foundTestItems }; // Scanner initialization array (two seems odd, but I may add more arrays depeinding on how I end up using this)
    for (int i = 0; i < maxComponentCount; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            scnInitArr[j][i].CheckCount = CheckCount;     // Amount of checks to run (auto set based on the amount of offsets in the local CheckOffsets array)

            scnInitArr[j][i].CheckOffsets = CheckOffsets; // Offset of the desired member in the parent struct
            scnInitArr[j][i].CheckNames = (const char **) &CheckNames;     // Name of the desired member in the parent struct
            

            // Return buffers for non-null checks
            scnInitArr[j][i].CheckBuffers = (unsigned long *) malloc(CheckCount * sizeof(unsigned long));
            memset((void *) scnInitArr[j][i].CheckBuffers, 0, CheckCount * sizeof(unsigned long));
        }
    }





    //#
    //## Main Scanner Loop
    //#
    while(pItem != null)
    {
        type = (dmenu_vptr_t) read_ulong(game->pid, pItem);

        if ((long) type == -1) // no need to check errno, as it shouldn't be reading -1 either
        {
            pItem = nullptr;
            
            __errg((long) type == -1, _contScannerLoop, "Failed to read item's vptr, or it was actually negative. Skipping item at 0x%llX, and returning from it's containing menu", pItem);
        }


        // Get component vptr at string
        dtype = DMENU_GetTypeAsString(type);
        


        newItem = (char) 1;
        pCurrComponent = nullptr;

        //#
        //## Check current type, saving it if it's not already been found
        //#
        if (encode_sid(dtype) == unknownTypeSID)
        {   
            for (int i = 0; i < unknownTypeCount; i++)
            {
                if (unknownComponents[i].Type == type)
                {
                    newItem = (char) 0x0;
                    goto _nextItem;
                }
            }

            pCurrComponent = &unknownComponents[unknownTypeCount++];
        }
        else {
            for (int i = 0; i < knownTypeCount; i++)
            {
                if (knownComponents[i].Type == type)
                {
                    newItem = (char) 0x0;
                    goto _nextItem;
                }
            }

            pCurrComponent = &knownComponents[knownTypeCount++];
        }


        // Zero current struct
        memset(pCurrComponent, 0, sizeof(DMENU_scanner_t));


        // Save general component info
        pCurrComponent->Type = type;
        strncpy(pCurrComponent->dType, dtype, sizeof(pCurrComponent->dType));
        pCurrComponent->pItem = pItem;




    _nextItem:
        if (nextIsMenu)
        {
            bzero(&menu, sizeof(menu));
            load_menu(pItem, &menu);

            if (newItem)
            {
                read_string(game->pid, menu.pMenuName, pCurrComponent->Name);
                read_string(game->pid, (void *) read_ulong(game->pid, (unsigned long *) &menu.Parent->pMenuName), pCurrComponent->ParentName);
                
                pCurrComponent->ReturnBuffer = 0;
                pCurrComponent->ItemVariable = menu.MenuVariable;
            }
            

            // Read next item pointer
            pItem = menu.FirstItem;
            nextIsMenu = 0;
        }
        else {
            bzero(&item, sizeof(item));
            load_item(pItem, &item);

            if (newItem)
            {
                if (item.pItemText)
                {
                    read_string(game->pid, item.pItemText, pCurrComponent->Name);
                }
                else {
                    strcpy(pCurrComponent->Name, "--------------------------------");
                }
    
                read_string(game->pid, (void *) read_ulong(game->pid, (unsigned long *) &item.Parent->pMenuName), pCurrComponent->ParentName);

                pCurrComponent->ReturnBuffer = item.ReturnBuffer;
                pCurrComponent->ItemVariable = item.ItemVariable;
            }


            unsigned long chk;
            if (foundTestItems[foundTestItemCount].CheckCount > 0)
            {
                for (unsigned i = 0; i < foundTestItems[foundTestItemCount].CheckCount; i++)
                {          
                    chk = *(unsigned long *) (((char *) &item) + foundTestItems[foundTestItemCount].CheckOffsets[i]);
                    
                    if (chk != 0)
                    {
                        // echo("\n# Check value non-null for item at 0x%llX\n# Check Offset: 0x%llX\n# Check Value: 0x%llX\n", pItem, foundTestItems[foundTestItemCount].CheckOffsets[i], chk);
                        foundTestItems[foundTestItemCount].CheckBuffers[i] = chk;

                        foundTestItems[foundTestItemCount].Type = type;
                        strncpy(foundTestItems[foundTestItemCount].dType, dtype, sizeof(foundTestItems[foundTestItemCount].dType));
                        foundTestItems[foundTestItemCount].pItem = pItem;
                        foundTestItems[foundTestItemCount].ReturnBuffer = item.ReturnBuffer;

                        read_string(game->pid, (void *) read_ulong(game->pid, (unsigned long *) &item.Parent->pMenuName), foundTestItems[foundTestItemCount].ParentName);
                        
                        if (item.pItemText)
                        {
                            read_string(game->pid, item.pItemText, foundTestItems[foundTestItemCount].Name);
                        }
                        else {
                            strcpy(foundTestItems[foundTestItemCount].Name, "--------------------------------");
                        }

                        ++foundTestItemCount;
                    }
                }
            }
            


            // Handle ItemSubmenu components
            if (DMENU_GetBaseType(pItem) == stype_itemsubmenu)
            {
                pItem = (dmenu_item_t *) item.ReturnBuffer;

                history[++depth] = (dmenu_menu_t *) item.NextItem;
                
                nextIsMenu = 1;
            }
            else {
                pItem = item.NextItem;
                nextIsMenu = 0;
            }
        }
        

        // Return from submenus
    _contScannerLoop:
        while (pItem == nullptr && depth >= 0)
        {
            pItem = history[depth];
            history[depth--] = nullptr;
        
            nextIsMenu = 0;
        }
    }
    #undef maxComponentCount





    //#
    //## Print scanned contents
    //#
    if (knownTypeCount == 0 && unknownTypeCount == 0 && foundTestMenuCount == 0 && foundTestItemCount == 0)
    {
        echo("\n# Scan finished; nothing found to print, somehow.");
        return;
    }
    echo("\n# Scan finished. Printing scanned contents.", unknownTypeCount);

    
    
    //## Print known types
    if (knownTypeCount > 0)
    {
        echo("Printing %d known types...", knownTypeCount);
        PrintScannerContents(knownComponents, knownTypeCount, parentName);
    }
    //## Print unknown types, if there are any
    if (unknownTypeCount > 0)
    {
        echo("Printing %d unknown types...", unknownTypeCount);
        PrintScannerContents(unknownComponents, unknownTypeCount, parentName);
    }


    //## Print discovered test menus
    if (foundTestMenuCount > (unsigned) 0)
    {
        echo("Printing %d test menus...", foundTestMenuCount);
        PrintScannerContents(foundTestMenus, foundTestMenuCount, parentName);
    }
    //## Print discovered test items
    if (foundTestItemCount > (unsigned) 0)
    {
        echo("Printing %d test items...", foundTestItemCount);
        PrintScannerContents(foundTestItems, foundTestItemCount, parentName);
    }


    echo("Finished printing scanned contents");
}




//! write me
void dump_menu_entries()
{
    dmenu_menu_group_t base;
    load_menu_group(game->Addrs.dev_menu_group, &base);
    
    do_dump_menu_entries(&base);


    if (0)
    {
        load_menu_group(game->Addrs.quick_menu_group, &base);
        do_dump_menu_entries(&base);
    }

    if (0)
    {
        load_menu_group(game->Addrs.favorites_menu_group, &base);
        do_dump_menu_entries(&base);
    }
        
    echo("\nFinished dumping menu group contents\n");
}














//#
//## Region for custom dmenu component creation functions //! write me a new region name
//#


//! write me
static int CreateBaseComponent(dmenu_item_t * item, const char * component_name)
{
    int ret;
    char * str_dest;
    int name_length;
    
    ret = 0;
    // Zero item in tracee's memory
    for (unsigned i = 0; i < sizeof(dmenu_item_t); i++)
    {
        ret |= write_byte(game->pid, (byte *) item + i, (byte) 0);

        __errg(ret != 0, __break, "Failed writing zero byte to struct in tracee's memory. (wrote %d out of %d bytes)", i, sizeof(dmenu_item_t));
    }


__break:
    ret |= write_ulong(game->pid, (unsigned long *) &item->vPtr, (unsigned long) game->Types.DMENU_Types_BaseComponent);
    
    ret |= write_uint(game->pid, &item->OffStateColour, (unsigned) 0xFFC8C8C8); // White
    ret |= write_uint(game->pid, &item->OnStateColour,  (unsigned) 0xFF00FFFF); // Yellow
    
    ret |= write_uint(game->pid, &item->UnknownInt, (unsigned) 0xB0FFB0FF); // Value it always has, still not sure what this does. My original assumption appears to be wrong, though
    
    
    ret |= write_uint(game->pid, (unsigned *) &item->_ItemCount, 1);
    ret |= write_uint(game->pid, (unsigned *) &item->_SelectionIndex, 1);


    if (component_name == NULL)
    {
        component_name = "unnamed";
        echo("Unnamed item created");
    }
    else {
        name_length = strlen(component_name);
        if (name_length < 0x20)
        {
            str_dest = (char *) &item->pItemText;
            ret |= write_string(game->pid, str_dest, component_name);
        }
        else {
            if (name_length < 0xFF)
            {
                name_length = 0xFF;
            }

            str_dest = (char *) proc_malloc(name_length + 1);
            __rerrv(str_dest == NULL, 1, "ERROR: Unable to allocate space for large component name!!!")
            
            write_string(game->pid, str_dest, component_name);
            str_dest[name_length] = 0;
        }

        ret |= write_ulong(game->pid, (unsigned long *) &item->pItemText, (unsigned long) str_dest);

        echo("%s created", component_name);
    }

    // Check for and report the number of failed processes
    __err(ret != 0, nullptr, "%d write processes failed when creating base component \"%s\".", component_name);

    return ret != 0;
}








//! write me
static int CreateNonBaseComponent(dmenu_item_t * pComponent, const char * ItemName, void * ItemVariable, dmenu_vptr_t ItemType, void * ItemEventHandler, void * DefaultEventHandler)
{
    int ret;
    // Create component base
    ret = CreateBaseComponent(pComponent, ItemName);
    __rerrv(ret != 0, 1, "Failed creating base component for %s, aborting.", ItemName)
    echo("Base component created, adding other crap");
    

    // Default to game's basic toggle item event handler if none is specified
    if (ItemEventHandler == nullptr)
    {
        //! TODO: FINISH THIS- this only works for tlou2's latest patch. finish the menu item dumper function we're gonna use to grab these
        ItemEventHandler = (void *) game->EventHandlers.DMENU_Handlers_ToggleItem;
    }
    
    // Item Class/Type (vTable pointer)
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->vPtr, (unsigned long) game->Types.DMENU_Types_ToggleItem);


    // Event Handler type
    ret |= write_uint(game->pid, (unsigned *) &pComponent->EventHandlerTypeID, (unsigned) DMENU_HandlerType_ToggleItem);
    
    // Event Handler
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->EventHandler, (unsigned long) ItemEventHandler);

    // Item Variable
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->ItemVariable, (unsigned long) ItemVariable);

    // Item return buffer
    ret |= write_ulong(game->pid, (unsigned long *) &pComponent->ReturnBuffer, (unsigned long) NULL);
    ret |= write_byte(game->pid, (byte *) &pComponent->ReturnBuffer, read_byte(game->pid, (byte *) &pComponent->ItemVariable));
    
    
    // Check for and report the number of failed processes
    __err(ret != 0, "%d write processes failed when creating %s \"%s\".", DMENU_GetTypeAsString(ItemType), ItemName);

    return ret;
}








//! write me
int CreateToggleItem(dmenu_item_t * pItem, const char * ItemName, void * ItemVariable, void * ItemAlternateEventHandler)
{
    return CreateNonBaseComponent(pItem, ItemName, ItemVariable, game->Types.DMENU_Types_ToggleItem, ItemAlternateEventHandler, game->EventHandlers.DMENU_Handlers_ToggleItem) != 0;
}








//! write me
int CreateItemSubmenu(dmenu_item_t * pItemSubmenu, const char * SubmenuName, dmenu_menu_t * pParentMenu, dmenu_menu_t * pNewSubmenu)
{
    return CreateNonBaseComponent(pItemSubmenu, SubmenuName, (void *) pParentMenu, game->Types.DMENU_Types_ItemSubmenu, pNewSubmenu, NULL) != 0;
}







//! write me
int CreateMenu(dmenu_item_t * pItem, const char * MenuName, dmenu_menu_t * pMenu, void * ItemAlternateEventHandler)
{
    return CreateNonBaseComponent(pItem, MenuName, (void *) pMenu, game->Types.DMENU_Types_Menu, ItemAlternateEventHandler, game->EventHandlers.DMENU_Handlers_ToggleItem) != 0;
}








// Shorthand for allocating the memory for, creating, and appending a new dmenu_item_t to the provided parent menu
// Sets the outvar pItem to the pointer of the new menu entry, unless proc_malloc fails
// returns zero on success, and 1 on failure.
int DMENU_CreateToggleItem(dmenu_menu_t * pParentMenu, dmenu_item_t ** pItem, const char * ItemName, void * ItemVariable, void * CustomEventHandler)
{
    int ret;
    char menu_name[0xFF];

    if (find_item(ItemName, pParentMenu, false) > 0)
    {
        echo("Item \"%s\" already added to specified menu base", ItemName);
        return 0;
    }

    ret = read_string(game->pid, (const char *) &pParentMenu->pMenuName, (char *) menu_name);
    __rerrv(ret == -1, 1, "Error creating and adding new item; read_string returned error when reading name of parent menu")

    echo("Adding %s to %s", ItemName, menu_name);


    // Allocate space for new menu entry
    dmenu_item_t * item;
    item = (dmenu_item_t *) proc_malloc(align(sizeof(dmenu_item_t)));
    __rerrv(!item, 1, "Error allocating space for item");

    *pItem = item;
    CreateToggleItem(item, ItemName, ItemVariable, CustomEventHandler);

    
    echo("Prepending item to parent menu");
    return AppendItemToParent(pParentMenu, item);
} 

















//#
//## Region for dmenu manipulation funcs like appending items or clearing menus //! write me a new region name
//#

//! write me
int AppendItemToParent(dmenu_menu_t * parent_menu, dmenu_item_t * item)
{
    dmenu_item_t
        **next_item_p = &parent_menu->FirstItem,
        *current_item = NULL,
        *prev_item
    ;


    // get the last item in the menu, if it has any contents
    do {
        prev_item = current_item;
        current_item = (dmenu_item_t *) read_ulong(game->pid, (unsigned long *) next_item_p);
        __rerrv((current_item == (dmenu_item_t *) -1 && errno != 0), 1, "ERROR: Failed reading next item when trying to find last item in menu");

        next_item_p = &current_item->NextItem;
        
    } while (current_item != NULL && errno == 0);
    
    if (prev_item == NULL)
    {
        // Default to the menu's first item if it has no entries
        next_item_p = &parent_menu->FirstItem;
    }
    else{
        next_item_p = &prev_item->NextItem;
    }


    
    int ret;
    ret = write_ulong(game->pid, (unsigned long *) next_item_p, (unsigned long) item);
    __rerrv(ret != 0, 1, "ERROR: unable to write item pointer to game memory, aborting append function. ");

    // Assign parent menu and set next item
    write_ulong(game->pid, (unsigned long *) &item->NextItem, 0);
    write_ulong(game->pid, (unsigned long *) &item->Parent, (u_long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint(game->pid, (unsigned int *) &parent_menu->ItemCount, read_uint(game->pid, &parent_menu->ItemCount) + 1);
    write_uint(game->pid, (unsigned int *) &parent_menu->UnknownBool, 1);
    



    char _menu_is_open;
    _menu_is_open = (char) read_byte(game->pid, (byte *) &parent_menu->_UnknownMenuBool);

    if (_menu_is_open)
    {
        echo("updating menu");
        
        void * update_menu;
        update_menu = (void *) read_ulong(game->pid, (unsigned long *) read_ulong(game->pid, (unsigned long *) &item->vPtr) + 0x18);
        if (!update_menu)
        {
            echo("Unable to read item's vPtr!!");
            return 1;
        }


        struct reg args;
        ret = proc_set_args(&args, (uintptr_t) item, 0, 0, 0, 0, 0, 0);
        __rerrv(ret != 0, 1, "ERROR: Unable to set args for call to menu update function.")
        
        
        ret = proc_call(update_menu, nullptr, &args);
        __rerrv(ret != 0, 1, "ERROR: Failed calling menu update function.")
    }

    return 0;
}








//! write me
int PrependItemToParent(dmenu_menu_t * parent_menu, dmenu_item_t * item)
{
    int ret;
    dmenu_item_t * first_item;

    first_item = (dmenu_item_t *) read_ulong(game->pid, (unsigned long *) &parent_menu->FirstItem);
    __rerrv((first_item == (dmenu_item_t *) -1 && errno != 0), 1, "PrependItemToPArent Failed first read", 1)

    // Assign parent menu and set next item
    if (first_item != NULL)
    {
        write_ulong(game->pid, (unsigned long *) &item->NextItem, (unsigned long) first_item);
    }
    write_ulong(game->pid, (unsigned long *) &parent_menu->FirstItem, (unsigned long) item);
    write_ulong(game->pid, (unsigned long *) &item->Parent, (unsigned long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint(game->pid, (unsigned int *) &parent_menu->ItemCount, read_uint(game->pid, &parent_menu->ItemCount) + 1);
    write_uint(game->pid, (unsigned int *) &parent_menu->UnknownBool, 1);
    



    char _menu_is_open;
    _menu_is_open = (char) read_byte(game->pid, (byte *) &parent_menu->_UnknownMenuBool);

    if (_menu_is_open)
    {
        echo("updating menu");
        
        void * update_menu;
        update_menu = (void *) read_ulong(game->pid, (unsigned long *) read_ulong(game->pid, (unsigned long *) &item->vPtr) + 0x18);
        if (!update_menu)
        {
            echo("Unable to read item's vPtr!!");
            return 1;
        }
        

        struct reg args;
        ret = proc_set_args(&args, (uintptr_t) item, 0, 0, 0, 0, 0, 0);
        __rerrv(ret != 0, 1, "ERROR: Unable to set args for call to menu update function.")
        
        
        ret = proc_call(update_menu, nullptr, &args);
        __rerrv(ret != 0, 1, "ERROR: Failed calling menu update function.")
    }

    return 0;
}








//! write me
int InsertItemInParent(dmenu_menu_t * parent_menu, dmenu_item_t * item, const char * target_item_name, char prepend)
{
    return 1;

    dmenu_item_t * target_item;
    dmenu_menu_t * menu_buf;
    dmenu_menu_group_t * base;

    menu_buf = parent_menu;
    do {
        base = (dmenu_menu_group_t *) read_ulong(game->pid, (unsigned long *) &menu_buf->MenuGroup);

        if (base != (dmenu_menu_group_t *) nullptr)
        {
            break;
        }

        menu_buf = (dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &menu_buf->Parent);
        __rerrv(menu_buf == (dmenu_menu_t *) nullptr, 1, "Failed to find menu base for parent menu when inserting item. (all parent menus have null bases, and we've run out of parents!!)");

    } while (1);





    target_item = (dmenu_item_t *) DMENU_FindComponent(target_item_name, parent_menu, stype_item, false);
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
int patch_game()
{
    dmenu_item_t * itemBuf;
    dmenu_menu_t * parentMenu;

    itemBuf = nullptr;
    parentMenu = (dmenu_menu_t *) read_ulong(game->pid, (unsigned long *) &game->Addrs.favorites_menu_group->root_menu);
    DMENU_CreateToggleItem(parentMenu, &itemBuf, "Disable All Visibility", game->Addrs.d_Novis, nullptr);

    return 0;
}

