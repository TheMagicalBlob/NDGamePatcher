//#
//## DMenu-related .elf Addresses, as well as some miscellaneous functions
//# (assumes a base address of 0x400000)

extern ndgame_t * game;

struct dmenu_types_t;
struct dmenu_menu_base_t;
typedef void * dmenu_vptr_t;




typedef struct address_table_s
{
    game_id_t game_id;             // Id of the loaded game (T2R, T1X, UC4, etc.)
    app_version_sid_t app_ver_sid; // The games app_ver encoded as an SID
    dmenu_addresses_t addresses;   // Table of function/variable addresses in the game process
    dmenu_types_t types;           // Table of dmenu vPtr's applicable to the current game's dmenu

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
            .menu_base = (dmenu_vptr_t) 0x322E688,

            .menu = (dmenu_vptr_t) 0x322E5B0,
            .dyn_menu = (dmenu_vptr_t) 0x31B5248,
            .weapon_upgrades_menu = (dmenu_vptr_t) 0x32280B0,
            
            .item_submenu = (dmenu_vptr_t) 0x322E488,
            .task_item_submenu = (dmenu_vptr_t) 0x3252D58,
            .subnode_task_item_submenu = (dmenu_vptr_t) 0x3252C30,
            
            .base_component = (dmenu_vptr_t) 0x322d5c8,
            .separator_item = (dmenu_vptr_t) 0x322D6A0,
            .toggle_item = (dmenu_vptr_t) 0x322DA18,
            .exec_item = (dmenu_vptr_t) 0x322E238,
            .alt_exec_item = (dmenu_vptr_t) 0x322E360,
            .string_item = (dmenu_vptr_t) 0x322DFE8,
            .string_list_item = (dmenu_vptr_t) 0x322DC68,
            .decimal_item = (dmenu_vptr_t) 0x322DEB8,
            .task_history_item = (dmenu_vptr_t) 0x32529E0,
            .task_item = (dmenu_vptr_t) 0x3252B08,
            .user_item = (dmenu_vptr_t) 0x3243BF0,
            .timer_item = (dmenu_vptr_t) 0x3117960,
            .checkpoint_item = (dmenu_vptr_t) 0x322DB40,
            .float_item = (dmenu_vptr_t) 0x322E110,
        },
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
            .menu_base = (dmenu_vptr_t) 0x3938C78,

            .menu = (dmenu_vptr_t) 0x3938BA8,
            .dyn_menu = (dmenu_vptr_t) 0x38C40B8,
            .weapon_upgrades_menu = (dmenu_vptr_t) 0x3932BB0,
            
            .item_submenu = (dmenu_vptr_t) 0x3938A90,
            .task_item_submenu = (dmenu_vptr_t) 0x39611F8,
            .subnode_task_item_submenu = (dmenu_vptr_t) 0x39610E0,
            
            .base_component = (dmenu_vptr_t) 0x3937c98,
            .separator_item = (dmenu_vptr_t) 0x39380B0,
            .toggle_item = (dmenu_vptr_t) 0x3938860,
            .exec_item = (dmenu_vptr_t) 0x3938978,
            .alt_exec_item = (dmenu_vptr_t) 0x3938630,
            .string_item = (dmenu_vptr_t) 0x39382E0,
            .string_list_item = (dmenu_vptr_t) 0x3938748,
            .decimal_item = (dmenu_vptr_t) 0x3938510,
            .task_history_item = (dmenu_vptr_t) 0x3937D68,
            .task_item = (dmenu_vptr_t) 0x3960EB0,
            .user_item = (dmenu_vptr_t) 0x3960FC8,
            .timer_item = (dmenu_vptr_t) 0x39507E8,
            .checkpoint_item = (dmenu_vptr_t) 0x381C180,
            .float_item = (dmenu_vptr_t) 0x39381C8
        }
    }
    //#
    //## T1X 02.002.000 END
    //#
};






//############################################
//## The Last of Us Part II Remastered / T2R #
//############################################









//#################################
//## The Last of Us Remake/Part I #
//#################################








// Load various function and variable addresses. Function's in the header file to try and make adding support less tedious
int do_load_ndgame(game_id_t game_id, app_version_sid_t app_ver)
{
    pid_t pid;
    pid = game->pid;

    for (int i = 0; i < sizeof(fuck); i++)
    {
        if (game_id == fuck[i].game_id && app_ver == fuck[i].app_ver_sid)
        {
            game->vptrs = fuck[i].types;
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
