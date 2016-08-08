/* Copyright Statement:
 * 
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2008
 *
 *  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
 *  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 *  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 *  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 *  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
 *  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
 *  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 *  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 *  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 *  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
 *  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
 *  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
 *  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
 *  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
 *  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
 *
 *****************************************************************************/
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
//	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_OTM1282 (0x1282)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY                                                                   0XFE
#define REGFLAG_END_OF_TABLE                                                            0x100   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)                lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)           lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                                                          lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                                      lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                                                                   lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE                                                      0

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
    { 0x00, 0x01, {0x00 }},
    { 0xff, 0x03, {0x12, 0x82, 0x01 }},
    { 0x00, 0x01, {0x80 }},
    { 0xff, 0x02, {0x12, 0x82 }},
    { 0x00, 0x01, {0x92 }},
    { 0xff, 0x02, {0x20, 0x02 }},
    { 0x00, 0x01, {0xb4 }},
    { 0xc0, 0x01, {0x40 }},
    { 0x00, 0x01, {0x91 }},
    { 0xb3, 0x02, {0x08, 0x10 }},
    { 0x00, 0x01, {0xb3 }},
    { 0xc0, 0x01, {0x33 }},
    { 0x00, 0x01, {0x00 }},
    { 0x1c, 0x01, {0x32 }},
    { 0x00, 0x01, {0x84 }},
    { 0xa4, 0x01, {0x00 }},
    { 0x00, 0x01, {0x80 }},
    { 0xc0, 0x0e, {0x00, 0x85, 0x00, 0x2c, 0x2c, 0x00, 0x7b, 0x2c, 0x2c, 0x00, 0x7b, 0x00, 0x2c, 0x2c }},
    { 0x00, 0x01, {0xa0 }},
    { 0xc0, 0x07, {0x00, 0x00, 0x00, 0x07, 0x00, 0x19, 0x09 }},
    { 0x00, 0x01, {0xd0 }},
    { 0xc0, 0x07, {0x00, 0x00, 0x00, 0x07, 0x00, 0x19, 0x09 }},
    { 0x00, 0x01, {0x80 }},
    { 0xc1, 0x02, {0x55, 0x55 }},
    { 0x00, 0x01, {0x90 }},
    { 0xc1, 0x03, {0x66, 0x00, 0x00 }},
    { 0x00, 0x01, {0x80 }},
    { 0xc2, 0x04, {0x83, 0x01, 0x45, 0x45 }},
    { 0x00, 0x01, {0x90 }},
    { 0xc2, 0x0f, {0xa9, 0x2c, 0x01, 0x00, 0x00, 0xab, 0x2c, 0x01, 0x00, 0x00, 0xaa, 0x2c, 0x01, 0x00, 0x00 }},
    { 0x00, 0x01, {0xa0 }},
    { 0xc2, 0x05, {0xa8, 0x2c, 0x01, 0x00, 0x00 }},
    { 0x00, 0x01, {0xec }},
    { 0xc2, 0x01, {0x00 }},
    { 0x00, 0x01, {0xfa }},
    { 0xc2, 0x03, {0x00, 0x80, 0x01 }},
    { 0x00, 0x01, {0x80 }},
    { 0xc3, 0x04, {0x83, 0x01, 0x45, 0x45 }},
    { 0x00, 0x01, {0x90 }},
    { 0xc3, 0x0f, {0xa9, 0x2c, 0x01, 0x00, 0x00, 0xab, 0x2c, 0x01, 0x00, 0x00, 0xaa, 0x2c, 0x01, 0x00, 0x00 }},
    { 0x00, 0x01, {0xa0 }},
    { 0xc3, 0x05, {0xa8, 0x2c, 0x01, 0x00, 0x00 }},
    { 0x00, 0x01, {0xec }},
    { 0xc3, 0x01, {0x00 }},
    { 0x00, 0x01, {0x90 }},
    { 0xcb, 0x0f, {0xc0, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00 }},
    { 0x00, 0x01, {0xa0 }},
    { 0xcb, 0x0f, {0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }},
    { 0x00, 0x01, {0xb0 }},
    { 0xcb, 0x0f, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x50, 0xc0, 0x00, 0x00, 0x00, 0x00 }},
    { 0x00, 0x01, {0xc0 }},
    { 0xcb, 0x0f, {0xff, 0x28, 0xeb, 0x28, 0xeb, 0x14, 0x14, 0x14, 0x14, 0x00, 0x14, 0x14, 0xd7, 0x28, 0x28 }},
    { 0x00, 0x01, {0xd0 }},
    { 0xcb, 0x0f, {0x28, 0x28, 0x14, 0x14, 0x14, 0xd7, 0x04, 0xf7, 0x04, 0x14, 0x14, 0x14, 0x14, 0x04, 0x04 }},
    { 0x00, 0x01, {0xe0 }},
    { 0xcb, 0x0f, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x15, 0x07, 0x14, 0x14, 0x00, 0x00 }},
    { 0x00, 0x01, {0xf0 }},
    { 0xcb, 0x0c, {0x30, 0x03, 0xfc, 0x03, 0xf0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }},
    { 0x00, 0x01, {0x80 }},
    { 0xcc, 0x0a, {0x0f, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x10 }},
    { 0x00, 0x01, {0xb0 }},
    { 0xcc, 0x0a, {0x0f, 0x06, 0x05, 0x04, 0x03, 0x07, 0x08, 0x09, 0x0a, 0x10 }},
    { 0x00, 0x01, {0x8a }},
    { 0xcd, 0x01, {0x0b }},
    { 0x00, 0x01, {0xa0 }},
    { 0xcd, 0x0f, {0x12, 0x14, 0x15, 0x04, 0x05, 0x01, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x26, 0x25, 0x24 }},
    { 0x00, 0x01, {0xb0 }},
    { 0xcd, 0x0f, {0x23, 0x22, 0x21, 0x20, 0x1f, 0x2d, 0x2d, 0x2d, 0x2d, 0x13, 0x0b, 0x0c, 0x2d, 0x2d, 0x2d }},
    { 0x00, 0x01, {0xc0 }},
    { 0xcd, 0x0a, {0x2d, 0x2d, 0x2d, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x1d, 0x2d }},
    { 0x00, 0x01, {0xd0 }},
    { 0xcd, 0x0f, {0x12, 0x14, 0x15, 0x02, 0x03, 0x01, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x26, 0x25, 0x24 }},
    { 0x00, 0x01, {0xe0 }},
    { 0xcd, 0x0f, {0x23, 0x22, 0x21, 0x20, 0x1f, 0x2d, 0x2d, 0x2d, 0x2d, 0x13, 0x0b, 0x0c, 0x2d, 0x2d, 0x2d }},
    { 0x00, 0x01, {0xf0 }},
    { 0xcd, 0x0a, {0x2d, 0x2d, 0x2d, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x1d, 0x2d }},
    { 0x00, 0x01, {0x00 }},
    { 0xd9, 0x01, {0x72 }},
    { 0x00, 0x01, {0x00 }},
    { 0xd8, 0x02, {0x34, 0x34 }},
    { 0x00, 0x01, {0x90 }},
    { 0xc5, 0x04, {0x92, 0xd6, 0xad, 0xb0 }},
    { 0x00, 0x01, {0xa0 }},
    { 0xc5, 0x04, {0x92, 0xd6, 0xad, 0xb0 }},
    { 0x00, 0x01, {0x00 }},
    { 0xe1, 0x18, {0x1b, 0x21, 0x2f, 0x3c, 0x46, 0x4e, 0x5b, 0x6d, 0x79, 0x89, 0x94, 0x9b, 0x60, 0x5d, 0x5a, 0x50, 0x42, 0x34, 0x2a, 0x24, 0x1f, 0x19, 0x18, 0x17 }},
    { 0x00, 0x01, {0x00 }},
    { 0xe2, 0x18, {0x1b, 0x21, 0x2f, 0x3c, 0x46, 0x4e, 0x5b, 0x6d, 0x79, 0x89, 0x94, 0x9b, 0x60, 0x5d, 0x5a, 0x50, 0x42, 0x34, 0x2a, 0x24, 0x1f, 0x19, 0x18, 0x17 }},
    { 0x00, 0x01, {0x00 }},
    { 0xe3, 0x18, {0x1b, 0x21, 0x2f, 0x3c, 0x46, 0x4e, 0x5b, 0x6d, 0x79, 0x89, 0x94, 0x9b, 0x60, 0x5d, 0x5a, 0x50, 0x42, 0x34, 0x2a, 0x24, 0x1f, 0x19, 0x18, 0x17 }},
    { 0x00, 0x01, {0x00 }},
    { 0xe4, 0x18, {0x1b, 0x21, 0x2f, 0x3c, 0x46, 0x4e, 0x5b, 0x6d, 0x79, 0x89, 0x94, 0x9b, 0x60, 0x5d, 0x5a, 0x50, 0x42, 0x34, 0x2a, 0x24, 0x1f, 0x19, 0x18, 0x17 }},
    { 0x00, 0x01, {0x00 }},
    { 0xe5, 0x18, {0x1b, 0x21, 0x2f, 0x3c, 0x46, 0x4e, 0x5b, 0x6d, 0x79, 0x89, 0x94, 0x9b, 0x60, 0x5d, 0x5a, 0x50, 0x42, 0x34, 0x2a, 0x24, 0x1f, 0x19, 0x18, 0x17 }},
    { 0x00, 0x01, {0x00 }},
    { 0xe6, 0x18, {0x1b, 0x21, 0x2f, 0x3c, 0x46, 0x4e, 0x5b, 0x6d, 0x79, 0x89, 0x94, 0x9b, 0x60, 0x5d, 0x5a, 0x50, 0x42, 0x34, 0x2a, 0x24, 0x1f, 0x19, 0x18, 0x17 }},
    { 0x00, 0x01, {0x93 }},
    { 0xf5, 0x01, {0x10 }},
    { 0x00, 0x01, {0x97 }},
    { 0xc5, 0x02, {0x33, 0x30 }},
    { 0x00, 0x01, {0xa7 }},
    { 0xc5, 0x02, {0x33, 0x30 }},
    { 0x00, 0x01, {0x83 }},
    { 0xc4, 0x01, {0x02 }},
    { 0x00, 0x01, {0x80 }},
    { 0xa5, 0x01, {0x0c }},
    { 0x00, 0x01, {0xb3 }},
    { 0xc0, 0x01, {0x33 }},
    { 0x00, 0x01, {0x80 }},
    { 0xc4, 0x02, {0x04, 0x0f }},
    { 0x00, 0x01, {0xa0 }},
    { 0xc1, 0x02, {0x02, 0xe0 }},
    { 0x00, 0x01, {0x00 }},
    { 0xff, 0x03, {0xff, 0xff, 0xff }},
    { 0x11, 0x01, {0x00 }},
    { REGFLAG_DELAY, 0xa0, {}},
    { 0x29, 0x01, {0x00 }},
    { REGFLAG_DELAY, 0x3c, {}},
    { REGFLAG_END_OF_TABLE, 0x00, {}},
};

static struct LCM_setting_table lcm_sleep_mode_in_setting[] = {
    { 0x28, 0x01, {0x00 }},
    { REGFLAG_DELAY, 0x14, {}},
    { 0x10, 0x01, {0x00 }},
    { REGFLAG_DELAY, 0x78, {}},
    { REGFLAG_END_OF_TABLE, 0x00, {}},
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;
    unsigned cmd;
    
    for(i = 0; i < count; i++) {
        
        cmd = table[i].cmd;
        
        switch (cmd) {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
                
            case REGFLAG_END_OF_TABLE :
                break;
                
            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
                MDELAY(2);
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
    
    params->dsi.LANE_NUM = 3;
    params->dsi.packet_size = 256;
    params->dsi.vertical_sync_active = 4;
    params->dsi.vertical_backporch = 16;
    params->dsi.horizontal_sync_active = 6;
    params->dsi.horizontal_blanking_pixel = 60;
    params->dsi.lcm_esd_check_table[0].cmd = 10;
    params->dsi.lcm_esd_check_table[0].para_list[0] = -100;
    params->dsi.PLL_CLOCK = 230;
    params->type = 2;
    params->dsi.mode = 2;
    params->dsi.data_format.format = 2;
    params->dsi.intermediat_buffer_num = 2;
    params->dsi.PS = 2;
    params->width = 720;
    params->dsi.horizontal_active_pixel = 720;
    params->height = 1280;
    params->dsi.vertical_active_line = 1280;
    params->dbi.te_mode = 0;
    params->dsi.data_format.color_order = 0;
    params->dsi.data_format.trans_seq = 0;
    params->dsi.data_format.padding = 0;
    params->dsi.vertical_frontporch = 20;
    params->dsi.horizontal_backporch = 20;
    params->dsi.horizontal_frontporch = 20;
    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;
    params->dsi.lcm_esd_check_table[0].count = 1;
    params->dsi.ssc_disable = 1;
    params->dsi.cont_clock = 1;
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);
    
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    #ifndef BUILD_LK
    SET_RESET_PIN(1u);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1u);
    MDELAY(20);
    push_table(lcm_sleep_mode_in_setting, sizeof(lcm_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    #endif
}


static void lcm_resume(void)
{
    #ifndef BUILD_LK
    lcm_init();
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
    
    data_array[0]= 0x00290508; //HW bug, so need send one HS packet
    dsi_set_cmdq(data_array, 1, 1);
    
    data_array[0]= 0x002c3909;
    dsi_set_cmdq(data_array, 1, 0);
    
}
#endif

static unsigned int lcm_compare_id(void)
{
    unsigned int id0,id1,id=0;
    unsigned char buffer[5];
    unsigned int array[16];
    
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    
    SET_RESET_PIN(1);
    MDELAY(20);
    
    array[0] = 0x00053700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    
    read_reg_v2(0xA1, buffer, 5);   //018B1283ff
    id0 = buffer[2];
    id1 = buffer[3];
    id=(id0<<8)|id1;
    
    #ifdef BUILD_LK
    printf("%s, LK otm1282a debug: otm1282a id = 0x%08x\n", __func__, id);
    #else
    printk("%s, kernel otm1282a horse debug: otm1282a id = 0x%08x\n", __func__, id);
    #endif
    
    if(id == LCM_ID_OTM1282)
        return 1;
    else
        return 0;
}

// static unsigned int lcm_esd_check(void)
// {
//     #ifndef BUILD_LK
//     char  buffer[3];
//     int   array[4];
//     
//     if(lcm_esd_test)
//     {
//         lcm_esd_test = FALSE;
//         return TRUE;
//     }
//     
//     //hct_set_hs_read();
//     array[0] = 0x00013708;
//     dsi_set_cmdq(array, 1, 1);
//     read_reg_v2(0x0a, buffer, 1);
//     
//     if(buffer[0]==0x9c)
//     {
//         return FALSE;
//     }
//     else
//     {
//         return TRUE;
//     }
//     #endif
//     
// }

// static unsigned int lcm_esd_recover(void)
// {
//     lcm_init();
//     lcm_resume();
//     
//     return TRUE;
// }

LCM_DRIVER hct_otm1282a_dsi_vdo_hd_auo =
{
    .name		= "hct_otm1282a_dsi_vdo_hd_auo",
    .set_util_funcs	= lcm_set_util_funcs,
    .get_params	= lcm_get_params,
    .init		= lcm_init,
    .suspend	= lcm_suspend,
    .resume		= lcm_resume,
    .compare_id	= lcm_compare_id,
    #if (LCM_DSI_CMD_MODE)
    //.set_backlight= lcm_setbacklight,
    .update         = lcm_update,
    #endif
    //.esd_check = lcm_esd_check,
    //.esd_recover=lcm_esd_recover,
};
