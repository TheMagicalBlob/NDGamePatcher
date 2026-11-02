//#
//## DMenu-related .elf Addresses, as well as some miscellaneous functions
//# (assumes a base address of 0x400000)

// typedef struct ndgame NdGame_t;

extern NdGame_t * game;

struct DMENU_types_t;
struct dmenu_menu_group_t;
typedef void * dmenu_vptr_t;




typedef struct address_table_s
{
    game_id_t game_id;              // Id of the loaded game (T2R, T1X, UC4, etc.)
    app_version_sid_t app_ver_sid;  // The games app_ver encoded as an SID
    
    DMENU_Addresses_t Addrs;        // Table of function/variable addresses in the game process
    DMENU_Types_t Types;            // Table of dmenu vPtr's applicable to the current game's dmenu
    DMENU_EventHandlers_t Handlers; // Table of event handler function pointers

} address_table_t;




// Where support for individual titles and patches for said titles is actually added
// Array containing the structs of various important addresses to apply to the global NdGame_t struct when a supported game is detected
static const address_table_t TitleSupportTable[] =
{
    //#
    //## T2R 02.001.010 BEGIN
    //#
    {
        T2R,
        V02001010,

        {
            .dev_menu_group       = (dmenu_menu_group_t *) 0x6b63998, // (is actually a dmenu_menu_group_t **, but is read and replaced on ndgame struct init)
            .quick_menu_group     = (dmenu_menu_group_t *) 0x6b64670,
            .favorites_menu_group = (dmenu_menu_group_t *) 0x6b639a0,

            .f_Free          = (void *) 0x02a8f680,
            .f_Malloc        = (void *) 0x02a8f690,
            .f_LookupSymbol = (void *) 0x051c0c0,

            .d_Novis = (byte *) 0x70cc124
        },

        {
            .DMENU_Types_MenuGroup =           (dmenu_vptr_t) 0x322E688,
            .DMENU_Types_Menu =               (dmenu_vptr_t) 0x322E5B0,
            .DMENU_Types_DynMenu =            (dmenu_vptr_t) 0x31B5248,
            .DMENU_Types_WeaponUpgradesMenu = (dmenu_vptr_t) 0x32280B0,
            .DMENU_Types_ItemSubmenu =        (dmenu_vptr_t) 0x322E488,
            .DMENU_Types_TaskItemSubmenu =    (dmenu_vptr_t) 0x3252D58,
            .DMENU_Types_SubnodeTaskItemSubmenu = (dmenu_vptr_t) 0x3252C30,
            .DMENU_Types_BaseComponent =      (dmenu_vptr_t) 0x322d5c8,
            .DMENU_Types_SeparatorItem =      (dmenu_vptr_t) 0x322D6A0,
            .DMENU_Types_ToggleItem =         (dmenu_vptr_t) 0x322DA18,
            .DMENU_Types_ExecItem =           (dmenu_vptr_t) 0x322E238,
            .DMENU_Types_AltExecItem =        (dmenu_vptr_t) 0x322E360,
            .DMENU_Types_StringItem =         (dmenu_vptr_t) 0x322DFE8,
            .DMENU_Types_StringListItem =     (dmenu_vptr_t) 0x322DC68,
            .DMENU_Types_DecimalItem =        (dmenu_vptr_t) 0x322DEB8,
            .DMENU_Types_TaskHistoryItem =    (dmenu_vptr_t) 0x32529E0,
            .DMENU_Types_TaskItem =           (dmenu_vptr_t) 0x3252B08,
            .DMENU_Types_UserItem =           (dmenu_vptr_t) 0x3243BF0,
            .DMENU_Types_TimerItem =          (dmenu_vptr_t) 0x3117960,
            .DMENU_Types_CheckpointItem =     (dmenu_vptr_t) 0x322DB40,
            .DMENU_Types_FloatItem =          (dmenu_vptr_t) 0x322E110,
        },

        {
            .DMENU_Handlers_MenuGroup =           (void *) nullptr,

            .DMENU_Handlers_Menu =               (void *) nullptr,
            .DMENU_Handlers_DynMenu =            (void *) nullptr,
            .DMENU_Handlers_WeaponUpgradesMenu = (void *) nullptr,
            
            .DMENU_Handlers_ItemSubmenu =        (void *) nullptr,
            .DMENU_Handlers_TaskItemSubmenu =    (void *) nullptr,
            .DMENU_Handlers_SubnodeTaskItemSubmenu = (void *) nullptr,
            
            .DMENU_Handlers_BaseComponent =      (void *) nullptr,
            .DMENU_Handlers_SeparatorItem =      (void *) nullptr,
            .DMENU_Handlers_ToggleItem =         (void *) nullptr,
            .DMENU_Handlers_ExecItem =           (void *) nullptr,
            .DMENU_Handlers_AltExecItem =        (void *) nullptr,
            .DMENU_Handlers_StringItem =         (void *) nullptr,
            .DMENU_Handlers_StringListItem =     (void *) nullptr,
            .DMENU_Handlers_DecimalItem =        (void *) nullptr,
            .DMENU_Handlers_TaskHistoryItem =    (void *) nullptr,
            .DMENU_Handlers_TaskItem =           (void *) nullptr,
            .DMENU_Handlers_UserItem =           (void *) nullptr,
            .DMENU_Handlers_TimerItem =          (void *) nullptr,
            .DMENU_Handlers_CheckpointItem =     (void *) nullptr,
            .DMENU_Handlers_FloatItem =          (void *) nullptr
        }
    },
    //#
    //## T2R 02.001.010 END
    //#




    

    //#
    //## T1X 02.002.000 BEGIN
    //#
    {
        T1X,
        V02002000,

        {
            .dev_menu_group       = (dmenu_menu_group_t *) 0x6a34a68, // (is actually a dmenu_menu_group_t **, but is read and replaced on ndgame struct init)
            .quick_menu_group     = (dmenu_menu_group_t *) 0x6a35720,
            .favorites_menu_group = (dmenu_menu_group_t *) 0x6a34a70,

            .f_Free          = (void *) 0x30ed250,
            .f_Malloc        = (void *) 0x30ed260,
            .f_LookupSymbol = (void *) 0x9a9150,
        
            .d_Novis = (byte *) 0x3a10794
             
        },

        {
            .DMENU_Types_MenuGroup =          (dmenu_vptr_t) 0x3938C78,
            
            .DMENU_Types_Menu =               (dmenu_vptr_t) 0x3938BA8,
            .DMENU_Types_DynMenu =            (dmenu_vptr_t) 0x38C40B8,
            .DMENU_Types_WeaponUpgradesMenu = (dmenu_vptr_t) 0x3932BB0,
            
            .DMENU_Types_ItemSubmenu =        (dmenu_vptr_t) 0x3938A90,
            .DMENU_Types_TaskItemSubmenu =    (dmenu_vptr_t) 0x39611F8,
            .DMENU_Types_SubnodeTaskItemSubmenu = (dmenu_vptr_t) 0x39610E0,
            
            .DMENU_Types_BaseComponent =      (dmenu_vptr_t) 0x3937c98,
            .DMENU_Types_SeparatorItem =      (dmenu_vptr_t) 0x39380B0,
            .DMENU_Types_ToggleItem =         (dmenu_vptr_t) 0x3938860,
            .DMENU_Types_ExecItem =           (dmenu_vptr_t) 0x3938978,
            .DMENU_Types_AltExecItem =        (dmenu_vptr_t) 0x3938630,
            .DMENU_Types_StringItem =         (dmenu_vptr_t) 0x39382E0,
            .DMENU_Types_StringListItem =     (dmenu_vptr_t) 0x3938748,
            .DMENU_Types_DecimalItem =        (dmenu_vptr_t) 0x3938510,
            .DMENU_Types_TaskHistoryItem =    (dmenu_vptr_t) 0x3937D68,
            .DMENU_Types_TaskItem =           (dmenu_vptr_t) 0x3960EB0,
            .DMENU_Types_UserItem =           (dmenu_vptr_t) 0x3960FC8,
            .DMENU_Types_TimerItem =          (dmenu_vptr_t) 0x39507E8,
            .DMENU_Types_CheckpointItem =     (dmenu_vptr_t) 0x381C180,
            .DMENU_Types_FloatItem =          (dmenu_vptr_t) 0x39381C8
        },
        
        {
            .DMENU_Handlers_MenuGroup =          (void *) nullptr,

            .DMENU_Handlers_Menu =               (void *) nullptr,
            .DMENU_Handlers_DynMenu =            (void *) nullptr,
            .DMENU_Handlers_WeaponUpgradesMenu = (void *) nullptr,
            
            .DMENU_Handlers_ItemSubmenu =        (void *) nullptr,
            .DMENU_Handlers_TaskItemSubmenu =    (void *) nullptr,
            .DMENU_Handlers_SubnodeTaskItemSubmenu = (void *) nullptr,
            
            .DMENU_Handlers_BaseComponent =      (void *) nullptr,
            .DMENU_Handlers_SeparatorItem =      (void *) nullptr,
            .DMENU_Handlers_ToggleItem =         (void *) 0x0114d100,
            .DMENU_Handlers_ExecItem =           (void *) nullptr,
            .DMENU_Handlers_AltExecItem =        (void *) nullptr,
            .DMENU_Handlers_StringItem =         (void *) nullptr,
            .DMENU_Handlers_StringListItem =     (void *) nullptr,
            .DMENU_Handlers_DecimalItem =        (void *) nullptr,
            .DMENU_Handlers_TaskHistoryItem =    (void *) nullptr,
            .DMENU_Handlers_TaskItem =           (void *) nullptr,
            .DMENU_Handlers_UserItem =           (void *) nullptr,
            .DMENU_Handlers_TimerItem =          (void *) nullptr,
            .DMENU_Handlers_CheckpointItem =     (void *) nullptr,
            .DMENU_Handlers_FloatItem =          (void *) nullptr
        }
    }
    //#
    //## T1X 02.002.000 END
    //#
};















// Load various function and variable addresses. Function's in the header file to try and make adding support less tedious
int do_load_ndgame(game_id_t game_id, app_version_sid_t app_ver)
{
    pid_t pid;
    pid = game->pid;
    address_table_t table;

    // Loop through the support table looking for an entry that matches the provided game id and app version
    for (unsigned i = 0; i < sizeof(TitleSupportTable); i++)
    {
        table = TitleSupportTable[i];

        if (game_id == table.game_id && app_ver == table.app_ver_sid)
        {
            game->Types = table.Types;
            game->EventHandlers = table.Handlers;
            game->Addrs = table.Addrs;

            // Load the menu base pointers
            game->Addrs.dev_menu_group       = (dmenu_menu_group_t *) read_ulong(pid, (unsigned long *) game->Addrs.dev_menu_group);
            game->Addrs.quick_menu_group     = (dmenu_menu_group_t *) read_ulong(pid, (unsigned long *) game->Addrs.quick_menu_group);
            game->Addrs.favorites_menu_group = (dmenu_menu_group_t *) read_ulong(pid, (unsigned long *) game->Addrs.favorites_menu_group);
            return 0;
        }
    }


    echo("ERROR: Unable to load addresses for current game. (Game ID: %d / app_ver SID: 0x%llX)", game_id, app_ver);
    return 1;
}
