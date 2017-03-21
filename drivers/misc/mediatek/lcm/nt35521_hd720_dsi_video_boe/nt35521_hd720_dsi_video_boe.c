#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
    #include <platform/disp_drv_platform.h>
	
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    #include <linux/delay.h>
    #include <mach/mt_gpio.h>
#endif
#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif


#define LCD_MODULE_ID                        0x09
#define LCM_ID                               0x55
//const static unsigned char LCD_MODULE_ID   0x09;//ID0->1;ID1->X
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE								0
#define FRAME_WIDTH  									(720)
#define FRAME_HEIGHT 									(1280)

#define REGFLAG_DELAY             							 0xFC
#define REGFLAG_END_OF_TABLE      							 0xFD   // END OF REGISTERS MARKER


#define GPIO_LCD_BIAS_ENP_PIN          GPIO12
#define GPIO_LCD_BIAS_ENN_PIN          GPIO105
#define GPIO_DISP_LRSTB_PIN            GPIO112
#define GPIO_LCD_DRV_EN_PIN            GPIO90
//#define GPIO_DISP_ID0_PIN
//#define GPIO_DISP_ID1_PIN



#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;           //only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

//const static unsigned int BL_MIN_LEVEL =20;
static LCM_UTIL_FUNCS lcm_util = {0};
#define SET_RESET_PIN(v)    	     (lcm_util.set_reset_pin((v)))
#define UDELAY(n)                              (lcm_util.udelay(n))
#define MDELAY(n)                             (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)								lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)									lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[128];
};

//update initial param for IC boe_nt35521 0.01
static struct LCM_setting_table lcm_initialization_setting[] = {
	//CCMON
    	{0xFF,	4,  	{0xAA,0x55,0xA5,0x80}},
	{0x6F,	2,  	{0x11,0x00}},
	{0xF7,  2,  	{0x20,0x00}},
	{0x6F,  1,  	{0x06}},
	{0xF7,  1,  	{0xA0}},
	{0x6F,  1,  	{0x19}},
	{0xF7,  1,  	{0x12}},
	{0x6F,	1,	{0x02}},
	{0xF7,	1,	{0x47}},
	{0x6F,	1,	{0x17}},
	{0xF4,	1,	{0x70}},
	{0x6F,	1,	{0x01}},
	{0xF9,	1,	{0x46}},
	
	//Page 0
	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x00}},
	{0xBD,	5,	{0x01,0xA0,0x10,0x10,0x01}},
	//{0xBD,	5,	{0x01,0xA0,0x0C,0x08,0x01}},
	{0xB8,	4,	{0x01,0x02,0x0C,0x02}},
	{0xBB,	2,	{0x11,0x11}},	
	{0xBC,	2,	{0x00,0x00}},
	{0xB6,	1,	{0x04}},
	{0xC8,	1,	{0x80}},
	{0xD9,	2,	{0x01,0x01}},
	{0xD4,	1,	{0xC7}},
	
	//Page 1
	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x01}},
	{0xB0,	2,	{0x09,0x09}},
	{0xB1,	2,	{0x09,0x09}},
	{0xBC,	2,	{0x80,0x00}},
	{0xBD,	2,	{0x80,0x00}},
	{0xCA,	1,	{0x00}},
	{0xC0,	1,	{0x0C}},
	{0xB5,	2,	{0x03,0x03}},
	//{0xBE,	1,	{0x59}},
	{0xB3,	2,	{0x19,0x19}},
	{0xB4,	2,	{0x19,0x19}},
	{0xB9,	2,	{0x26,0x26}},
	{0xBA,	2,	{0x24,0x24}},

	//Page 2 Gamma
	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x02}},
	{0xEE,	1,	{0x01}},
	/*
	//
	{0xB0,	16,	{0x00,0x20,0x00,0x31,0x00,0x4C,0x00,0x62,0x00,0x76,0x00,0x97,0x00,0xB2,0x00,0xDF}},
	{0xB1,	16,	{0x01,0x05,0x01,0x41,0x01,0x73,0x01,0xC1,0x02,0x03,0x02,0x04,0x02,0x3D,0x02,0x7B}},
	{0xB2,	16,	{0x02,0xA0,0x02,0xD4,0x02,0xF8,0x03,0x28,0x03,0x46,0x03,0x6E,0x03,0x87,0x03,0xB6}},
	{0xB3,	4,	{0x03,0xF9,0x03,0xFF}},
	{0xC0,	1,	{0x04}},	
        */
	//gamma2.2 
	{0xB0,	16,	
	{0x00,0x43,0x00,0x4F,0x00,0x65,0x00,0x78,0x00,0x89,0x00,0xA7,0x00,0xBF,0x00,0xEB}},
	{0xB1,	16,
	{0x01,0x0F,0x01,0x49,0x01,0x77,0x01,0xC4,0x02,0x02,0x02,0x04,0x02,0x3C,0x02,0x7C}},
	{0xB2,	16,
	{0x02,0xA1,0x02,0xD5,0x02,0xF8,0x03,0x28,0x03,0x46,0x03,0x70,0x03,0x8B,0x03,0xAA}},
	{0xB3,	4,	{0x03,0xDF,0x03,0xFF}},
	{0xC0,	1,	{0x04}},	

        /*
	//gamma2.5
	{0xB0,	16,	
	{0x00,0x43,0x00,0x50,0x00,0x49,0x00,0x7E,0x00,0x90,0x00,0xB0,0x00,0xCB,0x00,0xF9}},
	{0xB1,	16,	
	{0x01,0x1E,0x01,0x5B,0x01,0x8B,0x01,0xDA,0x02,0x19,0x02,0x1B,0x02,0x57,0x02,0x97}},
	{0xB2,	16,	
	{0x02,0xC5,0x02,0xF8,0x03,0x21,0x03,0x54,0x03,0x73,0x03,0x97,0x03,0xB1,0x03,0xD2}},
	{0xB3,	4,	{0x03,0xDE,0x03,0xFF}},
	{0xC0,	1,	{0x04}},	

	//gamma1.9
	{0xB0,	16,	
	{0x00,0x43,0x00,0x4D,0x00,0x60,0x00,0x72,0x00,0x81,0x00,0x9C,0x00,0xB4,0x00,0xDB}},
	{0xB1,	16,	
	{0x00,0xFD,0x01,0x35,0x01,0x63,0x01,0xAE,0x01,0xED,0x01,0xEF,0x02,0x27,0x02,0x65}},
	{0xB2,	16,	
	{0x02,0x85,0x02,0xB8,0x02,0xD3,0x03,0x01,0x03,0x20,0x03,0x45,0x03,0x66,0x03,0x86}},
	{0xB3,	4,	{0x03,0xC8,0x03,0xFF}},
	{0xC0,	1,	{0x04}},	

	//gamma test
	{0xB0,	16,	
	{0x00,0x20,0x00,0x45,0x00,0x6E,0x00,0x8C,0x00,0xA2,0x00,0xC5,0x00,0xE8,0x01,0x19}},
	{0xB1,	16,	
	{0x01,0x41,0x01,0x7C,0x01,0xAB,0x01,0xFA,0x02,0x35,0x02,0x38,0x02,0x6E,0x02,0xA7}},
	{0xB2,	16,
	{0x02,0xCF,0x03,0x04,0x03,0x2A,0x03,0x5C,0x03,0x7F,0x03,0xA6,0x03,0xBF,0x03,0xD8}},
	{0xB3,	4,	{0x03,0xF4,0x03,0xFF}},
	{0xC0,	1,	{0x04}},	

	
	*/
	
	//Page 6
	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x06}},
	{0xB0,	2,	{0x10,0x12}},
	{0xB1,	2,	{0x14,0x16}},
	{0xB2,	2,	{0x00,0x02}},
	{0xB3,	2,	{0x31,0x31}},
	{0xB4,	2,	{0x31,0x34}},
	{0xB5,	2,	{0x34,0x34}},
	{0xB6,	2,	{0x34,0x31}},
	{0xB7,	2,	{0x31,0x31}},
	{0xB8,	2,	{0x31,0x31}},
	{0xB9,	2,	{0x2D,0x2E}},
	{0xBA,	2,	{0x2E,0x2D}},
	{0xBB,	2,	{0x31,0x31}},
	{0xBC,	2,	{0x31,0x31}},
	{0xBD,	2,	{0x31,0x34}},
	{0xBE,	2,	{0x34,0x34}},
	{0xBF,	2,	{0x34,0x31}},
	{0xC0,	2,	{0x31,0x31}},
	{0xC1,	2,	{0x03,0x01}},
	{0xC2,	2,	{0x17,0x15}},
	{0xC3,	2,	{0x13,0x11}},
	{0xE5,	2,	{0x31,0x31}},
	{0xC4,	2,	{0x17,0x15}},
	{0xC5,	2,	{0x13,0x11}},
	{0xC6,	2,	{0x03,0x01}},
	{0xC7,	2,	{0x31,0x31}},
	{0xC8,	2,	{0x31,0x34}},
	{0xC9,	2,	{0x34,0x34}},
	{0xCA,	2,	{0x34,0x31}},
	{0xCB,	2,	{0x31,0x31}},
	{0xCC,	2,	{0x31,0x31}},
	{0xCD,	2,	{0x2E,0x2D}},
	{0xCE,	2,	{0x2D,0x2E}},
	{0xCF,	2,	{0x31,0x31}},
	{0xD0,	2,	{0x31,0x31}},
	{0xD1,	2,	{0x31,0x34}},
	{0xD2,	2,	{0x34,0x34}},
	{0xD3,	2,	{0x34,0x31}},
	{0xD4,	2,	{0x31,0x31}},
	{0xD5,	2,	{0x00,0x02}},
	{0xD6,	2,	{0x10,0x12}},
	{0xD7,	2,	{0x14,0x16}},
	{0xE6,	2,	{0x32,0x32}},
	{0xD8,	5,	{0x00,0x00,0x00,0x00,0x00}},
	{0xD9,	5,	{0x00,0x00,0x00,0x00,0x00}},
	{0xE7,	1,	{0x00}},
	
	//Page 5
	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x05}},
	{0xED,	1,	{0x30}},
	{0xB0,	2,	{0x17,0x06}},
	{0xB8,	1,	{0x00}},
	{0xC0,	1,	{0x0D}},
	{0xC1,	1,	{0x0B}},
	{0xC2,	1,	{0x23}},
	{0xC3,	1,	{0x40}},
	{0xC4,	1,	{0x84}},
	{0xC5,	1,	{0x82}},
	{0xC6,	1,	{0x82}},
	{0xC7,	1,	{0x80}},
	{0xC8,	2,	{0x0B,0x30}},
	{0xC9,	2,	{0x05,0x10}},
	{0xCA,	2,	{0x01,0x10}},
	{0xCB,	2,	{0x01,0x10}},
	{0xD1,	5,	{0x03,0x05,0x05,0x07,0x00}},
	{0xD2,	5,	{0x03,0x05,0x09,0x03,0x00}},
	{0xD3,	5,	{0x00,0x00,0x6A,0x07,0x10}},
	{0xD4,	5,	{0x30,0x00,0x6A,0x07,0x10}},
	
	//Page 3
	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x03}},
	{0xB0,	2,	{0x00,0x00}},
	{0xB1,	2,	{0x00,0x00}},
	{0xB2,	5,	{0x05,0x00,0x0A,0x00,0x00}},
	{0xB3,	5,	{0x05,0x00,0x0A,0x00,0x00}},
	{0xB4,	5,	{0x05,0x00,0x0A,0x00,0x00}},
	{0xB5,	5,	{0x05,0x00,0x0A,0x00,0x00}},
	{0xB6,	5,	{0x02,0x00,0x0A,0x00,0x00}},
	{0xB7,	5,	{0x02,0x00,0x0A,0x00,0x00}},
	{0xB8,	5,	{0x02,0x00,0x0A,0x00,0x00}},
	{0xB9,	5,	{0x02,0x00,0x0A,0x00,0x00}},
	{0xBA,	5,	{0x53,0x00,0x0A,0x00,0x00}},
	{0xBB,	5,	{0x53,0x00,0x0A,0x00,0x00}},
	{0xBC,	5,	{0x53,0x00,0x0A,0x00,0x00}},
	{0xBD,	5,	{0x53,0x00,0x0A,0x00,0x00}},
	{0xC4,	1,	{0x60}},
	{0xC5,	1,	{0x40}},
	{0xC6,	1,	{0x64}},
	{0xC7,	1,	{0x44}},
	
	{0x6F,	1,	{0x11}},
	{0xF3,	1,	{0x01}},
	
	//{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x00}},
	//{0xee,	4,	{0x87,0x78,0x02,0x40}},
	//{0xeF,	2,	{0x07,0xff}},
	
	{0x11,	1,	{0x00}},
	{REGFLAG_DELAY, 120, {}},
	
	{0x29,	1,	{0x00}},
	{REGFLAG_DELAY, 20, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}},
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    //Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {
            case REGFLAG_DELAY :
                if(table[i].count <= 10)
                    mdelay(table[i].count);
                else
                    msleep(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));
	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

#if (LCM_DSI_CMD_MODE)
    	params->dsi.mode   = CMD_MODE;
#else
    	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif


       #ifdef SLT_DEVINFO_LCM
					params->module="BV055HDM-N00-1903";
					params->vendor="BOE";
					params->ic="NT35521";
					params->info="720*1280";
       #endif

        
	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;

	//The following defined the fomat for data coming from LCD engine.//
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;

	params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;
	
	// Highly depends on LCD driver capability.//
	//params->dsi.packet_size=256;

	//video mode timing
    	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 4;
    params->dsi.vertical_backporch				= 40;
    params->dsi.vertical_frontporch				= 40;
    params->dsi.vertical_active_line				= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active			= 4;
    params->dsi.horizontal_backporch				= 75; //82->75
    params->dsi.horizontal_frontporch				= 75; //82->75
    params->dsi.horizontal_active_pixel			= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 229;//zhaofeng 修改为了避开影响射频干扰 //this value must be in MTK suggested table

    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;
}

/*to prevent electric leakage*/
static void lcm_id_pin_handle(void)
{
/*
    mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
    mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
*/
}
static unsigned int lcm_compare_id(void)
{
#if 1
	unsigned int  data_array[16];
	unsigned char buffer_c5[3];
	unsigned char buffer_04[3];
        int id = 0;

        lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
        lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
        mdelay(50);
        //reset high to low to high
        lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
        //mt_set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
        mdelay(5);
        //mt_set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
        lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
        mdelay(20);//5 -> 50
        //mt_set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
        lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
        mdelay(200);//10 -> 200

	data_array[0]=0x00063902;
	data_array[1]=0x52AA55F0;
	data_array[2]=0x00000108;
	dsi_set_cmdq(data_array,3,1);

	data_array[0] = 0x00033700;
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0xC5, buffer_c5, 3);
        id = buffer_c5[0];

    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang----%s--%x,%x----\n",__func__,buffer_c5[0],buffer_c5[1]);
    #else
	  printk("[KERNEL]---caozhengguang----%s--%x,%x--\n",__func__,buffer_c5[0],buffer_c5[1]);
    #endif

        if ((buffer_c5[0]==0x55)&&(buffer_c5[1]==0x21)){
		return 1;
	}else{
		return 0;
	}
#endif
}


static void lcm_init(void)
{

	 //enable VSP & VSN
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
    mdelay(50);
	//reset high to low to high
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(5);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(10);

    //lcm_id_pin_handle();
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);  
    
    // when phone initial , config output high, enable backlight drv chip 
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);
    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang----%s------\n",__func__);
    #else
	  printk("[KERNEL]---caozhengguang----%s------\n",__func__);
    #endif
    LCD_DEBUG("uboot:boe_nt35521_lcm_init\n");
}

static void lcm_suspend(void)
{

    //Back to MP.P7 baseline , solve LCD display abnormal On the right
    // when phone sleep , config output low, disable backlight drv chip  
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ZERO);

    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

    //reset low
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    //disable VSP & VSN
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
    mdelay(5);	
    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang----%s------\n",__func__);
    #else
	  printk("[KERNEL]---caozhengguang----%s------\n",__func__);
    #endif
    LCD_DEBUG("kernel:boe_nt35521_lcm_suspend\n");
}

static void lcm_resume(void)
{

    //enable VSP & VSN
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
    mdelay(50);

    //reset low to high
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(5);   
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    mdelay(5); 
    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ONE);
    mdelay(10);	


    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

    //Back to MP.P7 baseline , solve LCD display abnormal On the right
    //when sleep out, config output high ,enable backlight drv chip  
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);
    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang----%s------\n",__func__);
    #else
	  printk("[KERNEL]---caozhengguang----%s------\n",__func__);
    #endif
    LCD_DEBUG("kernel:boe_nt35521_lcm_resume\n");
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang----%s------\n",__func__);
    #else
	  printk("[KERNEL]---caozhengguang----%s------\n",__func__);
    #endif

}
#endif





static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[3];
	int   array[4];

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x0A, buffer, 1);
	if(buffer[0]==0x9C)
	{
		return FALSE;
	}
	else
	{			 
		return TRUE;
	}
 #endif
    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang----%s------\n",__func__);
    #else
	  printk("[KERNEL]---caozhengguang----%s------\n",__func__);
    #endif

}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	lcm_resume();
    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang----%s------\n",__func__);
    #else
	  printk("[KERNEL]---caozhengguang----%s------\n",__func__);
    #endif
	return TRUE;
}




LCM_DRIVER nt35521_hd720_dsi_video_boe_lcm_drv =
{
    .name           	= "nt35521_hd720_dsi_video_boe",
    .set_util_funcs 	= lcm_set_util_funcs,
    .get_params     	= lcm_get_params,
    .init           	= lcm_init,
    .suspend        	= lcm_suspend,
    .resume         	= lcm_resume,
    .compare_id     	= lcm_compare_id,
    
    .esd_check = lcm_esd_check,
    .esd_recover = lcm_esd_recover,
    #if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
    #endif
};

