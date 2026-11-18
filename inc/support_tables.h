//#
//## DMenu-related .elf Addresses, as well as some miscellaneous functions
//# (assumes a base address of 0x400000)

// typedef struct ndgame NdGame_t;

extern NdGame_t * game;

struct DMENU_types_t;
struct dmenu_menu_group_t;
typedef void * DMENU_Type_t;




typedef struct suppport_table_s
{
    GameId_t game_id;              // Id of the loaded game (T2R, T1X, UC4, etc.)
    app_version_sid_t app_ver_sid;  // The games app_ver encoded as an SID
    

    dmenu_menu_group_t * DevMenu;
    dmenu_menu_group_t * QuickMenu;
    dmenu_menu_group_t * FavoritesMenu;

    DMENU_Addresses_t Addrs;        // Table of function/variable addresses in the game process
    DMENU_Types_t Types;            // Table of dmenu vPtr's applicable to the current game's dmenu
    DMENU_EventHandlers_t Handlers; // Table of event handler function pointers

} suppport_table_t;






#if 0
//#
//## DEBUG support table(s)
//#

// Where support for individual titles and patches for said titles is actually added
// Array containing the structs of various important addresses to apply to the global NdGame_t struct when a supported game is detected
static const suppport_table_t TitleSupportTable[] =
{
    //#
    //## T2R 02.001.010 BEGIN
    //#
    {
        T2R,
        V02001010,

        // Menu Group pointer pointers (NOTE: dmenu_menu_group_t**'s. They're replaced with actual dmenu_menu_group_t*'s when loading the support table)
        .DevMenu       = (dmenu_menu_group_t *) 0x6b63998,
        .QuickMenu     = (dmenu_menu_group_t *) 0x6b64670,
        .FavoritesMenu = (dmenu_menu_group_t *) 0x6b639a0,

        { // Function & other misc. addresses

            .Free          = (void *) 0x02a8f680,
            .Malloc        = (void *) 0x02a8f690,
            .LookupSymbol  = (void *) 0x051c0c0,

            .Novis = (byte *) 0x70cc124
        },

        {0},

        {0}
    },
};


//#
//## Release support tables
//#
#else
// Where support for individual titles and patches for said titles is actually added
// Array containing the structs of various important addresses to apply to the global NdGame_t struct when a supported game is detected
static const suppport_table_t TitleSupportTable[] =
{
    //#
    //## T2R 02.001.010 BEGIN
    //#
    {
        T2R,
        V02001010,

        .DevMenu       = (dmenu_menu_group_t *) 0x6b63998, // (is actually a dmenu_menu_group_t **, but is read and replaced on ndgame struct init)
        .QuickMenu     = (dmenu_menu_group_t *) 0x6b64670,
        .FavoritesMenu = (dmenu_menu_group_t *) 0x6b639a0,

        { // Function & other misc. addresses

            .Free          = (void *) 0x02a8f680,
            .Malloc        = (void *) 0x02a8f690,
            .LookupSymbol  = (void *) 0x051c0c0,

            .Novis = (byte *) 0x70cc124
        },

        {
            .DMENU_Types_MenuGroup =           (DMENU_Type_t) 0x322E688,
            .DMENU_Types_Menu =               (DMENU_Type_t) 0x322E5B0,
            .DMENU_Types_DynMenu =            (DMENU_Type_t) 0x31B5248,
            .DMENU_Types_WeaponUpgradesMenu = (DMENU_Type_t) 0x32280B0,
            .DMENU_Types_ItemSubmenu =        (DMENU_Type_t) 0x322E488,
            .DMENU_Types_TaskItemSubmenu =    (DMENU_Type_t) 0x3252D58,
            .DMENU_Types_SubnodeTaskItemSubmenu = (DMENU_Type_t) 0x3252C30,
            .DMENU_Types_BaseComponent =      (DMENU_Type_t) 0x322d5c8,
            .DMENU_Types_SeparatorItem =      (DMENU_Type_t) 0x322D6A0,
            .DMENU_Types_ToggleItem =         (DMENU_Type_t) 0x322DA18,
            .DMENU_Types_ExecItem =           (DMENU_Type_t) 0x322E238,
            .DMENU_Types_AltExecItem =        (DMENU_Type_t) 0x322E360,
            .DMENU_Types_StringItem =         (DMENU_Type_t) 0x322DFE8,
            .DMENU_Types_StringListItem =     (DMENU_Type_t) 0x322DC68,
            .DMENU_Types_DecimalItem =        (DMENU_Type_t) 0x322DEB8,
            .DMENU_Types_TaskHistoryItem =    (DMENU_Type_t) 0x32529E0,
            .DMENU_Types_TaskItem =           (DMENU_Type_t) 0x3252B08,
            .DMENU_Types_UserItem =           (DMENU_Type_t) 0x3243BF0,
            .DMENU_Types_TimerItem =          (DMENU_Type_t) 0x3117960,
            .DMENU_Types_CheckpointItem =     (DMENU_Type_t) 0x322DB40,
            .DMENU_Types_FloatItem =          (DMENU_Type_t) 0x322E110
        },

        {
            .DMENU_Handlers_ItemSubmenu =        (void *) 0x0,
            .DMENU_Handlers_Menu          =      (void *) 0x0,
            .DMENU_Handlers_ToggleItem =         (void *) 0x8E72E0,
            .DMENU_Handlers_SeparatorItem =      (void *) 0x0,
            .DMENU_Handlers_StringListItem =     (void *) 0x1150910,
            // .DMENU_Handlers_DecimalItem =        (void *) 0x701,
            // .DMENU_Handlers_FloatItem =          (void *) 0x601,
            .DMENU_Handlers_ExecItem =           (void *) 0x2982700,
            .DMENU_Handlers_UserItem =           (void *) 0x0,
            .DMENU_Handlers_AltExecItem =        (void *) 0x1AED250,
            .DMENU_Handlers_TimerItem =          (void *) 0x1BD8660,
            .DMENU_Handlers_DynMenu =            (void *) 0x20B8CD0,
            .DMENU_Handlers_WeaponUpgradesMenu = (void *) 0x0,
            .DMENU_Handlers_TaskItemSubmenu =    (void *) 0x0,
            .DMENU_Handlers_TaskItem =           (void *) 0x0,
            .DMENU_Handlers_StringItem =         (void *) 0x13C026BDC8,
            .DMENU_Handlers_SubnodeTaskItemSubmenu = (void *) 0x0,
            .DMENU_Handlers_CheckpointItem =     (void *) 0x28A6E50,
            .DMENU_Handlers_TaskHistoryItem =    (void *) 0x28A6CC0
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

        // MenuGroups
        .DevMenu       = (dmenu_menu_group_t *) 0x6a34a68, // (is actually a dmenu_menu_group_t **, but is read and replaced on ndgame struct init)
        .QuickMenu     = (dmenu_menu_group_t *) 0x6a35720,
        .FavoritesMenu = (dmenu_menu_group_t *) 0x6a34a70,

        {
            .Free          = (void *) 0x30ed250,
            .Malloc        = (void *) 0x30ed260,
            .LookupSymbol  = (void *) 0x9a9150,
        
            .Novis = (byte *) 0x3a10794
             
        },

        {
            .DMENU_Types_MenuGroup =          (DMENU_Type_t) 0x3938C78,
            
            .DMENU_Types_Menu =               (DMENU_Type_t) 0x3938BA8,
            .DMENU_Types_DynMenu =            (DMENU_Type_t) 0x38C40B8,
            .DMENU_Types_WeaponUpgradesMenu = (DMENU_Type_t) 0x3932BB0,
            
            .DMENU_Types_ItemSubmenu =        (DMENU_Type_t) 0x3938A90,
            .DMENU_Types_TaskItemSubmenu =    (DMENU_Type_t) 0x39611F8,
            .DMENU_Types_SubnodeTaskItemSubmenu = (DMENU_Type_t) 0x39610E0,
            
            .DMENU_Types_BaseComponent =      (DMENU_Type_t) 0x3937c98,
            .DMENU_Types_SeparatorItem =      (DMENU_Type_t) 0x39380B0,
            .DMENU_Types_ToggleItem =         (DMENU_Type_t) 0x3938860,
            .DMENU_Types_ExecItem =           (DMENU_Type_t) 0x3938978,
            .DMENU_Types_AltExecItem =        (DMENU_Type_t) 0x3938630,
            .DMENU_Types_StringItem =         (DMENU_Type_t) 0x39382E0,
            .DMENU_Types_StringListItem =     (DMENU_Type_t) 0x3938748,
            .DMENU_Types_DecimalItem =        (DMENU_Type_t) 0x3938510,
            .DMENU_Types_TaskHistoryItem =    (DMENU_Type_t) 0x3937D68,
            .DMENU_Types_TaskItem =           (DMENU_Type_t) 0x3960EB0,
            .DMENU_Types_UserItem =           (DMENU_Type_t) 0x3960FC8,
            .DMENU_Types_TimerItem =          (DMENU_Type_t) 0x39507E8,
            .DMENU_Types_CheckpointItem =     (DMENU_Type_t) 0x381C180,
            .DMENU_Types_FloatItem =          (DMENU_Type_t) 0x39381C8
        },

        {
            .DMENU_Handlers_ItemSubmenu = (void *) 0x0,
            .DMENU_Handlers_Menu = (void *) 0x0,
            .DMENU_Handlers_SeparatorItem = (void *) 0x8E7C50,
            .DMENU_Handlers_TaskHistoryItem = (void *) 0x0,
            .DMENU_Handlers_StringItem = (void *) 0x28FA200,
            // .DMENU_Handlers_DecimalItem = (void *) 0x701,
            // .DMENU_Handlers_StringListItem = (void *) 0x601,
            .DMENU_Handlers_ToggleItem = (void *) 0x2F118E0,
            .DMENU_Handlers_TimerItem = (void *) 0x0,
            .DMENU_Handlers_ExecItem = (void *) 0x1A80DE0,
            .DMENU_Handlers_CheckpointItem = (void *) 0x1BD5B00,
            .DMENU_Handlers_DynMenu = (void *) 0x228B990,
            .DMENU_Handlers_WeaponUpgradesMenu = (void *) 0x0,
            .DMENU_Handlers_AltExecItem = (void *) 0x1400270BA8,
            .DMENU_Handlers_TaskItemSubmenu = (void *) 0x0,
            .DMENU_Handlers_UserItem = (void *) 0x0,
            .DMENU_Handlers_SubnodeTaskItemSubmenu = (void *) 0x0,
            .DMENU_Handlers_FloatItem = (void *) 0x2E17A30,
            .DMENU_Handlers_TaskItem = (void *) 0x2E17860
        }
    }
    //#
    //## T1X 02.002.000 END
    //#
};
#endif














// Load various function and variable addresses. Function's in the header file to try and make adding support less tedious
int InitSupportTable(GameId_t game_id, app_version_sid_t app_ver)
{
    suppport_table_t table;

    // Loop through the support table looking for an entry that matches the provided game id and app version
    for (unsigned i = 0; i < sizeof(TitleSupportTable); i++)
    {
        table = TitleSupportTable[i];

        if (game_id == table.game_id && app_ver == table.app_ver_sid)
        {
            game->Types = table.Types;
            game->EventHandlers = table.Handlers;
            game->Addrs = table.Addrs;

            // Load the menu group pointers
            game->DevMenu       = (dmenu_menu_group_t *) read_ulong((unsigned long *) table.DevMenu);
            game->QuickMenu     = (dmenu_menu_group_t *) read_ulong((unsigned long *) table.QuickMenu);
            game->FavoritesMenu = (dmenu_menu_group_t *) read_ulong((unsigned long *) table.FavoritesMenu);
            return 0;
        }
    }


    echo("ERROR: Unable to load addresses for current game. (Game ID: %d / app_ver SID: 0x%llX)", game_id, app_ver);
    return 1;
}
