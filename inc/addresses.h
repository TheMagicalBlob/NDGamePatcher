//#
//## DMenu-related .elf Addresses, as well as some miscellaneous functions
//# (assumes a base address of 0x400000)

extern ndgame_t * game;

struct dmenu_types_t;
struct dmenu_menu_base_t;
typedef void * dmenu_vptr_t;








//############################################
//## The Last of Us Part II Remastered / T2R #
//############################################



//#
//## 02.001.010
//#

#define DMENU_T2R_02001010_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6b63998
#define DMENU_T2R_02001010_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6b64670
#define DMENU_T2R_02001010_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6b639a0


#define DMENU_T2R_02001010_FREE (void *) 0x02a8f680
#define DMENU_T2R_02001010_MALLOC (void *) 0x02a8f690
#define DMENU_T2R_02001010_LOOKUPSYMBOL (void *) 0x051c0c0

#define DMENU_T2R_02001010_NOVIS (byte *) 0x70cc124



// The Last of Us Part II Remastered
// DMenu component virtual pointers
const dmenu_types_t dmenu_t2r_02001010_vptrs =
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
    .bool_item = (dmenu_vptr_t) 0x322DA18,
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
};










//#################################
//## The Last of Us Remake/Part I #
//#################################



//#
//## 02.002.000
//#

#define DMENU_T1X_02002000_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6a34a68
#define DMENU_T1X_02002000_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6a35720
#define DMENU_T1X_02002000_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6a34a70

#define DMENU_T1X_02002000_FREE (void *) 0x30ed250
#define DMENU_T1X_02002000_MALLOC (void *) 0x30ed260
#define DMENU_T1X_02002000_LOOKUPSYMBOL (void *) 0x9a9150

#define DMENU_T1X_02002000_NOVIS (byte *) 0x3a10794



// The Last of Us Remake / The Last of Us Part I
// DMenu component virtual pointers
const dmenu_types_t dmenu_t1x_02002000_vptrs =
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
    .bool_item = (dmenu_vptr_t) 0x3938860,
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
};







// Load various function and variable addresses. Function's in the header file to try and make adding support less tedious
int do_load_ndgame(game_id_t game_id, app_version_sid_t app_ver)
{
    pid_t pid;
    pid = game->pid;

    switch(game_id | app_ver)
    {
        //#
        //## The Last of Us Part II Remastered (addresses from PPSA10662)
        //#

         // 02.001.010
        case T2R | V02001010:
            game->dev_menu_base       = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) DMENU_T2R_02001010_DEV_MENU_BASE_PTR);
            game->quick_menu_base     = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) DMENU_T2R_02001010_QUICK_MENU_BASE_PTR);
            game->favorites_menu_base = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) DMENU_T2R_02001010_FAVORITES_MENU_BASE_PTR);

            game->dat.novis = DMENU_T2R_02001010_NOVIS;
            game->func.malloc = DMENU_T2R_02001010_MALLOC;
            game->func.free = DMENU_T2R_02001010_FREE;
            game->func.lookup_symbol = DMENU_T2R_02001010_LOOKUPSYMBOL;

            game->vptrs = &dmenu_t2r_02001010_vptrs;

        //#
        //## The Last of Us Remake/Part I (addresses from PPSA03396)
        //#

        // 02.002.000
        case T1X | V02002000:
            game->dev_menu_base       = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) DMENU_T1X_02002000_DEV_MENU_BASE_PTR);
            game->quick_menu_base     = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) DMENU_T1X_02002000_QUICK_MENU_BASE_PTR);
            game->favorites_menu_base = (dmenu_menu_base_t *) read_ulong(pid, (unsigned long *) DMENU_T1X_02002000_FAVORITES_MENU_BASE_PTR);

            game->dat.novis = DMENU_T1X_02002000_NOVIS;
            game->func.malloc = DMENU_T1X_02002000_MALLOC;
            game->func.free = DMENU_T1X_02002000_FREE;
            game->func.lookup_symbol = DMENU_T1X_02002000_LOOKUPSYMBOL;

            game->vptrs = &dmenu_t1x_02002000_vptrs;
            break;


        default:
            return 1;
    }

    return 0;
}
//! TODO: Simplify this to reduce the amount of manual work needed for every individual version support is added for for every single game
