#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
	

	.als_level  = { 5, 10,   25,   50,  100, 150,  200,  400, 1000,  1500,  2000,  3000,  5000, 8000,  10000, 10000},
    .als_value  = {10, 50,  120,  120,  200, 250,  280,  280, 1000,  1600,  1600,  6000,  6000, 9000,  10240, 10240},

#if defined (SLT_DRV_W8666_ALSPS)
	.ps_threshold_high =280, //320
    .ps_threshold_low = 200,//240 
#elif defined(SLT_DRV_LXF_W8666_B01_ALSPS)
   	.ps_threshold_high =240, 
    .ps_threshold_low = 160,  
#else
	//.ps_threshold_high =160,//180,//280,//180,// 65
   // .ps_threshold_low =120,//140,//240, //240,  //45
#endif
	
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
