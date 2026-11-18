#include "inc/include.h"
#include "inc/dmenu.h"
#include "inc/memory.h"
#include "inc/proc.h"

extern NdGame_t * game;




// Read the vPtr at the start of the provided item, and return the simple type of the option to simplify type checks (i.e. checking for stype_item rather than 14 different types)
DMENU_sType_t DMENU_GetBaseType(DMENU_Type_t Type)
{
    if (Type == game->Types.DMENU_Types_MenuGroup)
        return stype_menu_group;
    
    if (Type == game->Types.DMENU_Types_Menu ||
        Type == game->Types.DMENU_Types_DynMenu ||
        Type == game->Types.DMENU_Types_WeaponUpgradesMenu)
        return stype_menu;

    if (Type == game->Types.DMENU_Types_ItemSubmenu ||
        Type == game->Types.DMENU_Types_TaskItemSubmenu ||
        Type == game->Types.DMENU_Types_SubnodeTaskItemSubmenu)
        return stype_itemsubmenu;

    if (Type == game->Types.DMENU_Types_BaseComponent ||
        Type == game->Types.DMENU_Types_SeparatorItem ||
        Type == game->Types.DMENU_Types_ToggleItem ||
        Type == game->Types.DMENU_Types_ExecItem ||
        Type == game->Types.DMENU_Types_AltExecItem ||
        Type == game->Types.DMENU_Types_StringItem ||
        Type == game->Types.DMENU_Types_StringListItem ||
        Type == game->Types.DMENU_Types_DecimalItem ||
        Type == game->Types.DMENU_Types_TaskHistoryItem ||
        Type == game->Types.DMENU_Types_TaskItem ||
        Type == game->Types.DMENU_Types_UserItem ||
        Type == game->Types.DMENU_Types_TimerItem ||
        Type == game->Types.DMENU_Types_CheckpointItem ||
        Type == game->Types.DMENU_Types_FloatItem)
        return stype_item;


    return -1;
}








// [DEBUG-ONLY] Convert the name of the provided vptr to it's string representation, without the prefixed the game id and type name (assuming I haven't removed the latter)
const char * DMENU_GetTypeAsString(DMENU_Type_t type)
{
    static char return_string[0x20];
    memset(return_string, 0, sizeof(return_string));
    strncpy(return_string, "unknown_type", strlen("unknown_type"));
    
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

    const DMENU_Type_t types[] =
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

    return return_string;
}






// Set the current stack allocator
void PushAllocator(int Allocator) {
    // Allocate arg data space in game process
    uintptr_t buf = (uintptr_t) proc_malloc(0x200);
    struct reg args;

    // Write args
    write_uint((unsigned *) buf, Allocator);
    write_string((void *) buf + 0x10, __FILE__);
    write_string((void *) buf + strnlen(__FILE__, 0x200 - 0x40), __FUNCTION__);


    // Push stack allocator
    proc_set_args(&args, buf, buf + 0x10, (uintptr_t) __LINE__, buf + 0x50, 0, 0, 0);
    proc_call((void *) 0x4fa710, nullptr, &args);
}


void PopAllocator() {
    struct reg args;

    // Pop stack allocator
    proc_clear_args(&args);
    proc_call((void *) 0x5eebd0, nullptr, &args);
}








// Recursively check every item and submenu for an item matching the specified dmenu simple type and component name
static void * DMENU_FindComponent(const char * expected_component_name, dmenu_menu_t * start_menu, DMENU_sType_t expected_component_stype, char recurse)
{
    int ret;
    dmenu_menu_t * menu;
    DMENU_MenuGroupIndex_t curr_base_index = 0;
    echo("Beginning search for %s", expected_component_name);


    if (start_menu == nullptr)
    {
        echo("No start menu provided.");
        dmenu_menu_group_t * parent_group;
        
    _recheck:
        parent_group = *(&game->DevMenu + curr_base_index);
        __rerrv(parent_group == null, nullptr, "Null parent base in ndgame struct at game->DevMenu + %d, aborting", curr_base_index);

        echo("Checking menu group at 0x%llX.", parent_group);

        menu = (dmenu_menu_t *) read_ulong((unsigned long *) &parent_group->root_menu);
        __rerrv(menu == (dmenu_menu_t *) -1, nullptr, "error reading root menu from menu group #%d", curr_base_index)
        if (menu == 0)
        {
            echo("ERROR: Menu group doesn't even have a root menu! Aborting.");
            return nullptr;
        }
    }
    else{
        echo("Start menu provided. (0x%llX)", start_menu);
        menu = start_menu;
    }
    

    

    

    void * pItem = (void *) read_ulong((unsigned long *) &menu->FirstItem);
    char parent_name[0xFF];

#ifdef DEBUG
    // Read current menu name before checking for an empty menu
    ret = read_string((const char *) read_ulong((unsigned long *) &menu->pMenuName), parent_name);
    __err(ret == -1, "WARNING: Failed reading name of root menu from menu group \"%s\", it's probably gonna shit the bed after this.", expected_component_name)
#else
    strncpy(parent_name, "Menu", sizeof(parent_name));
#endif

    if (pItem == nullptr)
    {
        echo("%s contains no items, aborting", parent_name);
        return nullptr;
    }

    echo("Searching %s for %s", parent_name, expected_component_name);

    
    int depth = 0;
    void * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size

    char next_is_menu;

    DMENU_sType_t base_type;
    char entry_name[0xFF];
    const char * entry_name_ptr;
    
    next_is_menu = 0;
    while(pItem != nullptr)
    {
        base_type = DMENU_GetBaseType(pItem);
        
        entry_name_ptr = (const char *) read_ulong((unsigned long *) &((dmenu_item_t *) pItem)->pItemText);
        if ((void *) entry_name_ptr == nullptr)
        {
            goto _no_name;
        }
        

        ret = read_string(entry_name_ptr, entry_name);
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
            pItem = (void *) read_ulong((unsigned long *) &((dmenu_menu_t *) pItem)->FirstItem);
            next_is_menu = 0;
        }
        // Item
        else {
            if (base_type == stype_itemsubmenu && recurse > 0)
            {
                // Item Submenu
                pItem = (void *) read_ulong((unsigned long *) &((dmenu_item_t *) pItem)->ReturnBuffer);

                history[++depth] = (void *) read_ulong((unsigned long *) &((dmenu_item_t *) pItem)->NextItem);

                next_is_menu = 1;
            }
            else{
                pItem = (void *) read_ulong((unsigned long *) &((dmenu_item_t *) pItem)->NextItem);
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



    
    if (curr_base_index >= 2)
    {
        #ifdef DEBUG
        echo("Didn't find component %s in Menu #%d; checking next menu", expected_component_name, curr_base_index);
        #endif
        return nullptr;
    }

    curr_base_index++;
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




// Get the root menu of the specified menu group without loading the base in to memory first
dmenu_menu_t * get_root_menu(DMENU_MenuGroupIndex_t menu_group_index)
{
    // Why did I write it this way?!
    return ((dmenu_menu_t *) read_ulong((unsigned long *) &(* (dmenu_menu_group_t **) ((unsigned long) &game->DevMenu + menu_group_index * sizeof(dmenu_menu_group_t *)))->root_menu));
}








// Copy the dmenu_menu_group_t pointed to by the dmenu_menu_group_t * at the provided address in the traced process' memory into the provided menu group struct
int load_menu_group(const dmenu_menu_group_t * address, dmenu_menu_group_t * menu_group)
{
    return read_bytes(sizeof(dmenu_menu_group_t), (byte *) address, (byte *) menu_group) != sizeof(dmenu_menu_group_t);
}




// Copy the dmenu_menu_t at the provided address in the traced process' memory into the provided menu struct
int load_menu(const dmenu_menu_t * address, dmenu_menu_t * menu)
{
    return read_bytes(sizeof(dmenu_menu_t), (byte *) address, (byte *) menu) != sizeof(dmenu_menu_t);
}




// Copy the dmenu_item_t at the provided address in the traced process' memory into the provided item struct
int load_item(const dmenu_item_t * address, dmenu_item_t * item)
{
    return read_bytes(sizeof(dmenu_item_t), (byte *) address, (byte *) item) != sizeof(dmenu_item_t);
}








// Enum for specifying the struct to be used in the dmenu reversing scanner
typedef enum DMENU_ReversingScanner_Type : char {
    stMenu,
    stItem
} DMENU_ReScanner_Type_t;

// DMENU_RunReversingChecks-specific struct
typedef struct DMENU_ReversingScanner_Check {
    DMENU_ReScanner_Type_t Type;
    unsigned long Offset;
    const char * Name;
    unsigned long Buffer;
} DMENU_ReScanner_Check_t;
// do_DMENU_DumpSupportInfo-specific struct
typedef struct DMENU_ReversingScanner {
    char Name[0xFF];       // Component name
    char ParentName[0xFF]; // Component's parent name
    DMENU_Type_t Type;     // Component "type" (class vPtr)
    char dType[0x40];      // Component Type as String 
    dmenu_item_t * pItem;  // Pointer to item itself in the tracee's memory
    unsigned long ReturnBuffer; // Item event handler return value storage //!
    unsigned long ItemVariable; // Item tag
    unsigned long EventHandler; // Event Handler/Callback


    // Detailed-Scanner reserved
    size_t CheckCount;
    DMENU_ReScanner_Check_t * Checks;
    
} DMENU_ReScanner_t;

static void RunScannerCheck(DMENU_ReScanner_Type_t scannerType, DMENU_ReScanner_t Scanner[], unsigned * ScannerCount, void * pComponent, void * pItem, DMENU_Type_t type, const char * dtype)
{
    unsigned long chk;
    if (Scanner[*ScannerCount].CheckCount > 0)
    {
        for (unsigned i = 0; i < Scanner[*ScannerCount].CheckCount; i++)
        {          
            if (Scanner[*ScannerCount].Checks[i].Type != scannerType)
            {
                continue;
            }


            //! TODO: Add support for specific datatype sizes
            chk = *(unsigned long *) (((char *) pComponent) + Scanner[*ScannerCount].Checks[i].Offset);
            
            if (chk != 0)
            {
                // echo("\n# Check value non-null for item at 0x%llX\n# Check Offset: 0x%llX\n# Check Value: 0x%llX\n", pItem, Scanner[*ScannerCount].CheckOffsets[i], chk);
                Scanner[*ScannerCount].Checks[i].Buffer = chk;

                Scanner[*ScannerCount].pItem = pItem;
                Scanner[*ScannerCount].Type = type;
                strncpy(Scanner[*ScannerCount].dType, dtype, sizeof(Scanner[*ScannerCount].dType));

                //! TODO: Replace these dmenu_item_t casts with dmenu_component_t ones
                Scanner[*ScannerCount].ItemVariable = ((dmenu_item_t *) pComponent)->ItemVariable;

                if (scannerType == stMenu)
                {
                    Scanner[*ScannerCount].EventHandler = (unsigned long) ((dmenu_menu_t *) pComponent)->Callback;
                    Scanner[*ScannerCount].ReturnBuffer = 0xDEADBEEF;
                }
                else{
                    Scanner[*ScannerCount].ReturnBuffer = ((dmenu_item_t *) pComponent)->ReturnBuffer;
                    Scanner[*ScannerCount].EventHandler = (unsigned long) ((dmenu_item_t *) pComponent)->EventHandler;
                }

                read_string((void *) read_ulong((unsigned long *) &((dmenu_item_t *) pComponent)->Parent->pMenuName), Scanner[*ScannerCount].ParentName);
                
                if (((dmenu_item_t *) pComponent)->pItemText)
                {
                    read_string(((dmenu_item_t *) pComponent)->pItemText, Scanner[*ScannerCount].Name);
                }
                else {
                    strcpy(Scanner[*ScannerCount].Name, "--------------------------------");
                }

                ++*ScannerCount;
            }
        }
    }
}

// DMENU_RunReversingChecks-specific function
static void PrintReScannerContents(DMENU_ReScanner_t Scanner[], size_t ScannerCount, const char * MenuGroupName, char ShowNullChecks)
{
    if (MenuGroupName == nullptr)
    {
        MenuGroupName = "<unnamed>";
    }

    const char * menuString = "Menu";
    const char * itemString = "Item";
    const char * currentTypeString = nullptr;

    kprintf("\n");
    for (unsigned i = 0; i < ScannerCount; i++)
    {
        kprintf(
            "# /%s/%s/%s\n# {\n#     0x%llX\n#     Type: 0x%llX (%s)\n#     Item Variable: 0x%llX\n#     Return Buffer: 0x%llX\n# }\n",

            MenuGroupName, Scanner[i].ParentName, Scanner[i].Name, (unsigned long long) Scanner[i].pItem, (unsigned long long) Scanner[i].Type, Scanner[i].dType, Scanner[i].ItemVariable, Scanner[i].ReturnBuffer
        );

        if (Scanner[i].CheckCount > 0)
        {
            char scannerOut[0x1FC] = {"# DR Scanner:\n"};

            for (unsigned j = 0; j < Scanner[i].CheckCount; j++)
            {
                if (Scanner[i].Checks[j].Buffer != 0 && !ShowNullChecks)
                {
                    if (Scanner[i].Checks[j].Type == stItem)
                    {
                        currentTypeString = itemString;
                    }
                    else{
                        currentTypeString = menuString;
                    }

                    snprintf(&scannerOut[strnlen(scannerOut, sizeof(scannerOut) - 1)], sizeof(scannerOut), "# DMENU::%s.%s: 0x%llX = 0x%llX\n", currentTypeString, Scanner[i].Checks[j].Name, (unsigned long long) Scanner[i].Checks[j].Offset, (unsigned long long) Scanner[i].Checks[j].Buffer);
                }
            }
            kprintf("%s\n", (char *) scannerOut);
        }
        else{
            kprintf("\n");
        }
    }
}

// Check every dmenu entry in the dev & quick menus for ones with at least one of the specified checks being non-null
//! TODO: Finish refining this. it treats everything as a 64-bit unsigned integer
void DMENU_RunReversingChecks()
{
    dmenu_menu_group_t menuGroup;
    load_menu_group(game->DevMenu, &menuGroup);
    
    //#
    //## Basic null checks
    //#
    #pragma region
    if (menuGroup.root_menu == 0) // Empty menu group
    {
        if (menuGroup.name)
        {
            echo("Provided menu group %s has no menus.", menuGroup.name);
        }
        else {
            echo("Provided menu group (unnamed) has no menus.");
        }
        return;
    }

    char parentName[0x40] = {"Error"};
    __rerr(read_string((const char *) menuGroup.name, parentName) == -1, "Failed to read menu group name before dumping menu entries, aborting.");
    
    

    

    //#
    //## Begin dumper by reading root menu of provided menu group
    //#
    echo("Dumping %s contents.", parentName);
    
    dmenu_menu_t menu;
    read_bytes(sizeof(dmenu_menu_t), (byte *) menuGroup.root_menu, (byte *) &menu);
    
    void * pItem = menu.FirstItem;
    if(pItem == 0)
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
    
    DMENU_Type_t type;
    dmenu_item_t item;
    const char * dtype;




    //#
    //## Component Scanner Variables
    //#
    #define maxComponentCount 0x40

    unsigned foundTestMenuCount = 0;
    DMENU_ReScanner_t foundTestMenus[maxComponentCount] = {0};
    
    unsigned foundTestItemCount = 0;
    DMENU_ReScanner_t foundTestItems[maxComponentCount] = {0};

    


    //#
    //## Setup detailed scanner's current config
    //#
    
    // Array of offsets in the item and/or menu structures
    DMENU_ReScanner_Check_t checks[] =
    {
        { stItem, __offsetof(dmenu_item_t, UnknownInt), "UnknownInt", 0 },   
        { stItem, __offsetof(dmenu_item_t, _SelectionIndex), "_SelectionIndex", 0 },   
        { stItem, __offsetof(dmenu_item_t, _ItemCount), "_ItemCount", 0 }
        // { stMenu, __offsetof(dmenu_menu_t, Unmapped1), "Unmapped1", 0 }
    };
    // Amount of offsets added to the CheckOffsets array
    size_t CheckCount = sizeof(checks) / sizeof(DMENU_ReScanner_Check_t);

    
    DMENU_ReScanner_t * scnInitArr[0x2] = { foundTestMenus, foundTestItems }; // Scanner initialization array (two seems odd, but I may add more arrays depeinding on how I end up using this)
    for (int i = 0; i < maxComponentCount; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            scnInitArr[j][i].CheckCount = CheckCount; // Amount of checks to run (auto set based on the amount of offsets in the local CheckOffsets array)

            scnInitArr[j][i].Checks = checks;
        }
    }





    //#
    //## Main Scanner Loop
    //#
    while(pItem != null)
    {
        //## Read item vPtr/type, skipping it and it's parent menu if it's invalid
        type = (DMENU_Type_t) read_ulong(pItem);
        __errg(type == (void *) -1, _contScannerLoop, "Item @ 0x%llX; Either failed to read item's vptr, or the item was invalid (check errno). Skipping item and it's menu", pItem);
        __errg(type == (void *) 0, _contScannerLoop, "Item @ 0x%llX; Item vPtr/type was null. Skipping item and it's menu", pItem);


        //## Get component vPtr/type at string
        dtype = DMENU_GetTypeAsString(type);

        
        //#
        //## Read the new item/menu
        //#
        if (nextIsMenu)
        {
            bzero(&menu, sizeof(menu));
            load_menu(pItem, &menu);

            RunScannerCheck(stMenu, foundTestMenus, &foundTestMenuCount, &menu, pItem, type, dtype);


            // Read next item pointer
            pItem = menu.FirstItem;
            nextIsMenu = 0;
        }
        else {
            bzero(&item, sizeof(item));
            load_item(pItem, &item);

            RunScannerCheck(stItem, foundTestItems, &foundTestItemCount, &item, pItem, type, dtype);


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
        while (pItem == nullptr && depth >= 0)
        {
        _contScannerLoop:
            pItem = history[depth];
            history[depth--] = nullptr;
        
            nextIsMenu = 0;
        }
    }
    #undef maxComponentCount





    //#
    //## Print scanned contents
    //#

    // Lazy bool to toggle whether we show checks that are all null
    const char showNullChecks = 0;

    //## Make sure there's anything to print in the first place
    if ((foundTestMenuCount | foundTestItemCount) == (unsigned) 0)
    {
        echo("\n# Scan finished; nothing found to print, somehow.");
        return;
    }



    //## Print discovered test menus & items
    if (foundTestMenuCount + foundTestItemCount > (unsigned) 0)
    {
        echo(
            "\n\n"
            "============================================\n"
            "#\n"
            "## Printing %d Scanner Check Values\n"
            "#\n"
            "============================================\n",
            
            foundTestMenuCount + foundTestItemCount
        );

        if (foundTestMenuCount > (unsigned) 0)
        {
            echo("Printing %d test menus...", foundTestMenuCount);
            PrintReScannerContents(foundTestMenus, foundTestMenuCount, parentName, showNullChecks);
        }
        
        if (foundTestItemCount > (unsigned) 0)
        {
            echo("Printing %d test items...", foundTestItemCount);
            PrintReScannerContents(foundTestItems, foundTestItemCount, parentName, showNullChecks);
        }
    }

    echo("Finished printing scanned contents");
}








// do_DMENU_DumpSupportInfo-specific struct
typedef struct DMENU_SupportInfoScanner_s {
    DMENU_Type_t Type;     // Component "type" (class vPtr)
    char dType[0x40];      // Component Type as String 
    void * EventHandler;   // Component event handler (function ran on each cycle)
    
} DMENU_SupportScanner_t;

// DMENU_DumpSupportInfo-specific function
static void PrintSupportScannerContents(DMENU_SupportScanner_t Scanner[], size_t ScannerCount)
{
    //## Types
    kprintf("{\n");       for (unsigned i = 0; i < ScannerCount; i++)
    {
        kprintf("    .DMENU_Types_%s = (DMENU_Type_t) 0x%llX,\n", Scanner[i].dType, Scanner[i].Type);
    }
    
    //## Event Handlers
    kprintf("},\n\n{\n"); for (unsigned i = 0; i < ScannerCount; i++)
    {
        kprintf("    .DMENU_Handlers_%s = (void *) 0x%llX,\n", Scanner[i].dType, Scanner[i].EventHandler);
    }
    kprintf("}\n");
}

//! TODO: UPDATE THIS, YOU FUCKING MORON. Scanner needs filled-in type info to get the names, you overly-forgetful dunce. We're only doing two games here, so just save some option names we know will always be present
// Dump required addresses for dmenu support in the running title
void DMENU_DumpSupportInfo()
{
    dmenu_menu_group_t menuGroup;
    load_menu_group(game->DevMenu, &menuGroup);
    
    
    //#
    //## Basic null checks
    //#
    if (menuGroup.root_menu == 0) // Empty menu group
    {
        if (menuGroup.name)
        {
            echo("Provided menu group %s has no menus.", menuGroup.name);
        }
        else {
            echo("Provided menu group (unnamed) has no menus.");
        }
        return;
    }

    char parentName[0x40] = {"Error"};
    __rerr(read_string((const char *) menuGroup.name, parentName) == -1, "Failed to read menu group name before dumping menu entries, aborting.");
    
    

    

    //#
    //## Begin dumper by reading root menu of current menu group
    //#
    echo("Dumping Support info.", parentName);
    
    dmenu_menu_t menu;
    read_bytes(sizeof(dmenu_menu_t), (byte *) menuGroup.root_menu, (byte *) &menu);
    
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
    
    DMENU_Type_t type;
    dmenu_item_t item;
    const char * dtype;
    
    // Encoded SID of "unknown_type"
    const SID unknownTypeSID = (SID) 0x9E3E4FA85A926262ul;
    




    //#
    //## Component Scanner Variables
    //#
    #define maxComponentCount 0x40
    DMENU_SupportScanner_t * pCurrComponent; // Scanner ref to avoid having a bunch of if/else statements checking for unknown values.

    //## Basic type checks
    unsigned knownTypeCount = 0;
    DMENU_SupportScanner_t knownComponents[maxComponentCount] = {0};
    
    unsigned unknownTypeCount = 0;
    DMENU_SupportScanner_t unknownComponents[maxComponentCount] = {0};





    //#
    //## Main Scanner Loop
    //#
    while(pItem != null)
    {
        //## Read item vPtr/type, skipping it and it's parent menu if it's invalid
        type = (DMENU_Type_t) read_ulong(pItem);
        __errg(type == (void *) -1, _contScannerLoop, "Item @ 0x%llX; Either failed to read item's vptr, or the item was invalid (check errno). Skipping item and it's menu", pItem);
        __errg(type == (void *) 0, _contScannerLoop, "Item @ 0x%llX; Item vPtr/type was null. Skipping item and it's menu", pItem);


        //## Get component vPtr/type at string
        dtype = DMENU_GetTypeAsString(type);

        

        //#
        //## Check current type, saving it if it's not already been found
        //#
        newItem = (char) 1;
        pCurrComponent = nullptr;
        
        if (encode_sid(dtype) == unknownTypeSID)
        {   
            //## Handle UNKNOWN components
            for (int i = 0; i < unknownTypeCount; i++)
            {
                if (unknownComponents[i].Type == type)
                {
                    newItem = (char) 0x0;
                    goto _nextItem;
                }
            }

            if (unknownTypeCount >= maxComponentCount)
            {
                echo("Maximum UNKNOWN component recursion reached!! Skipping item type 0x%llX", type);
                goto _nextItem;
            }


            pCurrComponent = &unknownComponents[unknownTypeCount++];
        }
        else {
            //## Handle KNOWN components
            for (int i = 0; i < knownTypeCount; i++)
            {
                if (knownComponents[i].Type == type)
                {
                    newItem = (char) 0x0;
                    goto _nextItem;
                }
            }

            if (knownTypeCount >= maxComponentCount)
            {
                echo("Maximum KNOWN component recursion reached!! Skipping item type 0x%llX", type);
                goto _nextItem;
            }


            pCurrComponent = &knownComponents[knownTypeCount++];
        }



        //## Set basic scanner struct info
        if (pCurrComponent != nullptr) {
            // Zero current struct
            memset(pCurrComponent, 0, sizeof(DMENU_SupportScanner_t));

            // Save general component info
            pCurrComponent->Type = type;
            strncpy(pCurrComponent->dType, dtype, sizeof(pCurrComponent->dType));
        }





        
        //#
        //## Read the new item/menu
        //#
    _nextItem:
        if (nextIsMenu)
        {
            bzero(&menu, sizeof(menu));
            load_menu(pItem, &menu);

            if (newItem && pCurrComponent != nullptr)
            {
                pCurrComponent->EventHandler = menu.Callback;
            }
            


            // Read next item pointer
            pItem = menu.FirstItem;
            nextIsMenu = 0;
        }
        else {
            bzero(&item, sizeof(item));
            load_item(pItem, &item);

            if (newItem && pCurrComponent != nullptr)
            {
                pCurrComponent->EventHandler = item.EventHandler;
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
        while (pItem == nullptr && depth >= 0)
        {
        _contScannerLoop:
            pItem = history[depth];
            history[depth--] = nullptr;
        
            nextIsMenu = 0;
        }
    }
    #undef maxComponentCount





    //#
    //## Print scanned contents
    //#

    //## Make sure there's anything to print in the first place
    if ((knownTypeCount | unknownTypeCount) == (unsigned) 0)
    {
        echo("\n# Scan finished; nothing found to print, somehow.");
        return;
    }
    echo("\n# Scan finished. Printing scanned contents.");


    //## Print known and/or unknown types
    if (knownTypeCount + unknownTypeCount > (unsigned) 0)
    {
        echo(
            "\n\n"
            "============================================\n"
            "#\n"
            "## Printing %d Known &/or Unknown Types\n"
            "#\n"
            "============================================\n",

            
            knownTypeCount + unknownTypeCount
        );

        if (knownTypeCount > (unsigned) 0)
        {
            echo("Printing %d known types...", knownTypeCount);
            PrintSupportScannerContents(knownComponents, knownTypeCount);
        }
        //## Print unknown types, if there are any
        if (unknownTypeCount > (unsigned) 0)
        {
            echo("Printing %d unknown types...", unknownTypeCount);
            PrintSupportScannerContents(unknownComponents, unknownTypeCount);
        }
    }
    else {

        echo(
            "\n#\n"            
            "## No types to print!!! (How did this happen?)\n"
            "#\n"            
        );
    }


    echo("Finished Support Info");
}








// do_DMENU_DumpSupportInfo-specific struct
typedef struct ffsScanner_s {
    DMENU_Type_t Type;     // Component "type" (class vPtr)
    char dType[0x40];      // Component Type as String 
    void * EventHandler;   // Component event handler (function ran on each cycle)
    char ComponentName[0xFF];
    
} ffsScanner_t;

typedef enum ScannerNameSIDs : unsigned long {
    Display___ = 0x2EC64816C47E0B09,
    Display = 0x1125D159F7E64DF5,
    Draw_World_Axes = 0xB395C8699F19AA20,
    Separator = 0x0,
    FPS_Display_Mode = 0xD3CCD68F47D972D9,
    Msg_Con_Left_Pad = 0xC8B17F89F580EAA6,
    Msg_Con_Scale = 0x8E77FF26CD2B26AC,
    Clear_Msg_Con_Persistent = 0xEAE27E32AEDA0710,
    User = 0xA48F81F001B893D2,
    Set_Allocation_Diff_Detection_Marker = 0xB2588361502D39ED,
    Timer0_HH_MM_SS_UU = 0xEA3F6419BF322512,
    Give_Player_Weapon = 0x911AFF9B4E54453B,
    Player_Weapon_Upgrades = 0x4973E93586AB5D39,
    main_menu___ = 0x70AC0188912E5F36,
    ndi = 0x213CC31925758172,
    // Jackson = 0x117A5A051D638A06,
    // efg_guitar_flashback___ = 0x2E2F38FDA43A1C4E,
    Play_next_task = 0xEF8210605F73C499,
    Re_play_last_selected_task_10 = 0x6EFB7D8560194852
}
 NameSID_t;

static void SnakeTransform(char In[])
{
    size_t len = strlen(In);

    for(int i = 0; i < len; i++)
    {
        switch (In[i])
        {
            //! Lazy
            case ' ': case '-': case ';': case ':': case '.': case '?': case '!': case '(': case ')': case '[': case ']': case '+': case '=': case ',':
                In[i] = '_';
                break;

            default: 
                In[i] = In[i];
                break;
        }
    }
}

// DMENU_RunReversingChecks-specific function
static void ffsPrint(ffsScanner_t Scanner[], size_t ScannerCount)
{
    //## Types
    kprintf("{\n"); for (unsigned i = 0; i < ScannerCount; i++)
    {
        // kprintf("    DMENU_Types_%s = (DMENU_Type_t) 0x%llX,\n", Scanner[i].dType, Scanner[i].Type);
        SnakeTransform(Scanner[i].ComponentName);
        kprintf("%s, %s, &game->Types.DMENU_Types_%s", Scanner[i].ComponentName, Scanner[i].dType, Scanner[i].dType);
    }
    kprintf("}\n");
}

static const char * GuessItemTypeFromCommonName(const char * ItemName)
{
    SID NameSID = encode_sid(ItemName);

    char OutString[0x30] = { "unknown_type" };
    typedef struct {
        SID SID;
        const char * TypeName;
        const DMENU_Type_t Type;

    } lazyName_t;

    lazyName_t NameChecks[] = {};
    // {
    //     { Display___, "MenuGroup", &game->Types.DMENU_Types_MenuGroup },
    //     { Display, "Menu", &game->Types.DMENU_Types_Menu },
    //     { "DynMenu", &game->Types.DMENU_Types_DynMenu },
    //     { "WeaponUpgradesMenu", &game->Types.DMENU_Types_WeaponUpgradesMenu },
    //     { "ItemSubmenu", &game->Types.DMENU_Types_ItemSubmenu },
    //     { "TaskItemSubmenu", &game->Types.DMENU_Types_TaskItemSubmenu },
    //     { "SubnodeTaskItemSubmenu", &game->Types.DMENU_Types_SubnodeTaskItemSubmenu },
    //     { "BaseComponent", &game->Types.DMENU_Types_BaseComponent },
    //     { Separator, "SeparatorItem", &game->Types.DMENU_Types_SeparatorItem },
    //     { Draw_World_Axes, "ToggleItem", &game->Types.DMENU_Types_ToggleItem },
    //     { "ExecItem", &game->Types.DMENU_Types_ExecItem },
    //     { "AltExecItem", &game->Types.DMENU_Types_AltExecItem },
    //     { "StringItem", &game->Types.DMENU_Types_StringItem },
    //     { "StringListItem", &game->Types.DMENU_Types_StringListItem },
    //     { "DecimalItem", &game->Types.DMENU_Types_DecimalItem },
    //     { "TaskHistoryItem", &game->Types.DMENU_Types_TaskHistoryItem },
    //     { "TaskItem", &game->Types.DMENU_Types_TaskItem },
    //     { "UserItem", &game->Types.DMENU_Types_UserItem },
    //     { "TimerItem", &game->Types.DMENU_Types_TimerItem },
    //     { "CheckpointItem", &game->Types.DMENU_Types_CheckpointItem },
    //     { "FloatItem", &game->Types.DMENU_Types_FloatItem }
    // };

    NameSID = encode_sid(ItemName);
    size_t count = sizeof(NameChecks) / 0x10;
    for(int i = 0; i < count; i++)
    {
        if (NameSID == NameChecks[i].SID)
        {
            strncpy(OutString, NameChecks[i].TypeName, sizeof(OutString));
        }
    }

    return "";
} 

// Dump required addresses for dmenu support in the running title
void forFuckSake()
{
#pragma region
    //#
    //## Local Variables
    //#

    //## General Menu-Crawling Variables
    int depth = 0;
    dmenu_menu_t * history[0x20]; // surely there's no place with a submenu depth even approaching 32, so this should be more than sufficient in size
    
    char nextIsMenu = 0;
    char newItem = 0;
    
    DMENU_Type_t type;
    dmenu_item_t item;
    
    //## Component Scanner Variables
    #define maxTypeCount 0x20
    unsigned currTypeCount = 0;

    ffsScanner_t * pCurrComponent; // Scanner ref to avoid having a bunch of if/else statements checking for unknown values.
    ffsScanner_t unknownComponents[maxTypeCount] = {0}; // Basic type checks






    //#
    //## Load the "Dev Menu" menu group, and save the Type
    //#
    
    //## Make sure the menu base pointer pointers have been added
    if (game->DevMenu == 0)
    {
        echo("menu group addresses not yet added, dumbass");
        return;
    }

    dmenu_menu_group_t menuGroup;
    load_menu_group(game->DevMenu, &menuGroup);

    game->Types.DMENU_Types_MenuGroup = menuGroup.vptr;

    if (menuGroup.root_menu == 0)
    {
        echo("Provided menu group (0x%llX) has no menus.", game->DevMenu);
        return;
    }

    char parentName[0x40] = {"Error"};
    __rerr(read_string((const char *) menuGroup.name, parentName) == -1, "Failed to read menu group name before dumping menu entries, aborting.");
    
    

    

    //#
    //## Begin dumper by reading root menu of current menu group
    //#
    
    dmenu_menu_t menu;
    load_menu(menuGroup.root_menu, &menu);

    game->Types.DMENU_Types_Menu = menu.vPtr;
    
    void * pItem = menu.FirstItem;
    __rerr(pItem == nullptr, "Menu %s has no entries. Aborting.", menu.MenuName)
#pragma endregion



    //#
    //## Main Scanner Loop
    //#
    while(pItem != null)
    {
        //## Read item vPtr/type, skipping it and it's parent menu if it's invalid
        type = (DMENU_Type_t) read_ulong(pItem);
        __errg(type == (void *) -1, _contScannerLoop, "Item @ 0x%llX; Either failed to read item's vptr, or the item was invalid (check errno). Skipping item and it's menu", pItem);
        __errg(type == (void *) 0, _contScannerLoop, "Item @ 0x%llX; Item vPtr/type was null. Skipping item and it's menu", pItem);



        //#
        //## Check current type, saving it if it's not already been found
        //#
        newItem = (char) 1;
        pCurrComponent = nullptr;

        for (int i = 0; i < currTypeCount; i++)
        {
            if (unknownComponents[i].Type == type)
            {
                newItem = (char) 0x0;
                // echo("Known item 0x%llX", type);
                goto _nextItem;
            }
        }

        if (currTypeCount >= maxTypeCount)
        {
            echo("Maximum UNKNOWN component recursion reached!! Skipping item type 0x%llX", type);
            newItem = (char) 0x0;
            goto _nextItem;
        }


        // I'm Lazily leaving this pointer usage intact so the code's prettier
        pCurrComponent = &unknownComponents[currTypeCount++];


        // Zero current struct
        memset(pCurrComponent, 0, sizeof(ffsScanner_t));

        // Save general component info
        pCurrComponent->Type = type;
        strncpy(pCurrComponent->dType, DMENU_GetTypeAsString(type), sizeof(pCurrComponent->dType));
        echo("New type (0x%llX/%s)", type, pCurrComponent->dType);



        
        //#
        //## Read the new item/menu
        //#
    _nextItem:
        if (nextIsMenu)
        {
            bzero(&menu, sizeof(menu));
            load_menu(pItem, &menu);

            if (newItem && pCurrComponent != nullptr)
            {
                pCurrComponent->EventHandler = menu.Callback;
                read_string((const char *) menu.pMenuName, pCurrComponent->ComponentName);
            }
            


            // Read next item pointer
            pItem = menu.FirstItem;
            nextIsMenu = 0;
        }
        else {
            bzero(&item, sizeof(item));
            load_item(pItem, &item);

            if (newItem && pCurrComponent != nullptr)
            {
                pCurrComponent->EventHandler = item.EventHandler;

                if (item.pItemText == 0) {
                    pCurrComponent->ComponentName[0] = '\0';
                }
                else {
                    read_string((const char *) item.pItemText, pCurrComponent->ComponentName);
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
        while (pItem == nullptr && depth >= 0)
        {
        _contScannerLoop:
            pItem = history[depth];
            history[depth--] = nullptr;
        
            nextIsMenu = 0;
        }
    }
    #undef maxComponentCount





    //#
    //## Print scanned contents
    //#

    //## Make sure there's anything to print in the first place
    if (currTypeCount == (unsigned) 0)
    {
        echo("\n# Scan finished; nothing found to print, somehow.");
        return;
    }
    echo("\n# Scan finished. Printing scanned contents.");


    //## Print known and/or unknown types
    if (currTypeCount > (unsigned) 0)
    {
        echo(
            "\n\n"
            "============================================\n"
            "#\n"
            "## Printing %d Known &/or Unknown Types\n"
            "#\n"
            "============================================\n",

            
            currTypeCount
        );

        //## Print unknown types, if there are any
        if (currTypeCount > (unsigned) 0)
        {
            echo("Printing %d unknown types...", currTypeCount);
            ffsPrint(unknownComponents, currTypeCount);
        }
    }
    else {

        echo(
            "\n#\n"            
            "## No types to print!!! (How did this happen?)\n"
            "#\n"            
        );
    }


    echo("Finished Support Info");
}




















//#
//## DMENU Component Creation Functions
//#
#pragma region

// Initialize the provided dmenu_component_t (pComponent), setting basic dmenu_component_t structure properties
static int CreateBaseComponent(dmenu_item_t * pComponent, const char * component_name)
{
    int ret = 0;
    char * str_dest;
    int name_length;
    
    // Zero item in tracee's memory
    for (unsigned i = 0; i < sizeof(dmenu_item_t); i++)
    {
        ret |= write_byte((byte *) pComponent + i, (byte) 0);

        __errg(ret != 0, __break, "Failed writing zero byte to struct in tracee's memory. (wrote %d out of %d bytes)", i, sizeof(dmenu_item_t));
    }

__break:
    ret |= write_ulong((unsigned long *) &pComponent->vPtr, (unsigned long) game->Types.DMENU_Types_BaseComponent);
    
    ret |= write_uint(&pComponent->OffStateColour, (unsigned) 0xFFC8C8C8); // White
    ret |= write_uint(&pComponent->OnStateColour,  (unsigned) 0xFF00FFFF); // Yellow
    
    ret |= write_uint(&pComponent->UnknownInt, (unsigned) 0xFFB0FFB0); // Value it always has, still not sure what this does. My original assumption appears to be wrong, though
    
    
    ret |= write_uint((unsigned *) &pComponent->_ItemCount, 1);
    ret |= write_uint((unsigned *) &pComponent->_SelectionIndex, 1);



    // Write component name
    if (component_name == NULL)
    {
        component_name = "unnamed";
        echo("Unnamed item created");
    }
    else {
        name_length = strlen(component_name);
        if (name_length < 0x20)
        {
            str_dest = (char *) &pComponent->ItemText;
            write_string(str_dest, component_name);
        }
        else {
            echo("Item name \"%s\" does not fit the default component name/text buffer, allocating extended buffer", component_name);
            if (name_length < 0xFF)
            {
                name_length = 0xFF;
            }

            str_dest = (char *) proc_malloc(name_length + 1);
            __rerrv(str_dest == NULL, 1, "Unable to allocate space for large component name!!!")
            
            write_string(str_dest, component_name);
            str_dest[name_length] = 0;
        }

        ret |= write_ulong((unsigned long *) &pComponent->pItemText, (unsigned long) str_dest);

        echo("%s created", component_name);
    }



    // Check for and report the number of failed processes
    __err(ret != 0, "%d write processes failed when creating base component \"%s\" (0x%llX).", ret, component_name, pComponent);

    return ret != 0;
}





// Initialize the provided dmenu_menu_t (pMenu), setting basic dmenu_component_t structure properties and additional menu-related ones
static int CreateMenuComponent(dmenu_menu_t * pMenu, const char * MenuName, dmenu_menu_t * pParent, void * Callback, void * MenuVariable)
{
    int ret = 0;
    // Create component base
    ret = CreateBaseComponent((dmenu_item_t *) pMenu, MenuName);
    __rerrv(ret != 0, 1, "Failed creating base component for menu %s, aborting.", MenuName)
    
    
    echo("Base component created, finishing menu creation");
    
    // Item Class/Type (vTable pointer)
    ret += write_ulong((unsigned long *) &pMenu->vPtr, (unsigned long) game->Types.DMENU_Types_Menu);

    // Callback
    ret += write_ulong((unsigned long *) &pMenu->Callback, (unsigned long) Callback);

    // Menu Variable
    ret += write_ulong((unsigned long *) &pMenu->MenuVariable, (unsigned long) MenuVariable);

    // Menu Variable
    ret += write_ulong((unsigned long *) &pMenu->Parent, (unsigned long) pParent);

    
    // Check for and report the number of failed processes
    __err(ret != 0, "%d write processes failed when creating %s \"%s\".", ret, DMENU_GetTypeAsString(game->Types.DMENU_Types_Menu), MenuName);

    return ret;
}





// Initialize the provided dmenu_item_t (pComponent), setting basic dmenu_component_t structure properties and additional itemSubmenu-related ones
static int CreateItemSubmenuComponent(dmenu_item_t * pComponent, const char * ItemSubmenu, dmenu_menu_t * pChild, void * ItemEventHandler, void * ItemVariable)
{
    int ret;
    // Create component base
    ret = CreateBaseComponent(pComponent, ItemSubmenu);
    __rerrv(ret != 0, 1, "Failed creating base component for submenu %s, aborting.", ItemSubmenu)
    
    echo("Base component created, finishing item submenu creation");


    // Item Class/Type (vTable pointer)
    ret |= write_ulong((unsigned long *) &pComponent->vPtr, (unsigned long) game->Types.DMENU_Types_ItemSubmenu);


    // Event Handler type
    ret |= write_uint((unsigned *) &pComponent->EventHandlerTypeID, (unsigned) DMENU_HandlerType_ItemSubmenu);
    
    // Event Handler
    ret |= write_ulong((unsigned long *) &pComponent->EventHandler, (unsigned long) ItemEventHandler);


    // Item Variable
    ret |= write_ulong((unsigned long *) &pComponent->ItemVariable, (unsigned long) ItemVariable);

    // Item return buffer
    ret |= write_ulong((unsigned long *) &pComponent->ReturnBuffer, (unsigned long) pChild);
    
    
    // Check for and report the number of failed processes
    __err(ret != 0, "%d write processes failed when creating %s \"%s\".", DMENU_GetTypeAsString(game->Types.DMENU_Types_ItemSubmenu), ItemSubmenu);

    return ret;
}





// Initialize the provided dmenu_item_t (pComponent), setting basic dmenu_component_t structure properties and important item info
// (Type, event handler)
static int CreateGeneralComponent(dmenu_item_t * pComponent, const char * ItemName, void * ItemVariable, DMENU_Type_t ItemType, DMENU_item_eh_type_t EventHandlerType, void * EventHandler)
{
    int ret;
    // Create component base
    ret = CreateBaseComponent(pComponent, ItemName);
    __rerrv(ret != 0, 1, "Failed creating base component for %s, aborting.", ItemName)
    echo("Base component created, finishing general component");
    

    // Default to game's basic toggle item event handler if none is specified
    __rerrv(EventHandler == nullptr && EventHandlerType != DMENU_HandlerType_ToggleItem, 1, "Item event handler still null, aborting.");

    // Item Class/Type (vTable pointer)
    ret |= write_ulong((unsigned long *) &pComponent->vPtr, (unsigned long) ItemType);


    // Event Handler type
    ret |= write_uint((unsigned *) &pComponent->EventHandlerTypeID, (unsigned) EventHandlerType);
    
    // Event Handler
    ret |= write_ulong((unsigned long *) &pComponent->EventHandler, (unsigned long) EventHandler);

    // Item Variable
    ret |= write_ulong((unsigned long *) &pComponent->ItemVariable, (unsigned long) ItemVariable);

    // Item return buffer
    ret |= write_ulong((unsigned long *) &pComponent->ReturnBuffer, (unsigned long) ItemVariable);
    
    
    // Check for and report the number of failed processes
    __err(ret != 0, "%d write processes failed when creating %s \"%s\".", DMENU_GetTypeAsString(ItemType), ItemName);

    return ret;
}








// Initialize a provided DMENU::Menu (pMenu) with the specified name, parent, plus an optional callback and/or menu variable
int DMENU_CreateMenu(dmenu_menu_t * pMenu, const char * MenuName, dmenu_menu_t * pParent, void * Callback, void * MenuVariable)
{
    return CreateMenuComponent(pMenu, MenuName, pParent, Callback, MenuVariable);
}



// Initialize a provided DMENU::ItemSubmenu (pItemSubmenu) with the specified name, and an optional event handler and/or submenu variable
int DMENU_CreateItemSubmenu(dmenu_item_t * pItemSubmenu, const char * SubmenuName, dmenu_menu_t * pNewSubmenu, void * CustomEventHandler, void * SubmenuVariable)
{
    return CreateItemSubmenuComponent(pItemSubmenu, SubmenuName, (void *) pNewSubmenu, CustomEventHandler, SubmenuVariable) != 0;
}



// Initialize a provided DMENU::Item (pItem) of type ToggleItem with the specified name, variable, and an optional alternate event handler
int DMENU_CreateToggleItem(dmenu_item_t * pItem, const char * ItemName, void * ItemVariable, void * EventHandler)
{
    if (EventHandler == nullptr)
    {
        EventHandler = game->EventHandlers.DMENU_Handlers_ToggleItem;
    }

    return CreateGeneralComponent(pItem, ItemName, ItemVariable, game->Types.DMENU_Types_ToggleItem, DMENU_HandlerType_ToggleItem, EventHandler) != 0;
}



// Initialize a provided DMENU::Item (pItem) of type ExecItem with the specified name, variable, and an optional event handler
int DMENU_CreateExecItem(dmenu_item_t * pItem, const char * ItemName, void * ItemFunction)
{
    int ret = CreateGeneralComponent(pItem, ItemName, nullptr, game->Types.DMENU_Types_ExecItem, DMENU_HandlerType_ExecItem, ItemFunction) != 0;
    
    if (ret == 0)
    {
        ret = write_ulong((unsigned long *) &pItem->_SelectionIndex, (unsigned long) 0);
        ret = write_ulong((unsigned long *) &pItem->_ItemCount, (unsigned long) 0);
    }
    return ret;
}








// Shorthand for allocating the memory for, creating a new dmenu_menu_t
// If the menu has not yet been created, sets *ppMenu to the newly-allocated pMenu. If the menu already exists as a submenu for pParent, sets *ppMenu to the existing menu
//
// Returns:
// If a new menu is created, returns 1
// If the menu is found to already exist in pParent's children, 2 is returned
// On failure, returns -1
int DMENU_FullCreateMenu(dmenu_menu_t * pParent, dmenu_menu_t ** ppMenu, const char * MenuName, void * Callback, void * MenuVariable)
{
    dmenu_menu_t * menuChk = find_menu(MenuName, pParent, false);
    __rerrv(menuChk == (dmenu_menu_t *) -1, -1, "An error occured when searching the child menus of the provided menu at 0x%llX's for %s", pParent, MenuName);

    if (menuChk > (long) 0)
    {
        *ppMenu = menuChk;
        echo("Menu \"%s\" already added to specified menu group", MenuName);
        return 2;
    }



    // Allocate space for new menu
    dmenu_menu_t * pMenu = (dmenu_menu_t *) proc_malloc(align(sizeof(dmenu_menu_t)));
    __rerrv(!pMenu, -1, "Error allocating space for item");


    // Initialize the DMENU::Menu
    int ret = DMENU_CreateMenu(pMenu, MenuName, pParent, Callback, MenuVariable);
    __rerrv(ret != 0, -1, "Failed to create item. Aborting to avoid breaking pParent")
    
    *ppMenu = pMenu;
    return 1;
}




// Shorthand for allocating the memory for, creating a new dmenu_item_t ItemSubmenu
// If the submenu has not yet been created, sets *ppItem to the newly-allocated pItem. If the submenu already exists in pParent, sets *ppItem to the existing submenu
//
// Returns:
// If a new submenu is created, returns 1
// If the submenu is found to already exist in pParent, 2 is returned
// On failure, returns -1
int DMENU_FullCreateItemSubmenu(dmenu_menu_t * pParent, dmenu_item_t ** ppItem, const char * ItemSubmenuName, dmenu_menu_t * pNewSubmenu, void * CustomEventHandler, void * SubmenuVariable)
{
    dmenu_item_t * itemChk;
    if (ItemSubmenuName == nullptr)
    {
        ItemSubmenuName = "<unnamed>";
    }
    else {
        itemChk = find_item(ItemSubmenuName, pParent, false);
        __rerrv(itemChk == (dmenu_item_t *) -1, -1, "An error occured when searching the provided item in the menu at 0x%llX for %s", pParent, ItemSubmenuName)
    
        if (itemChk > (long) 0) {
            echo("Item Submenu \"%s\" already added to specified menu group", ItemSubmenuName);
            *ppItem = itemChk;
            return 2;
        }
    }



    // Allocate space for new menu entry
    dmenu_item_t * pItem = (dmenu_item_t *) proc_malloc(align(sizeof(dmenu_item_t)));
    __rerrv(!pItem, -1, "Error allocating space for %s", ItemSubmenuName);


    // Initialize the DMENU::ItemSubmenu
    int ret = DMENU_CreateItemSubmenu(pItem, ItemSubmenuName, pNewSubmenu, CustomEventHandler, SubmenuVariable);
    __rerrv(ret != 0, -1, "Failed to create item submenu. Aborting to avoid breaking pParent")
    *ppItem = pItem;

    
    // Append the newly-created DMENU::ItemSubmenu to the provided DMENU::Menu (pParent)
    ret = AppendItemToParent(pParent, pItem);
    __rerrv(ret != 0, -1, "Failed to append the created toggle item at 0x%llX to the parent menu at 0x%llX", pItem, pParent);
    
    return 1;
}




// Shorthand for allocating the memory for, creating a new dmenu_item_t ToggleItem
// If the item has not yet been created, sets *ppItem to the newly-allocated pItem. If the item already exists in pParent, sets *ppItem to the existing item
//
// Returns:
// If a new item is created, returns 1
// If the item is found to already exist in pParent, 2 is returned
// On failure, returns -1
int DMENU_FullCreateToggleItem(dmenu_menu_t * pParent, dmenu_item_t ** ppItem, const char * ItemName, void * ItemVariable, void * CustomEventHandler)
{
    dmenu_item_t * itemChk;
    if (ItemName == nullptr)
    {
        ItemName = "<unnamed>";
    }
    else {
        itemChk = find_item(ItemName, pParent, false);
        __rerrv(itemChk == (dmenu_item_t *) -1, -1, "An error occured when searching the provided item in the menu at 0x%llX for %s", pParent, ItemName)

        if (itemChk > (long) 0) {
            echo("Item \"%s\" already added to specified menu group", ItemName);
            *ppItem = itemChk;
            return 2;
        }
    }



    // Allocate space for new menu entry
    dmenu_item_t * pItem = (dmenu_item_t *) proc_malloc(align(sizeof(dmenu_item_t)));
    __rerrv(!pItem, -1, "Error allocating space for %s", ItemName)
    
    
    // Initialize the DMENU::Item
    int ret = DMENU_CreateToggleItem(pItem, ItemName, ItemVariable, CustomEventHandler);
    __rerrv(ret != 0, -1, "Failed to create item. Aborting to avoid breaking pParent")
    
    
    *ppItem = pItem;
    // Append the newly-created DMENU::ItemSubmenu to the provided DMENU::Menu (pParent)
    ret = AppendItemToParent(pParent, pItem);
    __rerrv(ret != 0, -1, "Failed to append the created toggle item at 0x%llX to the parent menu at 0x%llX", pItem, pParent);

    return 1;
}




// Shorthand for allocating the memory for, creating a new dmenu_item_t ToggleItem
// If the item has not yet been created, sets *ppItem to the newly-allocated pItem. If the item already exists in pParent, sets *ppItem to the existing item
//
// Returns:
// If a new item is created, returns 1
// If the item is found to already exist in pParent, 2 is returned
// On failure, returns -1
int DMENU_FullCreateExecItem(dmenu_menu_t * pParent, dmenu_item_t ** ppItem, const char * ItemName, void * ItemFunction)
{
    dmenu_item_t * itemChk;
    if (ItemName == nullptr)
    {
        ItemName = "<unnamed>";
    }
    else {
        itemChk = find_item(ItemName, pParent, false);
        __rerrv(itemChk == (dmenu_item_t *) -1, -1, "An error occured when searching the provided item in the menu at 0x%llX for %s", pParent, ItemName)

        if (itemChk > (long) 0) {
            echo("Item \"%s\" already added to specified menu group", ItemName);
            *ppItem = itemChk;
            return 2;
        }
    }



    // Allocate space for new menu entry
    dmenu_item_t * pItem = (dmenu_item_t *) proc_malloc(align(sizeof(dmenu_item_t)));
    __rerrv(!pItem, -1, "Error allocating space for %s", ItemName)
    
    
    // Initialize the DMENU::ItemSubmenu
    int ret = DMENU_CreateExecItem(pItem, ItemName, ItemFunction);
    __rerrv(ret != 0, -1, "Failed to create item. Aborting to avoid breaking pParent")
    *ppItem = pItem;
    
    
    // Append the newly-created DMENU::ItemSubmenu to the provided DMENU::Menu (pParent)
    ret = AppendItemToParent(pParent, pItem);
    __rerrv(ret != 0, -1, "Failed to append the created toggle item at 0x%llX to the parent menu at 0x%llX", pItem, pParent);

    return 1;
}
#pragma endregion
















//#
//## Region for dmenu manipulation funcs like appending items or clearing menus //! write me a new region name
//#
#pragma region

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
        current_item = (dmenu_item_t *) read_ulong((unsigned long *) next_item_p);
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
    ret = write_ulong((unsigned long *) next_item_p, (unsigned long) item);
    __rerrv(ret != 0, 1, "ERROR: unable to write item pointer to game memory, aborting append function. ");

    // Assign parent menu and set next item
    write_ulong((unsigned long *) &item->NextItem, 0);
    write_ulong((unsigned long *) &item->Parent, (u_long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint((unsigned int *) &parent_menu->ItemCount, read_uint(&parent_menu->ItemCount) + 1);
    write_uint((unsigned int *) &parent_menu->UnknownBool, 1);
    



    if (0)
    // if (read_byte((byte *) &parent_menu->_UnknownMenuBool))
    {
        echo("updating menu");
        
        void * update_menu;
        update_menu = (void *) read_ulong((unsigned long *) read_ulong((unsigned long *) &item->vPtr) + 0x18);
        if (!update_menu)
        {
            echo("Unable to read item's vPtr!!");
            return 1;
        }


        struct reg args;
        // ret = proc_set_args(&args, (uintptr_t) item, 0, 0, 0, 0, 0, 0);
        ret = proc_set_args(&args, (uintptr_t) parent_menu, 0, 0, 0, 0, 0, 0);
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

    first_item = (dmenu_item_t *) read_ulong((unsigned long *) &parent_menu->FirstItem);
    __rerrv((first_item == (dmenu_item_t *) -1 && errno != 0), 1, "PrependItemToPArent Failed first read", 1)

    // Assign parent menu and set next item
    if (first_item != NULL)
    {
        write_ulong((unsigned long *) &item->NextItem, (unsigned long) first_item);
    }
    write_ulong((unsigned long *) &parent_menu->FirstItem, (unsigned long) item);
    write_ulong((unsigned long *) &item->Parent, (unsigned long) parent_menu);

    // Increment parent menu's item count, and set that unknown bool to 1 like the game does (menu updated, maybe? still need to figure out what this boolean is)
    write_uint((unsigned int *) &parent_menu->ItemCount, read_uint(&parent_menu->ItemCount) + 1);
    write_uint((unsigned int *) &parent_menu->UnknownBool, 1);
    



    char _menu_is_open;
    _menu_is_open = (char) read_byte((byte *) &parent_menu->_UnknownMenuBool);

    if (_menu_is_open)
    {
        echo("updating menu");
        
        void * update_menu;
        update_menu = (void *) read_ulong((unsigned long *) read_ulong((unsigned long *) &item->vPtr) + 0x18);
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
        base = (dmenu_menu_group_t *) read_ulong((unsigned long *) &menu_buf->MenuGroup);

        if (base != (dmenu_menu_group_t *) nullptr)
        {
            break;
        }

        menu_buf = (dmenu_menu_t *) read_ulong((unsigned long *) &menu_buf->Parent);
        __rerrv(menu_buf == (dmenu_menu_t *) nullptr, 1, "Failed to find menu group for parent menu when inserting item. (all parent menus have null bases, and we've run out of parents!!)");

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
#pragma endregion
















//#
//## Region for custom menu patch functions //! write me a new region name
//#
#pragma region

//! TODO: Write the base menu item creation function and move the optimization to that. I don't want bzero being optimized out.
int patch_game()
{
    dmenu_menu_t * pFavoritesMenu = (dmenu_menu_t *) read_ulong((unsigned long *) &game->FavoritesMenu->root_menu);
    dmenu_item_t * pItem = nullptr;

    if (game->Addrs.Novis != nullptr)
    {
        DMENU_FullCreateToggleItem(pFavoritesMenu, &pItem, "Disable All Visibility", game->Addrs.Novis, nullptr);
    }
    

    if (game->game_id == T2R && encode_sid(game->app_ver) == V02001010)
    {
        void * DetonateAllEnemiesEventHandler = (void *) 0x16cc660;
        
        echo("========================================");
        if (read_byte(DetonateAllEnemiesEventHandler) == 0x83) // Check for patched-in temp function
        {
            DMENU_FullCreateExecItem(pFavoritesMenu, &pItem, "Detonate All Enemies", DetonateAllEnemiesEventHandler); //! Calls patched-in function
        }
        else{
            echo("WARNING: Event handler function at 0x%llX is missing, item will not be created", DetonateAllEnemiesEventHandler);
        }

        #ifdef DEBUG
        dmenu_menu_t * pTestMenu;
        DMENU_FullCreateMenu(pFavoritesMenu, &pTestMenu, "Test Menu", nullptr, nullptr);
        
        dmenu_item_t * pTestItemSubmenu;
        DMENU_FullCreateItemSubmenu(pFavoritesMenu, &pTestItemSubmenu, "Test Menu...", pTestMenu, nullptr, nullptr);
        
        DMENU_FullCreateToggleItem(pTestMenu, &pItem, "Test Toggle Item", game->Addrs.Novis, nullptr);

        // Push stack allocator for item creation
        PushAllocator(0x5D);

        dmenu_menu_t * pAvMenu;
        DMENU_FullCreateMenu(pFavoritesMenu, &pAvMenu, "Broken Actor Viewer Menu", nullptr, nullptr);
        
        dmenu_item_t * pAvSubmenu;
        DMENU_FullCreateItemSubmenu(pFavoritesMenu, &pAvSubmenu, "Broken Av Menu...", pAvMenu, nullptr, nullptr);
        
        
        // Create Small SpawnAvActor menu
        struct reg args;
        void * UnknownSpawnAvObjectMenuSmall = (void *) 0x27ad490;
        proc_set_args(&args, 0, (uintptr_t) pAvMenu, 0, 0, 0, 0, 0);
        proc_call(UnknownSpawnAvObjectMenuSmall, nullptr, &args);
        
        PopAllocator();
        #endif

        return 0;
    }
        
    return 0;
}
#pragma endregion

