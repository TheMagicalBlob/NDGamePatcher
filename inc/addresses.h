//#
//## DMenu-related .elf Addresses, as well as some miscellaneous functions
//# (assumes a base address of 0x400000)

// typedef struct ndgame ndgame_t;

extern ndgame_t * game;

struct dmenu_types_t;
struct dmenu_menu_base_t;
typedef void * dmenu_vptr_t;




typedef struct address_table_s
{
    game_id_t game_id;              // Id of the loaded game (T2R, T1X, UC4, etc.)
    app_version_sid_t app_ver_sid;  // The games app_ver encoded as an SID
    dmenu_addresses_t addresses;    // Table of function/variable addresses in the game process
    DMenuTypes_t types;             // Table of dmenu vPtr's applicable to the current game's dmenu
    DMenu_EventHandlers_t Handlers; //! write me

} address_table_t;



static const address_table_t fuck[] =
{
    //#
    //## T2R 02.001.010 BEGIN
    //#
    {
        T2R,
        V02001010,

        {
            .dev_menu_base       = (dmenu_menu_base_t *) 0x6b63998, // (is actually a dmenu_menu_base_t **, but is read and replaced on ndgame struct init)
            .quick_menu_base     = (dmenu_menu_base_t *) 0x6b64670,
            .favorites_menu_base = (dmenu_menu_base_t *) 0x6b639a0,

            .func = {
                .free          = (void *) 0x02a8f680,
                .malloc        = (void *) 0x02a8f690,
                .lookup_symbol = (void *) 0x051c0c0,
            },

            .dat = {
                .novis = (byte *) 0x70cc124
            } 
        },

        {
            .DMenu_Types_MenuBase =           (dmenu_vptr_t) 0x322E688,
            .DMenu_Types_Menu =               (dmenu_vptr_t) 0x322E5B0,
            .DMenu_Types_DynMenu =            (dmenu_vptr_t) 0x31B5248,
            .DMenu_Types_WeaponUpgradesMenu = (dmenu_vptr_t) 0x32280B0,
            .DMenu_Types_ItemSubmenu =        (dmenu_vptr_t) 0x322E488,
            .DMenu_Types_TaskItemSubmenu =    (dmenu_vptr_t) 0x3252D58,
            .DMenu_Types_SubnodeTaskItemSubmenu = (dmenu_vptr_t) 0x3252C30,
            .DMenu_Types_BaseComponent =      (dmenu_vptr_t) 0x322d5c8,
            .DMenu_Types_SeparatorItem =      (dmenu_vptr_t) 0x322D6A0,
            .DMenu_Types_ToggleItem =         (dmenu_vptr_t) 0x322DA18,
            .DMenu_Types_ExecItem =           (dmenu_vptr_t) 0x322E238,
            .DMenu_Types_AltExecItem =        (dmenu_vptr_t) 0x322E360,
            .DMenu_Types_StringItem =         (dmenu_vptr_t) 0x322DFE8,
            .DMenu_Types_StringListItem =     (dmenu_vptr_t) 0x322DC68,
            .DMenu_Types_DecimalItem =        (dmenu_vptr_t) 0x322DEB8,
            .DMenu_Types_TaskHistoryItem =    (dmenu_vptr_t) 0x32529E0,
            .DMenu_Types_TaskItem =           (dmenu_vptr_t) 0x3252B08,
            .DMenu_Types_UserItem =           (dmenu_vptr_t) 0x3243BF0,
            .DMenu_Types_TimerItem =          (dmenu_vptr_t) 0x3117960,
            .DMenu_Types_CheckpointItem =     (dmenu_vptr_t) 0x322DB40,
            .DMenu_Types_FloatItem =          (dmenu_vptr_t) 0x322E110,
        },

        {
            .DMenu_Handlers_MenuBase =           (void *) nullptr,

            .DMenu_Handlers_Menu =               (void *) nullptr,
            .DMenu_Handlers_DynMenu =            (void *) nullptr,
            .DMenu_Handlers_WeaponUpgradesMenu = (void *) nullptr,
            
            .DMenu_Handlers_ItemSubmenu =        (void *) nullptr,
            .DMenu_Handlers_TaskItemSubmenu =    (void *) nullptr,
            .DMenu_Handlers_SubnodeTaskItemSubmenu = (void *) nullptr,
            
            .DMenu_Handlers_BaseComponent =      (void *) nullptr,
            .DMenu_Handlers_SeparatorItem =      (void *) nullptr,
            .DMenu_Handlers_ToggleItem =         (void *) nullptr,
            .DMenu_Handlers_ExecItem =           (void *) nullptr,
            .DMenu_Handlers_AltExecItem =        (void *) nullptr,
            .DMenu_Handlers_StringItem =         (void *) nullptr,
            .DMenu_Handlers_StringListItem =     (void *) nullptr,
            .DMenu_Handlers_DecimalItem =        (void *) nullptr,
            .DMenu_Handlers_TaskHistoryItem =    (void *) nullptr,
            .DMenu_Handlers_TaskItem =           (void *) nullptr,
            .DMenu_Handlers_UserItem =           (void *) nullptr,
            .DMenu_Handlers_TimerItem =          (void *) nullptr,
            .DMenu_Handlers_CheckpointItem =     (void *) nullptr,
            .DMenu_Handlers_FloatItem =          (void *) nullptr
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
            .dev_menu_base       = (dmenu_menu_base_t *) 0x6a34a68, // (is actually a dmenu_menu_base_t **, but is read and replaced on ndgame struct init)
            .quick_menu_base     = (dmenu_menu_base_t *) 0x6a35720,
            .favorites_menu_base = (dmenu_menu_base_t *) 0x6a34a70,

            .func = {
                .free          = (void *) 0x30ed250,
                .malloc        = (void *) 0x30ed260,
                .lookup_symbol = (void *) 0x9a9150,
            },

            .dat = {
                .novis = (byte *) 0x3a10794
            } 
        },

        {
            .DMenu_Types_MenuBase =           (dmenu_vptr_t) 0x3938C78,
            
            .DMenu_Types_Menu =               (dmenu_vptr_t) 0x3938BA8,
            .DMenu_Types_DynMenu =            (dmenu_vptr_t) 0x38C40B8,
            .DMenu_Types_WeaponUpgradesMenu = (dmenu_vptr_t) 0x3932BB0,
            
            .DMenu_Types_ItemSubmenu =        (dmenu_vptr_t) 0x3938A90,
            .DMenu_Types_TaskItemSubmenu =    (dmenu_vptr_t) 0x39611F8,
            .DMenu_Types_SubnodeTaskItemSubmenu = (dmenu_vptr_t) 0x39610E0,
            
            .DMenu_Types_BaseComponent =      (dmenu_vptr_t) 0x3937c98,
            .DMenu_Types_SeparatorItem =      (dmenu_vptr_t) 0x39380B0,
            .DMenu_Types_ToggleItem =         (dmenu_vptr_t) 0x3938860,
            .DMenu_Types_ExecItem =           (dmenu_vptr_t) 0x3938978,
            .DMenu_Types_AltExecItem =        (dmenu_vptr_t) 0x3938630,
            .DMenu_Types_StringItem =         (dmenu_vptr_t) 0x39382E0,
            .DMenu_Types_StringListItem =     (dmenu_vptr_t) 0x3938748,
            .DMenu_Types_DecimalItem =        (dmenu_vptr_t) 0x3938510,
            .DMenu_Types_TaskHistoryItem =    (dmenu_vptr_t) 0x3937D68,
            .DMenu_Types_TaskItem =           (dmenu_vptr_t) 0x3960EB0,
            .DMenu_Types_UserItem =           (dmenu_vptr_t) 0x3960FC8,
            .DMenu_Types_TimerItem =          (dmenu_vptr_t) 0x39507E8,
            .DMenu_Types_CheckpointItem =     (dmenu_vptr_t) 0x381C180,
            .DMenu_Types_FloatItem =          (dmenu_vptr_t) 0x39381C8
        },
        
        {
            .DMenu_Handlers_MenuBase =           (void *) nullptr,

            .DMenu_Handlers_Menu =               (void *) nullptr,
            .DMenu_Handlers_DynMenu =            (void *) nullptr,
            .DMenu_Handlers_WeaponUpgradesMenu = (void *) nullptr,
            
            .DMenu_Handlers_ItemSubmenu =        (void *) nullptr,
            .DMenu_Handlers_TaskItemSubmenu =    (void *) nullptr,
            .DMenu_Handlers_SubnodeTaskItemSubmenu = (void *) nullptr,
            
            .DMenu_Handlers_BaseComponent =      (void *) nullptr,
            .DMenu_Handlers_SeparatorItem =      (void *) nullptr,
            .DMenu_Handlers_ToggleItem =         (void *) 0x0114d100,
            .DMenu_Handlers_ExecItem =           (void *) nullptr,
            .DMenu_Handlers_AltExecItem =        (void *) nullptr,
            .DMenu_Handlers_StringItem =         (void *) nullptr,
            .DMenu_Handlers_StringListItem =     (void *) nullptr,
            .DMenu_Handlers_DecimalItem =        (void *) nullptr,
            .DMenu_Handlers_TaskHistoryItem =    (void *) nullptr,
            .DMenu_Handlers_TaskItem =           (void *) nullptr,
            .DMenu_Handlers_UserItem =           (void *) nullptr,
            .DMenu_Handlers_TimerItem =          (void *) nullptr,
            .DMenu_Handlers_CheckpointItem =     (void *) nullptr,
            .DMenu_Handlers_FloatItem =          (void *) nullptr
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

    for (unsigned i = 0; i < sizeof(fuck); i++)
    {
        if (game_id == fuck[i].game_id && app_ver == fuck[i].app_ver_sid)
        {
            game->vptrs = fuck[i].types;
            game->Handlers = fuck[i].Handlers;
            game->addrs = fuck[i].addresses;

            // Load the menu base pointers
            game->addrs.dev_menu_base       = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) game->addrs.dev_menu_base);
            game->addrs.quick_menu_base     = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) game->addrs.quick_menu_base);
            game->addrs.favorites_menu_base = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) game->addrs.favorites_menu_base);
            return 0;
        }
    }


    echo("ERROR: Unable to load addresses for current game. (Game ID: %d / app_ver SID: 0x%llX)", game_id, app_ver);
    return 1;
}
