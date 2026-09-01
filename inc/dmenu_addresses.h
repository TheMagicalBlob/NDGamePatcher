
//#
//## DMenu .elf Addresses (assuming a base address of 0x400000)
//#

typedef void * dmenu_vptr_t;

struct dmenu_types_t;








//#
//## The Last of Us Part II Remastered / T1X
//#
typedef struct dmenu_menu_base_s dmenu_menu_base_t;
#define T2RDMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6b63998
#define T2RDMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6b64670
#define T2RDMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6b639a0


#define T2R_FREE (void *) 0x02a8f680
#define T2R_MALLOC (void *) 0x02a8f690

#define T2R_NOVIS (byte *) 0x70cc124
#define T2R_LOOKUPSYMBOL (void *) 0x051c0c0



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










//#
//## The Last of Us Part I / T1X
//#
#define T1XDMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6a34a68
#define T1XDMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6a35720
#define T1XDMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6a34a70

#define T1X_FREE (void *) 0x30ed250
#define T1X_MALLOC (void *) 0x30ed260

#define T1X_NOVIS (byte *) 0x3a10794
#define T1X_LOOKUPSYMBOL (void *) 0x9a9150



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

