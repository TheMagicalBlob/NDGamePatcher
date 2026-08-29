


//#
//## The Last of Us Part II Remastered / T1x
//#
typedef struct dmenu_menu_base_s dmenu_menu_base_t;
#define T2RDMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6b63998
#define T2RDMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6b64670
#define T2RDMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6b639a0


#define T2R_FREE (void *) 0x02a8f680
#define T2R_MALLOC (void *) 0x02a8f690

#define T2R_NOVIS (byte *) 0x70cc124
#define T2R_LOOKUPSYMBOL (void *) 0x051c0c0









//#
//## The Last of Us Part I / T1x
//#
#define T1XDMENU_DEV_MENU_BASE_PTR       (dmenu_menu_base_t **) 0x6a34a68
#define T1XDMENU_QUICK_MENU_BASE_PTR     (dmenu_menu_base_t **) 0x6a35720
#define T1XDMENU_FAVORITES_MENU_BASE_PTR (dmenu_menu_base_t **) 0x6a34a70

#define T1X_FREE (void *) 0x30ed250
#define T1X_MALLOC (void *) 0x30ed260

#define T1X_NOVIS (byte *) 0x3a10794
#define T1X_LOOKUPSYMBOL (void *) 0x9a9150





// DMenu component virtual pointers
typedef enum : unsigned long
{
    // The Last of Us Part II Remastered
    t2r_vptr_menu_base = 0x322E688, // The base for each debug menu

    t2r_vptr_menu = 0x322E5B0, // The menus themselves which contain individual items
    t2r_vptr_dyn_menu = 0x31B5248, // Menus with dynamic contents that're cleared and repopulated every time they're opened
    t2r_vptr_weapon_upgrades_menu = 0x32280B0,

    t2r_vptr_item_submenu = 0x322E488,
    t2r_vptr_task_item_submenu = 0x3252D58,
    t2r_vptr_noreturn_task_item_submenu = 0x3252C30,

    t2r_vptr_base_component = 0x322d5c8, // 0x??
    t2r_vptr_separator_item = 0x322D6A0, // 0xA8
    t2r_vptr_bool_item = 0x322DA18, // 0xB8
    t2r_vptr_exec_item = 0x322E238, // 0xC0
    t2r_vptr_alt_exec_item = 0x322E360,
    t2r_vptr_string_item = 0x322DFE8,
    t2r_vptr_string_list_item = 0x322DC68,
    t2r_vptr_decimal_item = 0x322DEB8,
    t2r_vptr_task_history_item = 0x32529E0,
    t2r_vptr_task_item = 0x3252B08,
    t2r_vptr_user_item = 0x3243BF0, // Entirely useless, maybe it's meant to be replaced by an actual user name? idk
    t2r_vptr_timer_item = 0x3117960,
    t2r_vptr_checkpoint_item = 0x322DB40,
    t2r_vptr_float_item = 0x322E110, // 0x158






    // The Last of Us Remake / The Last of Us Part I
    t1x_vptr_menu_base = 0x3938C78,

    t1x_vptr_menu = 0x3938BA8,
    t1x_vptr_dyn_menu = 0x38C40B8,
    t1x_vptr_weapon_upgrades_menu = 0x3932BB0,

    t1x_vptr_item_submenu = 0x3938A90,
    t1x_vptr_task_item_submenu = 0x39611F8,
    t1x_vptr_subtask_item_submenu = 0x39610E0,

    t1x_vptr_base_component = 0x3937c98,
    t1x_vptr_bool_item = 0x39380B0,
    t1x_vptr_exec_item = 0x3938860,
    t1x_vptr_alt_exec_item = 0x3938978,
    t1x_vptr_string_item = 0x3938630,
    t1x_vptr_string_list_item = 0x39382E0,
    t1x_vptr_float_item = 0x3938748,
    t1x_vptr_decimal_item = 0x3938510,
    t1x_vptr_separator_item = 0x3937D68,
    t1x_vptr_task_history_item = 0x3960EB0,
    t1x_vptr_task_item = 0x3960FC8,
    t1x_vptr_user_item = 0x39507E8,
    t1x_vptr_timer_item = 0x381C180,
    t1x_vptr_checkpoint_item = 0x39381C8,





    
    // Uncharted 4: A Thief's End Remastered
    uc4r_vptr_menu_base = 0xFFFFFFFFFFFFFFFF,

    uc4rx_vptr_menu = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_id_dyn_menu = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_id_unk_menu = 0xFFFFFFFFFFFFFFFF,
    
    uc4r_vptr_item_submenu = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_task_item_submenu = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_multiplayer_task_item_submenu = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_survival_task_item_submenu = 0xFFFFFFFFFFFFFFFF,
    
    uc4r_vptr_bool_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_exec_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_alt_exec_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_string_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_string_list_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_float_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_decimal_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_separator_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_task_history_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_task_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_user_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_timer_item = 0xFFFFFFFFFFFFFFFF,
    uc4r_vptr_checkpoint_item = 0xFFFFFFFFFFFFFFFF,





    
    // Uncharted: The Lost Legacy Remastered
    tllr_vptr_menu_base = 0xFFFFFFFFFFFFFFFF,

    tllrx_vptr_menu = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_id_dyn_menu = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_id_unk_menu = 0xFFFFFFFFFFFFFFFF,
    
    tllr_vptr_item_submenu = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_task_item_submenu = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_noreturn_task_item_submenu = 0xFFFFFFFFFFFFFFFF,
    
    tllr_vptr_bool_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_exec_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_alt_exec_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_string_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_string_list_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_float_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_decimal_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_separator_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_task_history_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_task_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_user_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_timer_item = 0xFFFFFFFFFFFFFFFF,
    tllr_vptr_unk_item = 0xFFFFFFFFFFFFFFFF

} dmenu_vptr_t;
