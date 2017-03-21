#ifndef BUILD_LK
#include <linux/string.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif



// ---------------------------------------------------------------------------
// Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH           (720)
#define FRAME_HEIGHT          (1280)

#define REGFLAG_DELAY          0XFD
#define REGFLAG_END_OF_TABLE   0xFE // END OF REGISTERS MARKER


#define GPIO_LCD_BIAS_ENP_PIN          GPIO12
#define GPIO_LCD_BIAS_ENN_PIN          GPIO105
#define GPIO_DISP_LRSTB_PIN            GPIO112
#define GPIO_LCD_DRV_EN_PIN            GPIO90



#define LCM_DSI_CMD_MODE       0   
#define LCM_ID                 0x8394


#define LCD_MODUL_ID           (0x02)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE; ///only for ESD test



// ---------------------------------------------------------------------------
// Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
// Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size) 




static void init_lcm_registers(void)
{
	unsigned int data_array[16];
	data_array[0] = 0x00043902;
	data_array[1] = 0x9483FFB9;//B9
	dsi_set_cmdq(&data_array, 2, 1);
	

	data_array[0] = 0x00033902;
	data_array[1] = 0x008333BA;//BA
	dsi_set_cmdq(&data_array, 2, 1);


	data_array[0] = 0x00103902;
	data_array[1] = 0x12126CB1;//B1
	data_array[2] = 0xF1110434;
	data_array[3] = 0x2393E480;
	data_array[4] = 0x58D2C080;
	dsi_set_cmdq(&data_array, 5, 1);
	
	data_array[0] = 0x000C3902;
	data_array[1] = 0x106400B2;//B2
	data_array[2] = 0x081C3207;
	data_array[3] = 0x004D1C08;
	dsi_set_cmdq(&data_array, 4, 1);


	data_array[0] = 0x00023902;
	data_array[1] = 0x000007BC;//C0
	dsi_set_cmdq(&data_array, 2, 1);


	data_array[0] = 0x00043902;
	data_array[1] = 0x010e41bf;//B9
	dsi_set_cmdq(&data_array, 2, 1);
		

	
	data_array[0] = 0x000d3902;
	data_array[1] = 0x40FF00B4;//B4
	data_array[2] = 0x435A405A;
	data_array[3] = 0x016A015a;
    data_array[4] = 0x0000006a;
	dsi_set_cmdq(&data_array, 5, 1);
	
	
	data_array[0] = 0x001f3902;
	data_array[1] = 0x000600D3;//D3
	data_array[2] = 0x00000740;
	data_array[3] = 0x00081032;
	data_array[4] = 0x0F155208;
	data_array[5] = 0x10320f05;
	data_array[6] = 0x47000000;
	data_array[7] = 0x470c0c44;
    data_array[8] = 0x00470c0c;
	dsi_set_cmdq(&data_array, 9, 1);

	data_array[0] = 0x002D3902;
	data_array[1] = 0x002120D5;//D5
	data_array[2] = 0x04030201;
	data_array[3] = 0x18070605;
	data_array[4] = 0x18181818;
	data_array[5] = 0x18181818;
	data_array[6] = 0x18181818;
	data_array[7] = 0x18181818;
	data_array[8] = 0x18181818;
	data_array[9] = 0x18181818;
	data_array[10] = 0x19181818;
	data_array[11] = 0x24181819;
	data_array[12] = 0x00000025;
	dsi_set_cmdq(&data_array, 13, 1);
	
	data_array[0] = 0x002D3902;
	data_array[1] = 0x072524D6;//D6
	data_array[2] = 0x03040506;
	data_array[3] = 0x18000102;
	data_array[4] = 0x18181818;
	data_array[5] = 0x58181818;
	data_array[6] = 0x18181858;
	data_array[7] = 0x18181818;
	data_array[8] = 0x18181818;
	data_array[9] = 0x18181818;
	data_array[10] = 0x18181818;
	data_array[11] = 0x20191918;
	data_array[12] = 0x00000021;
	dsi_set_cmdq(&data_array, 13, 1);
	
	data_array[0] = 0x00033902;
	data_array[1] = 0x006666B6;//B6 VCOM fliker
	dsi_set_cmdq(&data_array, 2, 1);
	
	
	data_array[0] = 0x002B3902;
	data_array[1] = 0x020000E0;//E0
	data_array[2] = 0x0E3F3C36;
	data_array[3] = 0x0A08053C;
	data_array[4] = 0x13110D16;
	data_array[5] = 0x14071312;
	data_array[6] = 0x00001A19;
	data_array[7] = 0x3F3C3602;
	data_array[8] = 0x08053C0E;
	data_array[9] = 0x110D160A;
	data_array[10] = 0x07131213;
	data_array[11] = 0x001A1914;
	dsi_set_cmdq(&data_array, 12, 1);
	
	data_array[0] = 0x00033902;
	data_array[1] = 0x001430C0;//C0
	dsi_set_cmdq(&data_array, 2, 1);
	
	
	data_array[0] = 0x00053902;
	data_array[1] = 0x40C000C7;//C7
	data_array[2] = 0x000000c0;
	dsi_set_cmdq(&data_array, 3, 1);
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x000009CC;//C0
	dsi_set_cmdq(&data_array, 2, 1);
	
	//data_array[0] = 0x00043902;
	//data_array[1] = 0x2E2E4FC9;//C9
	//dsi_set_cmdq(&data_array, 2, 1);
	
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x0000ff51;//C0
	dsi_set_cmdq(&data_array, 2, 1);
	
	data_array[0] = 0x00023902;
	data_array[1] = 0x00002453;//C0
	dsi_set_cmdq(&data_array, 2, 1);

	
	data_array[0] = 0x00023902;
	data_array[1] = 0x00000155;//C0
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(20);

	
	
	
	
	data_array[0] = 0x00110500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(200);
	data_array[0] = 0x00290500;
	dsi_set_cmdq(&data_array, 1, 1);
        MDELAY(10);
}




// ---------------------------------------------------------------------------
// LCM Driver Implementations
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
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
        #endif


#ifdef SLT_DEVINFO_LCM
		params->module="YT55F59F0";
		params->vendor="YASSY";
		params->ic="HX8394D";
		params->info="720*1280";
#endif
	
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
	        params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	        params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	        params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active				= 4;// 3    2
		params->dsi.vertical_backporch					= 12;// 20   1
		params->dsi.vertical_frontporch					= 15; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 12;// 50  2
		params->dsi.horizontal_backporch				= 90;
		params->dsi.horizontal_frontporch				= 90;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8; 

		// Bit rate calculation
		params->dsi.PLL_CLOCK = 229;

                params->dsi.compatibility_for_nvk = 1;
                params->dsi.ssc_disable = 1;
		
}



static unsigned int lcm_compare_id(void)
{
	unsigned int id,id1=0;
	unsigned char buffer[2];
	unsigned int array[16];  

       lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
       lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
       MDELAY(50);

	SET_RESET_PIN(0);
	MDELAY(20); 
	SET_RESET_PIN(1);
	MDELAY(20); 

	array[0]=0x00043902;
	array[1]=0x8983FFB9;// page enable
	dsi_set_cmdq(array, 2, 1);
	//MDELAY(10);
//{0x39,0xBA,7,{0x41,0x93,0x00,0x16,0xA4,0x10,0x18}},	
	array[0]=0x00033902;
	array[1]=0x008333BA;// page enable
	dsi_set_cmdq(array, 2, 1);


	array[0] = 0x00043700;// return byte number
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);



        read_reg_v2(0x04, buffer, 3);
	id  =  (buffer[0] << 8) | buffer[1]; 
	
#ifdef BUILD_LK
	printf("caozhengguang yassy   %s, id = 0x%08x  \n", __func__, id);
#else
	printk("caozhengguang yassy   %s, id = 0x%08x  \n", __func__, id);
#endif


    return (LCM_ID == id)?1:0;

}


static void lcm_init(void)
{   
       lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
       lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
       MDELAY(50);

		SET_RESET_PIN(0);
		MDELAY(20); 
		SET_RESET_PIN(1);
		MDELAY(120); 


		init_lcm_registers();
    lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);
    #ifdef BUILD_LK
	  printf("[LK]---caozhengguang--yassy--%s-------\n",__func__);
    #else
	  printk("[KERNEL]---caozhengguang--yassy--%s-------\n",__func__);
    #endif
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

        lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ZERO);


	
	SET_RESET_PIN(1);	
	SET_RESET_PIN(0);
	MDELAY(20); // 1ms
	
	SET_RESET_PIN(1);
	MDELAY(120);   

    lcm_util.set_gpio_out(GPIO_DISP_LRSTB_PIN, GPIO_OUT_ZERO);
    MDELAY(5);
    //disable VSP & VSN
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
    lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
    MDELAY(5);	  

}

static void lcm_resume(void)
{


     lcm_init();

    #ifdef BUILD_LK
	  printf("[LK]--caozhengguang-resume--yassy--%s------\n",__func__);
    #else
	  printk("[KERNEL]--caozhengguang--resume--yassy--%s------\n",__func__);
    #endif	
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

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif




static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[3];
	unsigned int data_array[16]; 

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

        data_array[0] = 0x00043901;
	data_array[1] = 0x9483FFB9;//B9
	dsi_set_cmdq(&data_array, 2, 1);
	

	data_array[0] = 0x00043901;
	data_array[1] = 0x008333BA;//BA
	dsi_set_cmdq(&data_array, 2, 1);

	read_reg_v2(0x0a, buffer, 1);
	if(buffer[0]==0x1c)
	{
		return FALSE;
	}
	else
	{			 
		return TRUE;
	}
 #endif
}

static unsigned int lcm_esd_recover(void)
{
 	lcm_init();
	
}


LCM_DRIVER hx8394d_hd720_dsi_vdo_yassy_lcm_drv = 
{
    .name			= "hx8394d_hd720_dsi_vdo_yassy",
    .set_util_funcs = lcm_set_util_funcs,
    .compare_id = lcm_compare_id,
    .get_params = lcm_get_params,
    .init = lcm_init,
    .suspend = lcm_suspend,
    .resume = lcm_resume,
    //.esd_check = lcm_esd_check,
    //.esd_recover = lcm_esd_recover,
    #if (LCM_DSI_CMD_MODE)
    //.set_backlight = lcm_setbacklight,
    .update = lcm_update,
    #endif
};

