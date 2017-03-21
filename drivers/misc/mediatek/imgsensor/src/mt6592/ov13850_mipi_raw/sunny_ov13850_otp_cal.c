#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <asm/system.h>

#include <linux/proc_fs.h> 


#include <linux/dma-mapping.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov13850mipiraw_Sensor.h"
#include "sunny_ov13850_otp.h"

extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);

#define OV13850_write_i2c(addr, para) iWriteReg((u16) addr , (u32) para , 1, OV13850MIPI_WRITE_ID)


#define Delay(ms)  mdelay(ms)


static kal_uint16 OV13850_read_i2c(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,OV13850MIPI_WRITE_ID);
    return get_byte;
}
/*
struct otp_struct {
int rg_ratio;
int bg_ratio;
//

int lenc[62];
};
*/
// index: index of otp group. (1, 2, 3)
// return: 0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
int check_otp_wb(int index)
{
	int flag;
	//set 0x5002[1] to ¡°0¡±
	int temp1;
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x00 & 0x02) | (temp1 & (~0x02)));
	OV13850_write_i2c(0x3d84, 0xC0);
	//partial mode OTP write start address
	OV13850_write_i2c(0x3d88, 0x72);
	OV13850_write_i2c(0x3d89, 0x4e);
	// partial mode OTP write end address
	OV13850_write_i2c(0x3d8A, 0x72);
	OV13850_write_i2c(0x3d8B, 0x4e);
	// read otp into buffer
	OV13850_write_i2c(0x3d81, 0x01);
	Delay(5);
	//select group
	flag = OV13850_read_i2c(0x724e);
	if (index == 1)
	{
		flag = (flag>>6) & 0x03;
	}
	else if (index == 2)
	{
		flag = (flag>>4) & 0x03;
	}
	else if (index == 3)
	{
		flag = (flag>>2) & 0x03;
	}
	// clear otp buffer
	OV13850_write_i2c(0x724e, 0x00);
	//set 0x5002[1] to ¡°1¡±
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x02 & 0x02) | (temp1 & (~0x02)));
	if (flag == 0x00) {
		return 0;
	}
	else if (flag & 0x02) {
		return 1;
	}
	else {
		return 2;
	}
}
// index: index of otp group. (1, 2, 3)
// return: 0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
int check_otp_lenc(int index)
{
	int flag;
	//set 0x5002[1] to ¡°0¡±
	int temp1;
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x00 & 0x02) | (temp1 & (~0x02)));
	OV13850_write_i2c(0x3d84, 0xC0);
	//partial mode OTP write start address
	OV13850_write_i2c(0x3d88, 0x72);
	OV13850_write_i2c(0x3d89, 0xa7);
	// partial mode OTP write end address
	OV13850_write_i2c(0x3d8A, 0x72);
	OV13850_write_i2c(0x3d8B, 0xa7);
	// read otp into buffer
	OV13850_write_i2c(0x3d81, 0x01);
	Delay(5);
	flag = OV13850_read_i2c(0x72a7);
	if (index == 1)
	{
		flag = (flag>>6) & 0x03;
	}
	else if (index == 2)
	{
		flag = (flag>>4) & 0x03;
	}
	else if (index == 3)
	{
		flag = (flag>> 2)& 0x03;
	}
	// clear otp buffer
	OV13850_write_i2c(0x72a7, 0x00);
	//set 0x5002[1] to ¡°1¡±
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x02 & 0x02) | (temp1 & (~0x02)));
	if (flag == 0x00) {
		return 0;
	}
	else if (flag & 0x02) {
		return 1;
	}
	else {
		return 2;
	}
}
// index: index of otp group. (1, 2, 3)
// otp_ptr: pointer of otp_struct
// return: 0,
int read_otp_wb(int index, struct otp_struct *otp_ptr)
{
	int i;
	int temp;
	int start_addr, end_addr;
int rg_typical,bg_typical;//typical_rg_ratio,typical_bg_ratio;
	if (index == 1) {
		start_addr = 0x724f;
		end_addr = 0x7263;
	}
	else if (index == 2) {
		start_addr = 0x7264;
		end_addr = 0x7278;
	}
	else if (index == 3) {
		start_addr = 0x7279;
		end_addr = 0x728d;
	}
	//set 0x5002[1] to ¡°0¡±
	int temp1;
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x00 & 0x02) | (temp1 & (~0x02)));
	OV13850_write_i2c(0x3d84, 0xC0);
	//partial mode OTP write start address
	OV13850_write_i2c(0x3d88, (start_addr >> 8) & 0xff);
	OV13850_write_i2c(0x3d89, start_addr & 0xff);
	// partial mode OTP write end address
	OV13850_write_i2c(0x3d8A, (end_addr >> 8) & 0xff);
	OV13850_write_i2c(0x3d8B, end_addr & 0xff);
	// read otp into buffer
	OV13850_write_i2c(0x3d81, 0x01);
	Delay(5);
	temp = OV13850_read_i2c(start_addr + 1);
	(*otp_ptr).rg_ratio = (OV13850_read_i2c(start_addr)<<2) + ((temp>>6) & 0x03);
	temp = OV13850_read_i2c(start_addr + 3);
	(*otp_ptr).bg_ratio = (OV13850_read_i2c(start_addr + 2)<<2) + ((temp>>6) & 0x03);
	//int typical_rg_ratio;
	temp = OV13850_read_i2c(start_addr + 7);
	(*otp_ptr).typical_rg_ratio = (OV13850_read_i2c(start_addr+6)<<2) + ((temp>>6) & 0x03);
	//int typical_bg_ratio;
	temp = OV13850_read_i2c(start_addr + 9);
	(*otp_ptr).typical_bg_ratio = (OV13850_read_i2c(start_addr+8)<<2) + ((temp>>6) & 0x03);

	
	rg_typical=(*otp_ptr).typical_rg_ratio;
    bg_typical=(*otp_ptr).typical_bg_ratio;       
	OV13850DB("%s,john rg_typical=%x\n",__func__,rg_typical);
    OV13850DB("%s,john bg_typical=%x\n",__func__,bg_typical);

	// clear otp buffer
	for (i=start_addr; i<=end_addr; i++) {
		OV13850_write_i2c(i, 0x00);
	}
	//set 0x5002[1] to ¡°1¡±
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x02 & 0x02) | (temp1 & (~0x02)));
	return 0;
}
// index: index of otp group. (1, 2,...,OTP_DRV_LSC_GROUP_COUNT)
// otp_ptr: pointer of otp_struct
// return: 0,
int read_otp_lenc(int index, struct otp_struct *otp_ptr)
{
	int i;
	int start_addr, end_addr;
	if (index == 1) {
		start_addr = 0x72a8;
		end_addr = 0x72e6;
	}
	else if (index == 2) {
		start_addr = 0x72e7;
		end_addr = 0x7325;
	}
	else if (index == 3) {
		start_addr = 0x7326;
		end_addr = 0x7364;
	}
	//set 0x5002[1] to ¡°0¡±
	int temp1;
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x00 & 0x02) | (temp1 & (~0x02)));
	OV13850_write_i2c(0x3d84, 0xC0);
	//partial mode OTP write start address
	OV13850_write_i2c(0x3d88, (start_addr >> 8) & 0xff);
	OV13850_write_i2c(0x3d89, start_addr & 0xff);
	// partial mode OTP write end address
	OV13850_write_i2c(0x3d8A, (end_addr >> 8) & 0xff);
	OV13850_write_i2c(0x3d8B, end_addr & 0xff);
	// read otp into buffer
	OV13850_write_i2c(0x3d81, 0x01);
	Delay(10);
	for(i=0; i<62; i++) {
		(* otp_ptr).lenc[i] = OV13850_read_i2c((start_addr + i));
	}
	// clear otp buffer
	for (i=start_addr; i<=end_addr; i++) {
		OV13850_write_i2c(i, 0x00);
	}
	//set 0x5002[1] to ¡°1¡±
	temp1 = OV13850_read_i2c(0x5002);
	OV13850_write_i2c(0x5002, (0x02 & 0x02) | (temp1 & (~0x02)));
	return 0;
}
// R_gain, sensor red gain of AWB, 0x400 =1
// G_gain, sensor green gain of AWB, 0x400 =1
// B_gain, sensor blue gain of AWB, 0x400 =1
// return 0;
int update_awb_gain(int R_gain, int G_gain, int B_gain)
{
	if (R_gain>0x400) {
		OV13850_write_i2c(0x5056, R_gain>>8);
		OV13850_write_i2c(0x5057, R_gain & 0x00ff);
	}
	if (G_gain>0x400) {
		OV13850_write_i2c(0x5058, G_gain>>8);
		OV13850_write_i2c(0x5059, G_gain & 0x00ff);
	}
	if (B_gain>0x400) {
		OV13850_write_i2c(0x505A, B_gain>>8);
		OV13850_write_i2c(0x505B, B_gain & 0x00ff);
	}
	return 0;
}
	// otp_ptr: pointer of otp_struct
int update_lenc(struct otp_struct * otp_ptr)
{
	int i, temp;
	temp = OV13850_read_i2c(0x5000);
	temp = 0x01 | temp;
	OV13850_write_i2c(0x5000, temp);
	//for(i=0;i<3 ;i++) {
	for(i=0;i<62 ;i++)
	{
		OV13850_write_i2c((0x5200 + i), (*otp_ptr).lenc[i]);
	}
	return 0;
}
// call this function after OV13850 initialization
// return value: 0 update success
// 1, no OTP
int update_otp_wb()
{
	struct otp_struct current_otp;
	int i;
	int otp_index;
	int temp;
	int rg,bg;
	// R/G and B/G of current camera module is read out from sensor OTP
	// check first OTP with valid data
	for(i=1;i<=3;i++) {
		temp = check_otp_wb(i);
		if (temp == 2) {
			otp_index = i;
			break;
		}
	}
	if (i>3) {
		// no valid wb OTP data
		return 1;
	}
	read_otp_wb(otp_index, &current_otp);
	if (current_otp.typical_rg_ratio==0 || current_otp.typical_bg_ratio==0)
	{
		OV13850DB(KERN_ERR,"typical info wrong\n");
	}
	/*
	if(current_otp.light_rg==0) {
		// no light source information in OTP, light factor = 1
		rg = current_otp.rg_ratio;
	}
	else {
		rg = current_otp.rg_ratio * (current_otp.light_rg +512) / 1024;
	}
	
	if(current_otp.light_bg==0) {
		// not light source information in OTP, light factor = 1
		bg = current_otp.bg_ratio;
	}
	else {
		bg = current_otp.bg_ratio * (current_otp.light_bg +512) / 1024;
	}
	*/
	
	rg = current_otp.rg_ratio;
	bg = current_otp.bg_ratio;

	OV13850DB("%s,before update ,RG = %x\n",__func__,rg);
	OV13850DB("%s,before update ,BG = %x\n",__func__,bg);

	//calculate G gain
	int nR_G_gain, nB_G_gain, nG_G_gain;
	int R_gain, G_gain, B_gain;

	int nBase_gain;
	nR_G_gain = (current_otp.typical_rg_ratio*1000) / rg;;//(RG_Ratio_Typical*1000) / rg;
	nB_G_gain = (current_otp.typical_bg_ratio*1000) / bg;;//(BG_Ratio_Typical*1000) / bg;
	nG_G_gain = 1000;
	if (nR_G_gain < 1000 || nB_G_gain < 1000)
	{
		if (nR_G_gain < nB_G_gain)
			nBase_gain = nR_G_gain;
		else
			nBase_gain = nB_G_gain;
	}
	else
	{
		nBase_gain = nG_G_gain;
	}
	
	R_gain = 0x400 * nR_G_gain / (nBase_gain);
	B_gain = 0x400 * nB_G_gain / (nBase_gain);
	G_gain = 0x400 * nG_G_gain / (nBase_gain);

	update_awb_gain(R_gain, G_gain, B_gain);

	return 0;
}
// call this function after OV13850 initialization
// return value: 0 update success
// 1, no OTP
int update_otp_lenc()
{
	struct otp_struct current_otp;
	int i;
	int otp_index;
	int temp;
	OV13850DB("john %s start\n ",__func__);
    
	// check first lens correction OTP with valid data
	for(i=1;i<=3;i++) {
		temp = check_otp_lenc(i);
		if (temp == 2) {
			otp_index = i;
			break;
		}
	}
	if (i>3) {
		// no valid WB OTP data
		return 1;
	}
	read_otp_lenc(otp_index, &current_otp);
	update_lenc(&current_otp);
    
    OV13850DB("john %s end\n ",__func__);
	// success
	return 0;
}

