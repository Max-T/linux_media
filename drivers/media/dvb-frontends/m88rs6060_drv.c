/*
 * Montage Technology M88rs6060 demodulator and tuner drivers
 * some code form m88ds3103
 * Copyright (c) 2021 Davin zhang <Davin@tbsdtv.com> www.Turbosight.com
 *
 * Copyright (C) 2020 Deep Thought <deeptho@gmail.com>: blindscan, spectrum, constellation code
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 only, as published by the Free Software Foundation.
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 */

#include <media/dvb_math.h>

#include "m88rs6060_priv.h"
#include "si5351_priv.h"

int verbose=0;
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "verbose debugging");

#define dprintk(fmt, arg...)																					\
	printk(KERN_DEBUG pr_fmt("%s:%d " fmt), __func__, __LINE__, ##arg)

#define vprintk(fmt, arg...)																					\
	if(verbose) printk(KERN_DEBUG pr_fmt("%s:%d " fmt),  __func__, __LINE__, ##arg)

static u16 mes_log10[] = {
	0, 3010, 4771, 6021, 6990, 7781, 8451, 9031, 9542, 10000,
	10414, 10792, 11139, 11461, 11761, 12041, 12304, 12553, 12788, 13010,
	13222, 13424, 13617, 13802, 13979, 14150, 14314, 14472, 14624, 14771,
	14914, 15052, 15185, 15315, 15441, 15563, 15682, 15798, 15911, 16021,
	16128, 16232, 16335, 16435, 16532, 16628, 16721, 16812, 16902, 16990,
	17076, 17160, 17243, 17324, 17404, 17482, 17559, 17634, 17709, 17782,
	17853, 17924, 17993, 18062, 18129, 18195, 18261, 18325, 18388, 18451,
	18513, 18573, 18633, 18692, 18751, 18808, 18865, 18921, 18976, 19031
};

static u16 mes_loge[] = {
	0, 6931, 10986, 13863, 16094, 17918, 19459, 20794, 21972, 23026,
	23979, 24849, 25649, 26391, 27081, 27726, 28332, 28904, 29444, 29957,
	30445, 30910, 31355, 31781, 32189, 32581, 32958, 33322, 33673, 34012,
	34340, 34657
};

static  struct MT_FE_PLS_INFO mPLSInfoTable[] =
 {
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0x00, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  TRUE, 		 0},
	 {0x01, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   TRUE, 		 0},
	 {0x02, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  TRUE, 		 1},
	 {0x03, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   TRUE, 		 1},
	 {0x04, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_4, 	  FALSE,  FALSE,		 0},
	 {0x05, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_4, 	  TRUE,   FALSE,		 0},
	 {0x06, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_4, 	  FALSE,  FALSE,		 1},
	 {0x07, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_4, 	  TRUE,   FALSE,		 1},
	 {0x08, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_3, 	  FALSE,  FALSE,		 0},
	 {0x09, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_3, 	  TRUE,   FALSE,		 0},
	 {0x0A, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_3, 	  FALSE,  FALSE,		 1},
	 {0x0B, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_3, 	  TRUE,   FALSE,		 1},
	 {0x0C, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_5, 	  FALSE,  FALSE,		 0},
	 {0x0D, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_5, 	  TRUE,   FALSE,		 0},
	 {0x0E, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_5, 	  FALSE,  FALSE,		 1},
	 {0x0F, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_5, 	  TRUE,   FALSE,		 1},
	 {0x10, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_2, 	  FALSE,  FALSE,		 0},
	 {0x11, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_2, 	  TRUE,   FALSE,		 0},
	 {0x12, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_2, 	  FALSE,  FALSE,		 1},
	 {0x13, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_1_2, 	  TRUE,   FALSE,		 1},
	 {0x14, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_5, 	  FALSE,  FALSE,		 0},
	 {0x15, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_5, 	  TRUE,   FALSE,		 0},
	 {0x16, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_5, 	  FALSE,  FALSE,		 1},
	 {0x17, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_5, 	  TRUE,   FALSE,		 1},
	 {0x18, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 0},
	 {0x19, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 0},
	 {0x1A, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 1},
	 {0x1B, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 1},
	 {0x1C, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 0},
	 {0x1D, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 0},
	 {0x1E, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 1},
	 {0x1F, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 1},
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0x20, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_4_5, 	  FALSE,  FALSE,		 0},
	 {0x21, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_4_5, 	  TRUE,   FALSE,		 0},
	 {0x22, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_4_5, 	  FALSE,  FALSE,		 1},
	 {0x23, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_4_5, 	  TRUE,   FALSE,		 1},
	 {0x24, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 0},
	 {0x25, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 0},
	 {0x26, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 1},
	 {0x27, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 1},
	 {0x28, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 0},
	 {0x29, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 0},
	 {0x2A, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 1},
	 {0x2B, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 1},
	 {0x2C, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 0},
	 {0x2D, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 0},
	 {0x2E, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 1},
	 {0x2F, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 1},
	 {0x30, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_5, 	  FALSE,  FALSE,		 0},
	 {0x31, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_5, 	  TRUE,   FALSE,		 0},
	 {0x32, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_5, 	  FALSE,  FALSE,		 1},
	 {0x33, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_5, 	  TRUE,   FALSE,		 1},
	 {0x34, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 0},
	 {0x35, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 0},
	 {0x36, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 1},
	 {0x37, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 1},
	 {0x38, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 0},
	 {0x39, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 0},
	 {0x3A, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 1},
	 {0x3B, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 1},
	 {0x3C, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 0},
	 {0x3D, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 0},
	 {0x3E, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 1},
	 {0x3F, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 1},
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0x40, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 0},
	 {0x41, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 0},
	 {0x42, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 1},
	 {0x43, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 1},
	 {0x44, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 0},
	 {0x45, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 0},
	 {0x46, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 1},
	 {0x47, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_8psk, 	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 1},
	 {0x48, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 0},
	 {0x49, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 0},
	 {0x4A, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 1},
	 {0x4B, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 1},
	 {0x4C, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 0},
	 {0x4D, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 0},
	 {0x4E, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 1},
	 {0x4F, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 1},
	 {0x50, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_4_5, 	  FALSE,  FALSE,		 0},
	 {0x51, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_4_5, 	  TRUE,   FALSE,		 0},
	 {0x52, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_4_5, 	  FALSE,  FALSE,		 1},
	 {0x53, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_4_5, 	  TRUE,   FALSE,		 1},
	 {0x54, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 0},
	 {0x55, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 0},
	 {0x56, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 1},
	 {0x57, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 1},
	 {0x58, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 0},
	 {0x59, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 0},
	 {0x5A, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 1},
	 {0x5B, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 1},
	 {0x5C, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 0},
	 {0x5D, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 0},
	 {0x5E, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 1},
	 {0x5F, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_16Apsk,	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 1},
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0x60, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 0},
	 {0x61, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 0},
	 {0x62, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 1},
	 {0x63, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 1},
	 {0x64, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_4_5, 	  FALSE,  FALSE,		 0},
	 {0x65, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_4_5, 	  TRUE,   FALSE,		 0},
	 {0x66, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_4_5, 	  FALSE,  FALSE,		 1},
	 {0x67, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_4_5, 	  TRUE,   FALSE,		 1},
	 {0x68, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 0},
	 {0x69, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 0},
	 {0x6A, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 1},
	 {0x6B, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 1},
	 {0x6C, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 0},
	 {0x6D, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 0},
	 {0x6E, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_8_9, 	  FALSE,  FALSE,		 1},
	 {0x6F, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_8_9, 	  TRUE,   FALSE,		 1},
	 {0x70, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 0},
	 {0x71, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 0},
	 {0x72, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_9_10,	  FALSE,  FALSE,		 1},
	 {0x73, 	  TRUE,   MtFeType_DvbS2,	  MtFeModMode_32Apsk,	  MtFeCodeRate_9_10,	  TRUE,   FALSE,		 1},
	 {0x74, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0x75, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0x76, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 1},
	 {0x77, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 1},
	 {0x78, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0x79, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0x7A, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 1},
	 {0x7B, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 1},
	 {0x7C, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0x7D, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0x7E, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 1},
	 {0x7F, 	  FALSE,  MtFeType_DvbS2,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 1},
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0x80, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0x81, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0x82, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0x83, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0x84, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_13_45,	  FALSE,  FALSE,		 0},
	 {0x85, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_13_45,	  TRUE,   FALSE,		 0},
	 {0x86, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_9_20,	  FALSE,  FALSE,		 0},
	 {0x87, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_9_20,	  TRUE,   FALSE,		 0},
	 {0x88, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_11_20,	  FALSE,  FALSE,		 0},
	 {0x89, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_11_20,	  TRUE,   FALSE,		 0},
	 {0x8A, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8Apsk_L,	  MtFeCodeRate_5_9, 	  FALSE,  FALSE,		 0},
	 {0x8B, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8Apsk_L,	  MtFeCodeRate_5_9, 	  TRUE,   FALSE,		 0},
	 {0x8C, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8Apsk_L,	  MtFeCodeRate_26_45,	  FALSE,  FALSE,		 0},
	 {0x8D, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8Apsk_L,	  MtFeCodeRate_26_45,	  TRUE,   FALSE,		 0},
	 {0x8E, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_23_36,	  FALSE,  FALSE,		 0},
	 {0x8F, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_23_36,	  TRUE,   FALSE,		 0},
	 {0x90, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_25_36,	  FALSE,  FALSE,		 0},
	 {0x91, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_25_36,	  TRUE,   FALSE,		 0},
	 {0x92, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_13_18,	  FALSE,  FALSE,		 0},
	 {0x93, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_13_18,	  TRUE,   FALSE,		 0},
	 {0x94, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_1_2, 	  FALSE,  FALSE,		 0},
	 {0x95, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_1_2, 	  TRUE,   FALSE,		 0},
	 {0x96, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_8_15,	  FALSE,  FALSE,		 0},
	 {0x97, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_8_15,	  TRUE,   FALSE,		 0},
	 {0x98, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_5_9, 	  FALSE,  FALSE,		 0},
	 {0x99, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_5_9, 	  TRUE,   FALSE,		 0},
	 {0x9A, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_26_45,	  FALSE,  FALSE,		 0},
	 {0x9B, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_26_45,	  TRUE,   FALSE,		 0},
	 {0x9C, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_5, 	  FALSE,  FALSE,		 0},
	 {0x9D, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_5, 	  TRUE,   FALSE,		 0},
	 {0x9E, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_3_5, 	  FALSE,  FALSE,		 0},
	 {0x9F, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_3_5, 	  TRUE,   FALSE,		 0},
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0xA0, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_28_45,	  FALSE,  FALSE,		 0},
	 {0xA1, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_28_45,	  TRUE,   FALSE,		 0},
	 {0xA2, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_23_36,	  FALSE,  FALSE,		 0},
	 {0xA3, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_23_36,	  TRUE,   FALSE,		 0},
	 {0xA4, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 0},
	 {0xA5, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk_L,   MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 0},
	 {0xA6, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_15,	  FALSE,  FALSE,		 0},
	 {0xA7, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_15,	  TRUE,   FALSE,		 0},
	 {0xA8, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_13_18,	  FALSE,  FALSE,		 0},
	 {0xA9, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_13_18,	  TRUE,   FALSE,		 0},
	 {0xAA, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_7_9, 	  FALSE,  FALSE,		 0},
	 {0xAB, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_7_9, 	  TRUE,   FALSE,		 0},
	 {0xAC, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_77_90,	  FALSE,  FALSE,		 0},
	 {0xAD, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_77_90,	  TRUE,   FALSE,		 0},
	 {0xAE, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk_L,   MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 0},
	 {0xAF, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk_L,   MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 0},
	 {0xB0, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0xB1, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0xB2, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_32_45,	  FALSE,  FALSE,		 0},
	 {0xB3, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_32_45,	  TRUE,   FALSE,		 0},
	 {0xB4, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_11_15,	  FALSE,  FALSE,		 0},
	 {0xB5, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_11_15,	  TRUE,   FALSE,		 0},
	 {0xB6, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_7_9, 	  FALSE,  FALSE,		 0},
	 {0xB7, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_7_9, 	  TRUE,   FALSE,		 0},
	 {0xB8, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk_L,   MtFeCodeRate_32_45,	  FALSE,  FALSE,		 0},
	 {0xB9, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk_L,   MtFeCodeRate_32_45,	  TRUE,   FALSE,		 0},
	 {0xBA, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk,	  MtFeCodeRate_11_15,	  FALSE,  FALSE,		 0},
	 {0xBB, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk,	  MtFeCodeRate_11_15,	  TRUE,   FALSE,		 0},
	 {0xBC, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0xBD, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0xBE, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk,	  MtFeCodeRate_7_9, 	  FALSE,  FALSE,		 0},
	 {0xBF, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0xC0, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0xC1, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0xC2, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk,	  MtFeCodeRate_4_5, 	  FALSE,  FALSE,		 0},
	 {0xC3, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk,	  MtFeCodeRate_4_5, 	  TRUE,   FALSE,		 0},
	 {0xC4, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0xC5, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0xC6, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk,	  MtFeCodeRate_5_6, 	  FALSE,  FALSE,		 0},
	 {0xC7, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_64Apsk,	  MtFeCodeRate_5_6, 	  TRUE,   FALSE,		 0},
	 {0xC8, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_128Apsk,	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 0},
	 {0xC9, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_128Apsk,	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 0},
	 {0xCA, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_128Apsk,	  MtFeCodeRate_7_9, 	  FALSE,  FALSE,		 0},
	 {0xCB, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_128Apsk,	  MtFeCodeRate_7_9, 	  TRUE,   FALSE,		 0},
	 {0xCC, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_29_45,	  FALSE,  FALSE,		 0},
	 {0xCD, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_29_45,	  TRUE,   FALSE,		 0},
	 {0xCE, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 0},
	 {0xCF, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 0},
	 {0xD0, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_31_45,	  FALSE,  FALSE,		 0},
	 {0xD1, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_31_45,	  TRUE,   FALSE,		 0},
	 {0xD2, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk,	  MtFeCodeRate_32_45,	  FALSE,  FALSE,		 0},
	 {0xD3, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk,	  MtFeCodeRate_32_45,	  TRUE,   FALSE,		 0},
	 {0xD4, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_11_15,	  FALSE,  FALSE,		 0},
	 {0xD5, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk_L,  MtFeCodeRate_11_15,	  TRUE,   FALSE,		 0},
	 {0xD6, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk,	  MtFeCodeRate_3_4, 	  FALSE,  FALSE,		 0},
	 {0xD7, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_256Apsk,	  MtFeCodeRate_3_4, 	  TRUE,   FALSE,		 1},
	 {0xD8, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_11_45,	  FALSE,  FALSE,		 1},
	 {0xD9, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_11_45,	  TRUE,   FALSE,		 1},
	 {0xDA, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_4_15,	  FALSE,  FALSE,		 1},
	 {0xDB, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_4_15,	  TRUE,   FALSE,		 1},
	 {0xDC, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_14_45,	  FALSE,  FALSE,		 1},
	 {0xDD, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_14_45,	  TRUE,   FALSE,		 1},
	 {0xDE, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_7_15,	  FALSE,  FALSE,		 1},
	 {0xDF, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_7_15,	  TRUE,   FALSE,		 1},
 //   PLS Code,   Valid,  DVB Type, 		  Mod Mode, 			  Code Rate,			  Pilot,  Dummy Frames,  Frame Length
	 {0xE0, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_8_15,	  FALSE,  FALSE,		 1},
	 {0xE1, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_8_15,	  TRUE,   FALSE,		 1},
	 {0xE2, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_32_45,	  FALSE,  FALSE,		 1},
	 {0xE3, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_Qpsk, 	  MtFeCodeRate_32_45,	  TRUE,   FALSE,		 1},
	 {0xE4, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_7_15,	  FALSE,  FALSE,		 1},
	 {0xE5, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_7_15,	  TRUE,   FALSE,		 1},
	 {0xE6, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_8_15,	  FALSE,  FALSE,		 1},
	 {0xE7, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_8_15,	  TRUE,   FALSE,		 1},
	 {0xE8, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_26_45,	  FALSE,  FALSE,		 1},
	 {0xE9, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_26_45,	  TRUE,   FALSE,		 1},
	 {0xEA, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_32_45,	  FALSE,  FALSE,		 1},
	 {0xEB, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_8psk, 	  MtFeCodeRate_32_45,	  TRUE,   FALSE,		 1},
	 {0xEC, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_7_15,	  FALSE,  FALSE,		 1},
	 {0xED, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_7_15,	  TRUE,   FALSE,		 1},
	 {0xEE, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_15,	  FALSE,  FALSE,		 1},
	 {0xEF, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_8_15,	  TRUE,   FALSE,		 1},
	 {0xF0, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_26_45,	  FALSE,  FALSE,		 1},
	 {0xF1, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_26_45,	  TRUE,   FALSE,		 1},
	 {0xF2, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_5, 	  FALSE,  FALSE,		 1},
	 {0xF3, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_3_5, 	  TRUE,   FALSE,		 1},
	 {0xF4, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_32_45,	  FALSE,  FALSE,		 1},
	 {0xF5, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_16Apsk,	  MtFeCodeRate_32_45,	  TRUE,   FALSE,		 1},
	 {0xF6, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_2_3, 	  FALSE,  FALSE,		 1},
	 {0xF7, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_2_3, 	  TRUE,   FALSE,		 1},
	 {0xF8, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_32_45,	  FALSE,  FALSE,		 1},
	 {0xF9, 	  TRUE,   MtFeType_DvbS2X,	  MtFeModMode_32Apsk,	  MtFeCodeRate_32_45,	  TRUE,   FALSE,		 1},
	 {0xFA, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0xFB, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0xFC, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0xFD, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0},
	 {0xFE, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  FALSE,  FALSE,		 0},
	 {0xFF, 	  FALSE,  MtFeType_DvbS2X,	  MtFeModMode_Undef,	  MtFeCodeRate_Undef,	  TRUE,   FALSE,		 0}
 };

static int rs6060_set_tuner_reg(struct m88rs6060_state *dev, u8 reg, u8 data)
{
	struct i2c_client *client = dev->demod_client;
	u8 buf[] = { reg, data };
	u8 val;
	int ret;
	struct i2c_msg msg = {
		.addr = dev->config.tuner_adr,.flags = 0,.buf = buf,.len = 2
	};

	val = 0x11;
	ret = regmap_write(dev->demod_regmap, 0x03, val);
	if (ret)
		dev_dbg(&client->dev, "fail=%d\n", ret);

	ret = i2c_transfer(dev->tuner_client->adapter, &msg, 1);
	if (ret != 1) {
		dev_err(&client->dev,
			"0x%02x (ret=%i, reg=0x%02x, value=0x%02x)\n",
			dev->config.tuner_adr, ret, reg, data);
		return -EREMOTEIO;
	}

	dev_dbg(&client->dev, "0x%02x reg 0x%02x, value 0x%02x\n",
		dev->config.tuner_adr, reg, data);

	return 0;
}

static int rs6060_get_tuner_reg(struct m88rs6060_state *dev, u8 reg)
{

	struct i2c_client *client = dev->demod_client;
	int ret;
	unsigned val;
	u8 b0[] = { reg };
	u8 b1[] = { 0 };
	struct i2c_msg msg[] = {
		{
		 .addr = dev->config.tuner_adr,
		 .flags = 0,
		 .buf = b0,
		 .len = 1},
		{
		 .addr = dev->config.tuner_adr,
		 .flags = I2C_M_RD,
		 .buf = b1,
		 .len = 1}
	};

	val = dev->config.repeater_value;
	ret = regmap_write(dev->demod_regmap, 0x03, val);
	if (ret)
		dev_dbg(&client->dev, "fail=%d\n", ret);

	ret = i2c_transfer(dev->tuner_client->adapter, msg, 2);
	if (ret != 2) {
		dev_err(&client->dev, "0x%02x (ret=%d, reg=0x%02x)\n",
			dev->config.tuner_adr, ret, reg);
		return -EREMOTEIO;
	}

	dev_dbg(&client->dev, "0x%02x reg 0x%02x, value 0x%02x\n",
		dev->config.tuner_adr, reg, b1[0]);

	return b1[0];

}

static int m88rs6060_fireware_download(struct m88rs6060_state *dev, u8 reg,
				       const u8 * data, int len)
{
	struct i2c_client *client = dev->demod_client;
	int ret;
    u8 buf[70];
    struct i2c_msg msg = {
		.addr = dev->config.demod_adr,.flags = 0,.buf = buf,.len =
		    len + 1
	};

    buf[0] = reg;
	memcpy(&buf[1], data, len);

	ret = i2c_transfer(dev->demod_client->adapter, &msg, 1);
	if (ret != 1) {
		dev_err(&client->dev,
			"0x%02x (ret=%i, reg=0x%02x)\n",
			dev->config.tuner_adr, ret, reg);
		return -EREMOTEIO;
	}

	return 0;
}

static int m88rs6060_update_bits(struct m88rs6060_state* state, u8 reg, u8 mask, u8 val)
{
	int ret;
	unsigned tmp;

	/* no need for read if whole reg is written */
	if (mask != 0xff) {
		ret = regmap_read(state->demod_regmap, reg, &tmp);
		if (ret)
			return ret;

		val &= mask;
		tmp &= ~mask;
		val |= tmp;
	}

	return regmap_write(state->demod_regmap, reg, val);
}

static void m88rs6060_calc_PLS_gold_code(u8 * pNormalCode, u32 PLSGoldCode)
{
	struct PLS_Table_t {
		u32 iIndex;
		u8 PLSCode[3];
	};

	struct PLS_Table_t PLS_List[] = {
		{0, {0x01, 0x00, 0x00}},
		{5000, {0x0d, 0xe0, 0x00}},
		{10000, {0x51, 0x15, 0x00}},
		{15000, {0xcf, 0xc9, 0x00}},
		{20000, {0x67, 0x33, 0x03}},
		{25000, {0x02, 0xc9, 0x02}},
		{30000, {0xe5, 0xc6, 0x01}},
		{35000, {0xdb, 0xc0, 0x03}},
		{40000, {0x7c, 0x5f, 0x02}},
		{45000, {0x8d, 0x65, 0x00}},
		{50000, {0x14, 0x96, 0x00}},
		{55000, {0xf7, 0x61, 0x03}},
		{60000, {0xbc, 0x28, 0x00}},
		{65000, {0x77, 0xa9, 0x01}},
		{70000, {0xe7, 0x05, 0x01}},
		{75000, {0x88, 0x85, 0x01}},
		{80000, {0x2f, 0xbb, 0x02}},
		{85000, {0xe1, 0x07, 0x00}},
		{90000, {0xd5, 0x67, 0x01}},
		{95000, {0x94, 0x37, 0x03}},
		{100000, {0x57, 0x39, 0x02}},
		{105000, {0xc7, 0x03, 0x00}},
		{110000, {0xbf, 0x12, 0x00}},
		{115000, {0x50, 0x0e, 0x00}},
		{120000, {0xca, 0xc4, 0x00}},
		{125000, {0x46, 0xc3, 0x00}},
		{130000, {0x2f, 0xc6, 0x01}},
		{135000, {0x7c, 0xe5, 0x01}},
		{140000, {0xb9, 0x36, 0x01}},
		{145000, {0x9d, 0xe5, 0x01}},
		{150000, {0xc4, 0x32, 0x01}},
		{155000, {0x13, 0xb3, 0x00}},
		{160000, {0x0c, 0x9f, 0x02}},
		{165000, {0xb2, 0xb5, 0x03}},
		{170000, {0xac, 0x7e, 0x01}},
		{175000, {0xb6, 0xa2, 0x01}},
		{180000, {0xb6, 0x3e, 0x01}},
		{185000, {0x17, 0x2c, 0x02}},
		{190000, {0xd7, 0x2a, 0x02}},
		{195000, {0x93, 0x61, 0x02}},
		{200000, {0x67, 0x92, 0x02}},
		{205000, {0x38, 0x07, 0x01}},
		{210000, {0xb4, 0x5a, 0x01}},
		{215000, {0xed, 0x31, 0x02}},
		{220000, {0x9e, 0x4d, 0x02}},
		{225000, {0x17, 0x08, 0x02}},
		{230000, {0x37, 0xb9, 0x00}},
		{235000, {0x2c, 0xed, 0x00}},
		{240000, {0xe0, 0x64, 0x00}},
		{245000, {0x90, 0x39, 0x01}},
		{250000, {0x35, 0x0e, 0x01}},
		{255000, {0x1c, 0x9e, 0x02}},
		{260000, {0x58, 0x78, 0x00}}
	};

	u8 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15,
	    x16, x17;
	int i;
	u8 tmp;

	u32 ulPLSCode = 0, ulPLSIndex = 0;

	int iPLSCnt =
	    sizeof(PLS_List) / sizeof(struct PLS_Table_t), iPLSListIndex;
	u8 iPLSCode[3];

	ulPLSCode = PLSGoldCode;
	iPLSListIndex = ulPLSCode / 5000;

	if (iPLSListIndex > iPLSCnt - 1)
		iPLSListIndex = iPLSCnt - 1;

	ulPLSIndex = PLS_List[iPLSListIndex].iIndex;
	iPLSCode[0] = PLS_List[iPLSListIndex].PLSCode[0];
	iPLSCode[1] = PLS_List[iPLSListIndex].PLSCode[1];
	iPLSCode[2] = PLS_List[iPLSListIndex].PLSCode[2];

	x0 = (iPLSCode[0] >> 0) & 0x01;
	x1 = (iPLSCode[0] >> 1) & 0x01;
	x2 = (iPLSCode[0] >> 2) & 0x01;
	x3 = (iPLSCode[0] >> 3) & 0x01;
	x4 = (iPLSCode[0] >> 4) & 0x01;
	x5 = (iPLSCode[0] >> 5) & 0x01;
	x6 = (iPLSCode[0] >> 6) & 0x01;
	x7 = (iPLSCode[0] >> 7) & 0x01;
	x8 = (iPLSCode[1] >> 0) & 0x01;
	x9 = (iPLSCode[1] >> 1) & 0x01;
	x10 = (iPLSCode[1] >> 2) & 0x01;
	x11 = (iPLSCode[1] >> 3) & 0x01;
	x12 = (iPLSCode[1] >> 4) & 0x01;
	x13 = (iPLSCode[1] >> 5) & 0x01;
	x14 = (iPLSCode[1] >> 6) & 0x01;
	x15 = (iPLSCode[1] >> 7) & 0x01;
	x16 = (iPLSCode[2] >> 0) & 0x01;
	x17 = (iPLSCode[2] >> 1) & 0x01;

	for (i = ulPLSIndex; i <= ulPLSCode; i++) {
		iPLSCode[0] =
		    (x7 << 7) + (x6 << 6) + (x5 << 5) + (x4 << 4) + (x3 << 3) +
		    (x2 << 2) + (x1 << 1) + (x0 << 0);
		iPLSCode[1] =
		    (x15 << 7) + (x14 << 6) + (x13 << 5) + (x12 << 4) +
		    (x11 << 3) + (x10 << 2) + (x9 << 1) + (x8 << 0);
		iPLSCode[2] = (x17 << 1) + x16;

		tmp = (x0 ^ x7) & 0x01;
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = x4;
		x4 = x5;
		x5 = x6;
		x6 = x7;
		x7 = x8;
		x8 = x9;
		x9 = x10;
		x10 = x11;
		x11 = x12;
		x12 = x13;
		x13 = x14;
		x14 = x15;
		x15 = x16;
		x16 = x17;
		x17 = tmp;

	}

	pNormalCode[0] = iPLSCode[0];
	pNormalCode[1] = iPLSCode[1];
	pNormalCode[2] = iPLSCode[2];

	return;
}

static int m88rs6060_get_gain(struct m88rs6060_state *dev, u32 freq_MHz, s32 * p_gain)
{
	static s32 bb_list_dBm[16][16] = {
		{-5000, -4999, -4397, -4044, -3795, -3601, -3442, -3309, -3193,
		 -3090, -2999, -2916, -2840, -2771, -2706, -2647},
		{-2590, -2538, -2488, -2441, -2397, -2354, -2314, -2275, -2238,
		 -2203, -2169, -2136, -2104, -2074, -2044, -2016},
		{-1988, -1962, -1936, -1911, -1886, -1862, -1839, -1817, -1795,
		 -1773, -1752, -1732, -1712, -1692, -1673, -1655},
		{-1636, -1618, -1601, -1584, -1567, -1550, -1534, -1518, -1502,
		 -1487, -1472, -1457, -1442, -1428, -1414, -1400},
		{-1386, -1373, -1360, -1347, -1334, -1321, -1309, -1296, -1284,
		 -1272, -1260, -1249, -1237, -1226, -1215, -1203},
		{-1193, -1182, -1171, -1161, -1150, -1140, -1130, -1120, -1110,
		 -1100, -1090, -1081, -1071, -1062, -1052, -1043},
		{-1034, -1025, -1016, -1007, -999, -990, -982, -973, -965, -956,
		 -948, -940, -932, -924, -916, -908},
		{-900, -893, -885, -877, -870, -862, -855, -848, -840, -833,
		 -826, -819, -812, -805, -798, -791},
		{-784, -778, -771, -764, -758, -751, -745, -738, -732, -725,
		 -719, -713, -706, -700, -694, -688},
		{-682, -676, -670, -664, -658, -652, -647, -641, -635, -629,
		 -624, -618, -612, -607, -601, -596},
		{-590, -585, -580, -574, -569, -564, -558, -553, -548, -543,
		 -538, -533, -528, -523, -518, -513},
		{-508, -503, -498, -493, -488, -483, -479, -474, -469, -464,
		 -460, -455, -450, -446, -441, -437},
		{-432, -428, -423, -419, -414, -410, -405, -401, -397, -392,
		 -388, -384, -379, -375, -371, -367},
		{-363, -358, -354, -350, -346, -342, -338, -334, -330, -326,
		 -322, -318, -314, -310, -306, -302},
		{-298, -294, -290, -287, -283, -279, -275, -271, -268, -264,
		 -260, -257, -253, -249, -246, -242},
		{-238, -235, -231, -227, -224, -220, -217, -213, -210, -206,
		 -203, -199, -196, -192, -189, -186}

	};

	s32 BB_Power = 0;
	u32 Total_Gain = 8000;
	s32 delta = 0;

	u8 reg5a, reg5f, reg77, reg76, reg3f;
	u8 reg96 = 0;

	u32 PGA2_cri_GS = 46, PGA2_crf_GS = 290, TIA_GS = 290;
	u32 RF_GC = 1200, IF_GC = 1100, BB_GC = 300, PGA2_GC = 300, TIA_GC =
	    300;
	u32 PGA2_cri = 0, PGA2_crf = 0;
	u32 RFG = 0, IFG = 0, BBG = 0, PGA2G = 0, TIAG = 0;

	u32 i = 0;

	u32 RFGS[13] = { 0, 276, 278, 283, 272, 294, 296, 292, 292, 299, 305, 292, 300 };
	u32 IFGS[12] = { 0, 0, 232, 268, 266, 289, 295, 290, 291, 298, 304, 304 };
	u32 BBGS[13] = { 0, 296, 297, 295, 298, 302, 293, 292, 286, 294, 278, 298, 267 };

	reg5a = rs6060_get_tuner_reg(dev, 0x5A);
	RF_GC = reg5a & 0x0f;

	reg5f = rs6060_get_tuner_reg(dev, 0x5F);
	IF_GC = reg5f & 0x0f;

	reg3f = rs6060_get_tuner_reg(dev, 0x3F);
	TIA_GC = (reg3f >> 4) & 0x07;

	reg77 = rs6060_get_tuner_reg(dev, 0x77);
	BB_GC = (reg77 >> 4) & 0x0f;

	reg76 = rs6060_get_tuner_reg(dev, 0x76);
	PGA2_GC = reg76 & 0x3f;
	PGA2_cri = PGA2_GC >> 2;
	PGA2_crf = PGA2_GC & 0x03;

	if (freq_MHz >= 1750) {
		RFGS[1] = 240;
		RFGS[2] = 260;
		IFGS[2] = 200;
		IFGS[3] = 245;
		IFGS[4] = 255;
	} else if (freq_MHz >= 1350) {
		RFGS[12] = 285;
	} else {
		RFGS[1] = 310;
		RFGS[2] = 293;
		IFGS[2] = 270;
		IFGS[3] = 290;
		IFGS[4] = 280;
		IFGS[11] = 320;
	}

	for (i = 0; i <= RF_GC; i++) {
		RFG += RFGS[i];
	}

	for (i = 1; i <= IF_GC; i++) {
		IFG += IFGS[i];
	}

	TIAG = TIA_GC * TIA_GS;

	for (i = 0; i <= BB_GC; i++) {
		BBG += BBGS[i];
	}

	PGA2G = PGA2_cri * PGA2_cri_GS + PGA2_crf * PGA2_crf_GS;
	Total_Gain = RFG + IFG - TIAG + BBG + PGA2G;

	if (freq_MHz >= 1750) {
		delta = 800;
	} else if (freq_MHz >= 1350) {
		delta = 900;
	} else {
		delta = 1000;
	}

	reg96 = rs6060_get_tuner_reg(dev, 0x96);
	BB_Power = bb_list_dBm[(reg96 >> 4) & 0x0f][reg96 & 0x0f];
	*p_gain = Total_Gain - delta - BB_Power;

	return 0;

}

static void rs6060_wakeup(struct m88rs6060_state *dev)
{
	rs6060_set_tuner_reg(dev, 0x10, 0xfb);
	rs6060_set_tuner_reg(dev, 0x11, 0x1);
	rs6060_set_tuner_reg(dev, 0x7, 0x7d);
	msleep(10);

	return;
}

static void rs6060_sleep(struct m88rs6060_state *dev)
{
	rs6060_set_tuner_reg(dev, 0x7, 0x6d);
	rs6060_set_tuner_reg(dev, 0x10, 0x00);
	rs6060_set_tuner_reg(dev, 0x11, 0x7d);
	msleep(10);
	return;
}

static void m88rs6060_hard_rest(struct m88rs6060_state *dev)
{
	unsigned val;

	rs6060_set_tuner_reg(dev, 0x4, 0x1);
	rs6060_set_tuner_reg(dev, 0x4, 0x0);

	msleep(1);
	rs6060_wakeup(dev);


	regmap_read(dev->demod_regmap, 0x08, &val);
	regmap_write(dev->demod_regmap, 0x8, (val | 0x1));

	regmap_read(dev->demod_regmap, 0x0b, &val);
	regmap_write(dev->demod_regmap, 0x0b, (val | 0x1));

	regmap_read(dev->demod_regmap, 0xb2, &val);
	if (val == 0x1) {
		regmap_write(dev->demod_regmap, 0x00, 0x00);
		regmap_write(dev->demod_regmap, 0xb2, 0x00);
	}

	regmap_write(dev->demod_regmap, 0x07, 0x80);
	regmap_write(dev->demod_regmap, 0x07, 0x00);
	msleep(1);
	regmap_read(dev->demod_regmap, 0x08, &val);
	regmap_write(dev->demod_regmap, 0x8, (val | 0x1));

	return;
}

static void rs6060_select_mclk(struct m88rs6060_state *dev, u32 freq_MHz,
			       u32 symbol_rate)
{
	u32 adc_freq_MHz[3] = { 96, 93, 99 };
	u8 reg16_list[3] = { 96, 92, 100 }, reg15, reg16;
	u32 offset_MHz[3];
	u32 max_offset = 0;
	int i = 0;
	reg16 = 96;
	dev->mclk = 96000;
	if (symbol_rate >= 46000) {
		dev->mclk = 99000;
		reg16 = 100;
	} else {
		for (i = 0; i < 3; i++) {
			offset_MHz[i] = freq_MHz % adc_freq_MHz[i];

			if (offset_MHz[i] > (adc_freq_MHz[i] / 2))
				offset_MHz[i] = adc_freq_MHz[i] - offset_MHz[i];

			if (offset_MHz[i] > max_offset) {

				max_offset = offset_MHz[i];
				reg16 = reg16_list[i];
				dev->mclk = adc_freq_MHz[i] * 1000;

			}

		}

	}
	reg15 = rs6060_get_tuner_reg(dev, 0x15);
	reg15 &= ~0x1;
	rs6060_set_tuner_reg(dev, 0x15, reg15);
	rs6060_set_tuner_reg(dev, 0x16, reg16);
	rs6060_set_tuner_reg(dev, 0x17, 0xc1);
	rs6060_set_tuner_reg(dev, 0x17, 0x81);

	msleep(5);

	return;

}

static void rs6060_set_ts_mclk(struct m88rs6060_state *dev, u32 mclk)
{
	u8 reg15, reg16, reg1D, reg1E, reg1F, tmp;
	u8 sm, f0 = 0, f1 = 0, f2 = 0, f3 = 0;
	u16 pll_div_fb, N;
	u32 div;

	reg15 = rs6060_get_tuner_reg(dev, 0x15);
	reg16 = rs6060_get_tuner_reg(dev, 0x16);
	reg1D = rs6060_get_tuner_reg(dev, 0x1d);

	if (dev->config.ts_mode != MtFeTsOutMode_Serial) {
		if (reg16 == 92) {
			tmp = 93;
		} else if (reg16 == 100) {
			tmp = 99;
		} else		// if(reg16 == 96)
		{
			tmp = 96;
		}

		mclk *= tmp;
		mclk /= 96;
	}

	pll_div_fb = (reg15 & 0x01) << 8;
	pll_div_fb += reg16;
	pll_div_fb += 32;

	div = 9000 * pll_div_fb * 4;
	div /= mclk;

	if (dev->config.ts_mode == MtFeTsOutMode_Serial) {
		if (div <= 32) {
			N = 2;

			f0 = 0;
			f1 = div / N;
			f2 = div - f1;
			f3 = 0;
		} else if (div <= 64) {
			N = 4;

			f0 = div / N;
			f1 = (div - f0) / (N - 1);
			f2 = (div - f0 - f1) / (N - 2);
			f3 = div - f0 - f1 - f2;
		} else {
			N = 4;

			f0 = 16;
			f1 = 16;
			f2 = 16;
			f3 = 16;
		}

		if (f0 == 16)
			f0 = 0;
		else if ((f0 < 8) && (f0 != 0))
			f0 = 8;

		if (f1 == 16)
			f1 = 0;
		else if ((f1 < 8) && (f1 != 0))
			f1 = 8;

		if (f2 == 16)
			f2 = 0;
		else if ((f2 < 8) && (f2 != 0))
			f2 = 8;

		if (f3 == 16)
			f3 = 0;
		else if ((f3 < 8) && (f3 != 0))
			f3 = 8;
	} else {
		if (div <= 32) {
			N = 2;

			f0 = 0;
			f1 = div / N;
			f2 = div - f1;
			f3 = 0;
		} else if (div <= 48) {
			N = 3;

			f0 = div / N;
			f1 = (div - f0) / (N - 1);
			f2 = div - f0 - f1;
			f3 = 0;
		} else if (div <= 64) {
			N = 4;

			f0 = div / N;
			f1 = (div - f0) / (N - 1);
			f2 = (div - f0 - f1) / (N - 2);
			f3 = div - f0 - f1 - f2;
		} else {
			N = 4;

			f0 = 16;
			f1 = 16;
			f2 = 16;
			f3 = 16;
		}

		if (f0 == 16)
			f0 = 0;
		else if ((f0 < 9) && (f0 != 0))
			f0 = 9;

		if (f1 == 16)
			f1 = 0;
		else if ((f1 < 9) && (f1 != 0))
			f1 = 9;

		if (f2 == 16)
			f2 = 0;
		else if ((f2 < 9) && (f2 != 0))
			f2 = 9;

		if (f3 == 16)
			f3 = 0;
		else if ((f3 < 9) && (f3 != 0))
			f3 = 9;
	}

	sm = N - 1;

	reg1D &= ~0x03;
	reg1D |= sm;
	reg1D |= 0x80;

	reg1E = ((f3 << 4) + f2) & 0xFF;
	reg1F = ((f1 << 4) + f0) & 0xFF;

	rs6060_set_tuner_reg(dev, 0x1d, reg1D);
	rs6060_set_tuner_reg(dev, 0x1e, reg1E);
	rs6060_set_tuner_reg(dev, 0x1f, reg1F);
	msleep(1);

}

static int rs6060_get_ts_mclk(struct m88rs6060_state *dev,u32*mclk )
{
	u8 reg15, reg16, reg1D, reg1E, reg1F;
	u8 sm, f0, f1, f2, f3;
	u16 pll_div_fb, N;
	u32 MCLK_KHz;


	*mclk = MT_FE_MCLK_KHZ;

	reg15 = rs6060_get_tuner_reg(dev, 0x15);
	reg16 = rs6060_get_tuner_reg(dev, 0x16);
	reg1D = rs6060_get_tuner_reg(dev, 0x1d);
	reg1E = rs6060_get_tuner_reg(dev, 0x1e);
	reg1F = rs6060_get_tuner_reg(dev, 0x1f);

	MCLK_KHz = 9000;
	pll_div_fb = reg15 & 0x01;
	pll_div_fb <<= 8;
	pll_div_fb += reg16;

	MCLK_KHz *= (pll_div_fb + 32);

	sm = reg1D & 0x03;

	f3 = (reg1E >> 4) & 0x0F;
	f2 = reg1E & 0x0F;
	f1 = (reg1F >> 4) & 0x0F;
	f0 = reg1F & 0x0F;

	if(f3 == 0)		f3 = 16;
	if(f2 == 0)		f2 = 16;
	if(f1 == 0)		f1 = 16;
	if(f0 == 0)		f0 = 16;
	N = f2 + f1;

	switch(sm){
		case 3:
			N = f3 + f2 + f1 + f0;
			break;
		case 2:
			N = f2 + f1 + f0;
			break;
		case 1:
		case 0:
		default:
			N = f2 + f1;
			break;
	}

	MCLK_KHz *= 4;
	MCLK_KHz /= N;

	*mclk = MCLK_KHz;

	return 0;

}

static int rs6060_set_pll_freq(struct m88rs6060_state *dev, u32 tuner_freq_MHz)
{
	u32 fcry_KHz, ulNDiv1, ulNDiv2;
	u8 refDiv1, refDiv2, ucLoDiv1, ucLomod1, ucLoDiv2, ucLomod2, div1m,
	    div1p5m, lodiv_en_opt_div2;
	u8 reg27, reg29;
	u8 tmp;

	fcry_KHz = dev->config.clk / 1000;	/*tuner cycle */
	if (fcry_KHz == 27000) {

		div1m = 19;
		div1p5m = 10;
		rs6060_set_tuner_reg(dev, 0x41, 0x82);
	} else if (fcry_KHz == 24000) {
		div1m = 16;
		div1p5m = 8;
		rs6060_set_tuner_reg(dev, 0x41, 0x8a);

	} else {

		div1m = 19;
		div1p5m = 10;
		rs6060_set_tuner_reg(dev, 0x41, 0x82);

	}
	if (tuner_freq_MHz >= 1550) {
		ucLoDiv1 = 2;
		ucLomod1 = 0;
		refDiv1 = div1m;
		ucLoDiv2 = 2;
		ucLomod2 = 0;
		refDiv2 = div1m;
		lodiv_en_opt_div2 = 0;
	} else if (tuner_freq_MHz >= 1380) {
		ucLoDiv1 = 3;
		ucLomod1 = 16;
		refDiv1 = div1p5m;
		ucLoDiv2 = 2;
		ucLomod2 = 0;
		refDiv2 = div1m;
		lodiv_en_opt_div2 = 0;
	} else if (tuner_freq_MHz >= 1070) {
		ucLoDiv1 = 3;
		ucLomod1 = 16;
		refDiv1 = div1p5m;
		ucLoDiv2 = 3;
		ucLomod2 = 16;
		refDiv2 = div1p5m;
		lodiv_en_opt_div2 = 0;
	} else if (tuner_freq_MHz >= 1000) {
		ucLoDiv1 = 4;
		ucLomod1 = 64;
		refDiv1 = div1m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2 = div1m;
		lodiv_en_opt_div2 = 0;
	} else if (tuner_freq_MHz >= 775) {
		ucLoDiv1 = 4;
		ucLomod1 = 64;
		refDiv1 = div1m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2 = div1m;
		lodiv_en_opt_div2 = 0;
	} else if (tuner_freq_MHz >= 700) {
		ucLoDiv1 = 6;
		ucLomod1 = 48;
		refDiv1 = div1p5m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2 = div1m;
		lodiv_en_opt_div2 = 0;
	} else if (tuner_freq_MHz >= 520) {
		ucLoDiv1 = 6;
		ucLomod1 = 48;
		refDiv1 = div1p5m;
		ucLoDiv2 = 6;
		ucLomod2 = 48;
		refDiv2 = div1p5m;
		lodiv_en_opt_div2 = 0;
	} else if (tuner_freq_MHz >= 375) {
		ucLoDiv1 = 8;
		ucLomod1 = 96;
		refDiv1 = div1m;
		ucLoDiv2 = 8;
		ucLomod2 = 96;
		refDiv2 = div1m;
		lodiv_en_opt_div2 = 0;
	} else {
		ucLoDiv1 = 12;
		ucLomod1 = 80;
		refDiv1 = div1m;
		ucLoDiv2 = 12;
		ucLomod2 = 80;
		refDiv2 = div1m;
		lodiv_en_opt_div2 = 1;
	}

	ulNDiv1 =
	    ((tuner_freq_MHz * 1000 * ucLoDiv1) * (refDiv1 + 8) / fcry_KHz -
	     1024) / 2;
	ulNDiv2 =
	    ((tuner_freq_MHz * 1000 * ucLoDiv2) * (refDiv2 + 8) / fcry_KHz -
	     1024) / 2;

	reg27 = (((ulNDiv1 >> 8) & 0x0F) + ucLomod1) & 0x7F;

	rs6060_set_tuner_reg(dev, 0x27, reg27);
	rs6060_set_tuner_reg(dev, 0x28, (u8) (ulNDiv1 & 0xFF));

	reg29 = (((ulNDiv2 >> 8) & 0x0F) + ucLomod2) & 0x7f;

	rs6060_set_tuner_reg(dev, 0x29, reg29);
	rs6060_set_tuner_reg(dev, 0x2a, (u8) (ulNDiv2 & 0xFF));

	refDiv1 = refDiv1 & 0x1F;
	rs6060_set_tuner_reg(dev, 0x36, refDiv1);

	rs6060_set_tuner_reg(dev, 0x39, refDiv2);

	tmp = rs6060_get_tuner_reg(dev, 0x3d);
	if (lodiv_en_opt_div2 == 1)
		tmp |= 0x80;
	else
		tmp &= 0x7F;

	rs6060_set_tuner_reg(dev, 0x3d, tmp);

	if (refDiv1 == 19) {
		rs6060_set_tuner_reg(dev, 0x2c, 0x02);
	} else {
		rs6060_set_tuner_reg(dev, 0x2c, 0x00);
	}

	return 0;
}

/*
	set tuner bandwith according to symbol rate, adding safety margin of 2MHz
	but add lpf_offset_KHz
	for symbol rates below 5MS/s also add, 3Ms/s which is a frequency offset applied
 */
static int rs6060_set_bb(struct m88rs6060_state *dev,
			 u32 symbol_rate_KSs, s32 lpf_offset_KHz)
{
	u32 f3dB;
	u8 reg40;

	f3dB = symbol_rate_KSs * 9 / 14 + 2000; //bw approx 1.3 * symbolrate
	f3dB += lpf_offset_KHz;
	f3dB = clamp_val(f3dB, 6000U, 43000U);
	reg40 = f3dB / 1000;

	return rs6060_set_tuner_reg(dev, 0x40, reg40);

}

static void rs6060_init(struct m88rs6060_state *dev)
{
	rs6060_set_tuner_reg(dev, 0x15, 0x6c);
	rs6060_set_tuner_reg(dev, 0x2b, 0x1e);

	rs6060_wakeup(dev);

	rs6060_set_tuner_reg(dev, 0x24, 0x4);
	rs6060_set_tuner_reg(dev, 0x6e, 0x39);
	rs6060_set_tuner_reg(dev, 0x83, 0x1);

	rs6060_set_tuner_reg(dev, 0x70, 0x90);
	rs6060_set_tuner_reg(dev, 0x71, 0xF0);
	rs6060_set_tuner_reg(dev, 0x72, 0xB6);
	rs6060_set_tuner_reg(dev, 0x73, 0xEB);
	rs6060_set_tuner_reg(dev, 0x74, 0x6F);
	rs6060_set_tuner_reg(dev, 0x75, 0xFC);

	return;
}

static void m88res6060_set_ts_mode(struct m88rs6060_state *dev)
{
	unsigned tmp, val = 0;
	regmap_read(dev->demod_regmap, 0x0b, &val);
	val &= ~0x1;
	regmap_write(dev->demod_regmap, 0x0b, val);

	regmap_read(dev->demod_regmap, 0xfd, &tmp);
	if (dev->config.ts_mode == MtFeTsOutMode_Parallel) {
		tmp &= ~0x01;
		tmp &= ~0x04;

		regmap_write(dev->demod_regmap, 0xfa, 0x01);
		regmap_write(dev->demod_regmap, 0xf1, 0x60);
		regmap_write(dev->demod_regmap, 0xfa, 0x00);
	} else if (dev->config.ts_mode == MtFeTsOutMode_Serial) {

		tmp &= ~0x01;
		tmp |= 0x04;
	} else {
		tmp |= 0x01;
		tmp &= ~0x04;

		regmap_write(dev->demod_regmap, 0xfa, 0x01);
		regmap_write(dev->demod_regmap, 0xf1, 0x60);
		regmap_write(dev->demod_regmap, 0xfa, 0x00);
	}

	tmp &= ~0xb8;
	tmp |= 0x42;
	tmp |= 0x80;
	regmap_write(dev->demod_regmap, 0xfd, tmp);

	val = 0;
	if (dev->config.ts_mode != MtFeTsOutMode_Serial) {
		tmp = MT_FE_ENHANCE_TS_PIN_LEVEL_PARALLEL_CI;

		val |= tmp & 0x03;
		val |= (tmp << 2) & 0x0c;
		val |= (tmp << 4) & 0x30;
		val |= (tmp << 6) & 0xc0;
	} else {
		val = 0x00;
	}

	regmap_write(dev->demod_regmap, 0x0a, val);

	regmap_read(dev->demod_regmap, 0x0b, &tmp);
	if (dev->config.ts_pinswitch) {
		tmp |= 0x20;
	} else {
		tmp &= ~0x20;
	}
	tmp |= 0x1;
	printk("tmp = 0x%x \n",tmp);
	regmap_write(dev->demod_regmap, 0x0b, tmp);
	regmap_read(dev->demod_regmap, 0x0c, &tmp);
	regmap_write(dev->demod_regmap, 0xf4, 0x01);
	tmp &= ~0x80;
	regmap_write(dev->demod_regmap, 0x0c, tmp);


	return;

}

static int m88rs6060_get_channel_info(struct m88rs6060_state *dev,
						struct MT_FE_CHAN_INFO_DVBS2*p_info)
{
	unsigned tmp,val_0x17,val_0x18,ucPLSCode;

	regmap_read(dev->demod_regmap,0x08,&tmp);
	if((tmp&0x08)==0x08) {    //dvbs2 signal
		dprintk("delsys=DVBS2\n");
		p_info->type = MtFeType_DvbS2;

		regmap_write(dev->demod_regmap,0x8a,0x01);
		regmap_read(dev->demod_regmap,0x17,&val_0x17);
		p_info->is_dummy_frame = (val_0x17&0x40)?true:false;

		regmap_read(dev->demod_regmap,0x18,&val_0x18);
		tmp = (val_0x18>>5) & 0x07;
		switch(tmp){
		case 1: p_info->mod_mode = MtFeModMode_8psk; break;
		case 2: p_info->mod_mode = MtFeModMode_16Apsk; break;
		case 3: p_info->mod_mode = MtFeModMode_32Apsk; break;
		case 4: p_info->mod_mode = MtFeModMode_64Apsk; break;
		case 5: p_info->mod_mode = MtFeModMode_128Apsk; break;
		case 6:
		case 7: p_info->mod_mode = MtFeModMode_256Apsk; break;
		case 0:
		default: p_info->mod_mode = MtFeModMode_Qpsk; break;
		}
		dprintk("modulation=%d\n", p_info->mod_mode);

		p_info->iVcmCycle = val_0x18 &0x1F;
		regmap_read(dev->demod_regmap,0x19,&ucPLSCode);
		p_info->iPlsCode = ucPLSCode;
		p_info->type = mPLSInfoTable[ucPLSCode].mDvbType;

		if(mPLSInfoTable[ucPLSCode].bValid){
			p_info->mod_mode = mPLSInfoTable[ucPLSCode].mModMode;
			p_info->code_rate = mPLSInfoTable[ucPLSCode].mCodeRate;
			p_info->is_dummy_frame = mPLSInfoTable[ucPLSCode].bDummyFrame;
			p_info->is_pilot_on = mPLSInfoTable[ucPLSCode].bPilotOn;
			p_info->iFrameLength = mPLSInfoTable[ucPLSCode].iFrameLength;

		} else {
			//p_info->mod_mode = mPLSInfoTable[ucPLSCode].mModMode;
			p_info->code_rate = mPLSInfoTable[ucPLSCode].mCodeRate;
			//p_info->is_dummy_frame = mPLSInfoTable[ucPLSCode].bDummyFrame;
			p_info->is_pilot_on = mPLSInfoTable[ucPLSCode].bPilotOn;
			p_info->iFrameLength = mPLSInfoTable[ucPLSCode].iFrameLength;

		}
		/*matype[0:1]= rolloff
			matype[2] = null packet deletion
			matype[3] = isi
			matype[4] = acm/vm
			matype[5] = sis/mis
			matype[6:7]: gfp/gcs/gse/ts
		*/
		dprintk("matype???=%d/%d cycle=%d\n", p_info->iPlsCode, val_0x18, p_info->iVcmCycle);
		dprintk("code_rate=%d\n", p_info->code_rate);
		regmap_read(dev->demod_regmap,0x89,&tmp);
		p_info->is_spectrum_inv =
			((tmp&0x80)!=0)? MtFeSpectrum_Inversion: MtFeSpectrum_Normal;
		dprintk("inversion=%d\n", p_info->is_spectrum_inv);
		regmap_read(dev->demod_regmap,0x76,&tmp);
		tmp &=0x03;
		if(p_info->type == MtFeType_DvbS2X){  //dvbs2x
			switch(tmp){
			case 0 : p_info->roll_off = MtFeRollOff_0p15;break;
			case 1 : p_info->roll_off = MtFeRollOff_0p10;break;
			case 2 : p_info->roll_off = MtFeRollOff_0p05;break;
			default : p_info->roll_off = MtFeRollOff_Undef;break;

			}
		} else {    //dvbs2
			switch(tmp){
			case 0 : p_info->roll_off = MtFeRollOff_0p35;break;
			case 1 : p_info->roll_off = MtFeRollOff_0p25;break;
			case 2 : p_info->roll_off = MtFeRollOff_0p20;break;
			default : p_info->roll_off = MtFeRollOff_Undef;break;

			}
		}
		dprintk("rolloff=%d\n", p_info->roll_off);
	} else {
		dprintk("delsys=DVBS1\n");
		p_info->type = MtFeType_DvbS;
		p_info->mod_mode = MtFeModMode_Qpsk;

		regmap_read(dev->demod_regmap,0xe6,&tmp);
		tmp= (u8)tmp>>5;
		switch(tmp){
			case 0 : p_info->code_rate = MtFeCodeRate_7_8;break;
			case 1 : p_info->code_rate = MtFeCodeRate_5_6;break;
			case 2 : p_info->code_rate = MtFeCodeRate_3_4;break;
			case 3 : p_info->code_rate = MtFeCodeRate_2_3;break;
			case 4 : p_info->code_rate = MtFeCodeRate_1_2;break;
			default : p_info->code_rate = MtFeCodeRate_Undef;break;
		}
		dprintk("code_rate=%d\n", p_info->code_rate);
		p_info->is_pilot_on = false;
		regmap_read(dev->demod_regmap,0xe0,&tmp);
		p_info->is_spectrum_inv =
			((tmp&0x40)!=0)? MtFeSpectrum_Inversion: MtFeSpectrum_Normal;
		dprintk("inversion=%d\n", p_info->is_spectrum_inv);
		p_info->roll_off = MtFeRollOff_0p35;
		dprintk("roll_off=%d\n", p_info->roll_off);
		p_info->is_dummy_frame = false;
		p_info->iVcmCycle = -1;
		p_info->iPlsCode = 0;
		p_info->iFrameLength = 0;
	}

	return 0;
}


static void m88rs6060_get_total_carrier_offset(struct m88rs6060_state* state, s32 *carrier_offset_KHz)
{
	u32	tmp, val_0x5e, val_0x5f;
	s32	val_0x5e_0x5f_1, val_0x5e_0x5f_2, nval_1, nval_2;

	regmap_read(state->demod_regmap, 0x5d, &tmp); //?? trigger frequency offset measurement?
	tmp &= 0xf8;                             //lower 3bits set to zero
	regmap_write(state->demod_regmap, 0x5d, tmp); //?? trigger frequency offset measurement?

	regmap_read(state->demod_regmap, 0x5e, &val_0x5e); //frequency offset msb
	regmap_read(state->demod_regmap, 0x5f, &val_0x5f); //frequency offset lsb
	val_0x5e_0x5f_1 = (val_0x5f << 8) | val_0x5e;

	tmp |= 0x06;                             //lower 3bits set to 6
	regmap_write(state->demod_regmap, 0x5d, tmp);

	regmap_read(state->demod_regmap, 0x5e, &val_0x5e); //2nd frequency offset msb
	regmap_read(state->demod_regmap, 0x5f, &val_0x5f); //2nd frequency offset lsb
	val_0x5e_0x5f_2 = (val_0x5f << 8) | val_0x5e;


	if(((val_0x5e_0x5f_1 >> 15) & 0x01) == 0x01)
		nval_1 = val_0x5e_0x5f_1 - (1 << 16);
	else
		nval_1 = val_0x5e_0x5f_1;

	if(((val_0x5e_0x5f_2 >> 15) & 0x01) == 0x01)
		nval_2 = val_0x5e_0x5f_2 - (1 << 16);
	else
		nval_2 = val_0x5e_0x5f_2;

	//nval_2 is a correction
	*carrier_offset_KHz = (nval_1 - nval_2) * state->mclk / (1 << 16);
	dprintk("Carrier offset =%dkHz %dkHz freq=%d => %d\n", 	(nval_1) * state->mclk / (1 << 16),
					*carrier_offset_KHz, state->frequency, state->frequency+ *carrier_offset_KHz);
}

/*
	 retrieve information about modulation, frequency, symbol_rate
	 CNR, BER

	 returns 1 if tuner frequency or bandwidth needs to be updated
*/
static bool m88rs6060_get_signal_info(struct dvb_frontend *fe)
{
	bool need_retune = false;
	struct dtv_frontend_properties* p = &fe->dtv_property_cache;
	struct m88rs6060_state* state = fe->demodulator_priv;
	u8 regs[2];
	u32 bandwidth_hz;
	s32 carrier_frequency_offset;
	u8 rolloff_status, rolloff;
	struct MT_FE_CHAN_INFO_DVBS2 info;
	s32 carrier_offset_KHz;
	m88rs6060_get_channel_info(state, &info);
	m88rs6060_get_total_carrier_offset(state, &carrier_offset_KHz);
#ifdef TODO
	read_regs(state, RSTV0910_P2_CFR2, regs, 2);
	carrier_frequency_offset = (s16)((regs[0]<<8) | regs[1]);
	carrier_frequency_offset *= ((state->base->mclk>>16) / 1000);
	state->signal_info.frequency = state->tuner_frequency + carrier_frequency_offset;
	dprintk("freq: %d/%d/%d offset %d", p->frequency,
					state->tuner_frequency,
					state->signal_info.frequency, carrier_frequency_offset);
	if (carrier_frequency_offset > 1000 || carrier_frequency_offset< -1000)
		need_retune = true;

	stv091x_symbol_rate(state, &state->symbol_rate, false);
	p->symbol_rate = state->symbol_rate;
	p->frequency = state->signal_info.frequency;

	stv091x_get_standard(fe);

	p->delivery_system = state->signal_info.standard;

	regs[0] = read_reg(state, RSTV0910_P2_TMGOBS);
	rolloff_status = (regs[0]>>6);
	switch(rolloff_status) {
		case 0x03:
			rolloff = 115;
			break;
		case 0x02:
			rolloff = 120;
			break;
		case 0x01:
			rolloff = 125;
			break;
		case 0x00:
		default:
			rolloff = 135;
			break;
		}
	dprintk("mis_mode=%d isi=%d pls_mode=%d pls_code=%d\n",
					state->mis_mode, state->signal_info.isi, state->signal_info.pls_mode,
					state->signal_info.pls_code);
	p->stream_id = ((state->mis_mode ? (state->signal_info.isi &0xff) :0xff) |
									(state->signal_info.pls_mode << 26) |
									((state->signal_info.pls_code &0x3FFFF)<<8)
									);

		bandwidth_hz = (state->symbol_rate * rolloff) / 100;

		if (state->tuner_bw > bandwidth_hz || state->tuner_bw < (bandwidth_hz*8)/10)
			need_retune = true; // ask tuner to change its bandwidth, except duting blind scan

		dprintk("SR: %d, BW: %d => %d need_retune=%d\n", state->symbol_rate, state->tuner_bw, bandwidth_hz, need_retune);
		stv091x_isi_scan(fe);
		return need_retune;
#endif
		return need_retune;
}

static int m88rs6060_tune_once(struct dvb_frontend *fe)
{
	struct m88rs6060_state* state = fe->demodulator_priv;
	struct i2c_client *client = state->demod_client;
	struct dtv_frontend_properties* p = &fe->dtv_property_cache;
	int ret;
	u32 symbol_rate_KSs;
	unsigned tmp, tmp1;
	static const struct reg_sequence reset_buf[] = {
		{0x7, 0x80}, {0x7, 0x0}

	};
	u32 realFreq, freq_MHz;
	s16 lpf_offset_KHz = 0;
	u32 target_mclk = 144000;
	u32 u32tmp;
	s32 s32tmp;
	u32 pls_mode, pls_code;
	u8 pls[] = { 0x1, 0, 0 }, isi;
	u8 buf[2];
	int i = 0, j = 0;
	unsigned tsid[16];
	bool mis = false;

	dprintk("[%d] delivery_system=%u modulation=%u frequency=%u symbol_rate=%u inversion=%u stream_id=%d\n",
					state->adapterno,
					p->delivery_system, p->modulation, p->frequency,
					p->symbol_rate, p->inversion, p->stream_id);

	state->has_timedout=false;

	symbol_rate_KSs = p->symbol_rate / 1000;
	realFreq = p->frequency;
	/*reset */
	ret = regmap_multi_reg_write(state->demod_regmap, reset_buf, 2);
	if (ret)
		goto err;

	regmap_write(state->demod_regmap, 0x7, 0x80);
	regmap_write(state->demod_regmap, 0x7, 0x00);
	msleep(2);
	/*clear ts */
	regmap_write(state->demod_regmap, 0xf5, 0x00);
	msleep(2);
	regmap_read(state->demod_regmap, 0xb2, &tmp);
	if (tmp == 0x01) {
		regmap_write(state->demod_regmap, 0x00, 0x0);
		regmap_write(state->demod_regmap, 0xb2, 0x0);
	}
	if (p->symbol_rate < 5000000) {
		lpf_offset_KHz = 3000;
		realFreq = p->frequency + 3000;
	}

	regmap_write(state->demod_regmap, 0x6, 0xe0);
	rs6060_select_mclk(state, realFreq / 1000, symbol_rate_KSs);
	if (state->mclk == 93000)
		regmap_write(state->demod_regmap, 0xa0, 0x42);
	else if (state->mclk == 96000)
		regmap_write(state->demod_regmap, 0xa0, 0x44);
	else if (state->mclk == 99000)
		regmap_write(state->demod_regmap, 0xa0, 0x46);
	else if (state->mclk == 110250)
		regmap_write(state->demod_regmap, 0xa0, 0x4e);
	else
		regmap_write(state->demod_regmap, 0xa0, 0x44);

	rs6060_set_ts_mclk(state, target_mclk);
	regmap_write(state->demod_regmap, 0x6, 0x0);
	msleep(10);

	//lower the  AGC value?
	rs6060_set_tuner_reg(state, 0x5b, 0x4c);
	rs6060_set_tuner_reg(state, 0x5c, 0x54);
	rs6060_set_tuner_reg(state, 0x60, 0x4b);

	//set tuner pll accordding to desired frequency
	freq_MHz = (realFreq + 500) / 1000;
	state->frequency = freq_MHz*1000;
	ret = rs6060_set_pll_freq(state, freq_MHz);
	if (ret)
		goto err;
	ret = rs6060_set_bb(state, symbol_rate_KSs, lpf_offset_KHz);

	if (ret)
		goto err;

	rs6060_set_tuner_reg(state, 0x00, 0x1);
	rs6060_set_tuner_reg(state, 0x00, 0x0);

	regmap_write(state->demod_regmap, 0xb2, 0x1);
	regmap_write(state->demod_regmap, 0x00, 0x0);

	for (i = 0; i < (sizeof(rs6060_reg_tbl_def) / 2); i++)
		ret =
		    regmap_write(state->demod_regmap, rs6060_reg_tbl_def[i].reg,
				 rs6060_reg_tbl_def[i].val);

	if (ret)
		goto err;

	if ((symbol_rate_KSs > 47100) && (symbol_rate_KSs < 47500)) {
		regmap_write(state->demod_regmap, 0xe6, 0x00);
		regmap_write(state->demod_regmap, 0xe7, 0x03);
	}

	regmap_read(state->demod_regmap, 0x4d, &tmp);
	tmp &= (~0x2);
	regmap_write(state->demod_regmap, 0x4d, tmp);

	regmap_read(state->demod_regmap, 0x08, &tmp);
	regmap_write(state->demod_regmap, 0x08, tmp & 0x7f);

	regmap_read(state->demod_regmap, 0xc9, &tmp);
	regmap_write(state->demod_regmap, 0xc9, tmp | 0x8);

	if (symbol_rate_KSs <= 3000)
		tmp = 0x20;
	else if (symbol_rate_KSs <= 10000)
		tmp = 0x10;
	else
		tmp = 0x6;

	regmap_write(state->demod_regmap, 0xc3, 0x8);
	regmap_write(state->demod_regmap, 0xc8, tmp);
	regmap_write(state->demod_regmap, 0xc4, 0x8);
	regmap_write(state->demod_regmap, 0xc7, 0x0);

	u32tmp = ((symbol_rate_KSs << 15) + (state->mclk / 4)) / (state->mclk / 2);

	regmap_write(state->demod_regmap, 0x61, (u8) (u32tmp & 0xff));
	regmap_write(state->demod_regmap, 0x62, (u8) ((u32tmp >> 8) & 0xff));

	regmap_read(state->demod_regmap, 0x76, &tmp);
	regmap_write(state->demod_regmap, 0x76, 0x30);

	regmap_write(state->demod_regmap, 0x22, pls[0]);
	regmap_write(state->demod_regmap, 0x23, pls[1]);
	regmap_write(state->demod_regmap, 0x24, pls[2]);

	regmap_read(state->demod_regmap, 0x08, &tmp);
	switch (p->delivery_system) {
	case SYS_DVBS:
		tmp = (tmp & 0xfb) | 0x40;
		regmap_write(state->demod_regmap, 0x08, tmp);
		regmap_write(state->demod_regmap, 0xe0, 0xf8);
		break;
	case SYS_DVBS2:
		tmp |= 0x44;
		regmap_write(state->demod_regmap, 0x08, tmp);
		break;
	default:
		tmp &= 0xbb;
		regmap_write(state->demod_regmap, 0x08, tmp);
		regmap_write(state->demod_regmap, 0xe0, 0xf8);
	}

//	regmap_write(state->demod_regmap, 0x08, 3);
//	regmap_write(state->demod_regmap, 0xe0, 0xf8);

	s32tmp = 0x10000 * lpf_offset_KHz;
	s32tmp = (2 * s32tmp + state->mclk) / (2 * state->mclk);
	buf[0] = (s32tmp >> 0) & 0xff;
	buf[1] = (s32tmp >> 8) & 0xff;
	ret = regmap_bulk_write(state->demod_regmap, 0x5e, buf, 2);
	if (ret)
		goto err;

	ret = regmap_write(state->demod_regmap, 0x00, 0x00);
	if (ret)
		goto err;

	ret = regmap_write(state->demod_regmap, 0xb2, 0x00);
	if (ret)
		goto err;


//	ret = regmap_write(state->demod_regmap, 0xfe, 0xe1);
//	if (ret)
//		goto err;

//	ret = regmap_write(state->demod_regmap, 0xea, 0x4);
	//if (ret)
//		goto err;
	if (p->stream_id != NO_STREAM_ID_FILTER) {
		isi = p->stream_id & 0xff;
		pls_mode = (p->stream_id >> 26) & 3;
		pls_code = (p->stream_id >> 8) & 0x3ffff;
		if (!pls_mode && !pls_code)
			pls_code = 1;
	} else {
		isi = 0;
		pls_mode = 0;
		pls_code = 1;
	}

	if (p->scrambling_sequence_index) {
		pls_mode = 1;
		pls_code = p->scrambling_sequence_index;
	}

	if (pls_mode)
		m88rs6060_calc_PLS_gold_code(pls, pls_code);
	else {
		pls[0] = pls_code & 0xff;
		pls[1] = (pls_code >> 8) & 0xff;
		pls[2] = (pls_code >> 16) & 3;
	}

	dev_dbg(&client->dev, "isi = %d", isi);
	dev_dbg(&client->dev, "pls mode %d, code %d\n", pls_mode, pls_code);
	dev_dbg(&client->dev, "pls buf =%*ph \n", 3, pls);

	ret = regmap_bulk_write(state->demod_regmap, 0x22, pls, 3);

	//adjust agc
	rs6060_set_tuner_reg(state, 0x5b, 0xcc);
	rs6060_set_tuner_reg(state, 0x5c, 0xf4);
	rs6060_set_tuner_reg(state, 0x60, 0xcb);

	state->demod_locked = false;
	state->has_timedout = false;
	//wait_for_dmdlock
	for (i = 0; i < 150; i++) {
		regmap_read(state->demod_regmap, 0x8, &tmp);
		regmap_read(state->demod_regmap, 0xd, &tmp1); //get all lock bits
		state->fec_locked = (tmp1 & 0x80);
		state->has_viterbi = (tmp1 & 0x80);
		state->has_sync = (tmp1 & 0x08) && //frequency locked
			(tmp1 & 0x04); //timing locked

		if ((tmp1 == 0x8f) //fully locked dvbs2
				|| (tmp1 == 0xf7) //fully locked dvbs1
				) {
			state->demod_locked = true;
			break;
		}
		msleep(20);
	}
	state->has_lock = state->demod_locked;
	state->has_timedout = (i==150) && !state->has_viterbi;
	/*perform some actions after dvbs2 lock
		something to do with spectral inversion?
		set isi
	*/
	if (tmp1 == 0x8f) { //locked dvbs2
		regmap_read(state->demod_regmap, 0x89, &tmp); //read spectral inversion
		regmap_read(state->demod_regmap, 0xca, &tmp1);
		tmp &= 0x80; //spectrum inverted = tmp!=0
		regmap_write(state->demod_regmap, 0xca,
			     (u8) ((tmp1 & 0xf7) | (tmp >> 4) | 0x02));
		regmap_write(state->demod_regmap, 0xfa, 0x00);
		regmap_write(state->demod_regmap, 0xf0, 0x00);
		regmap_write(state->demod_regmap, 0xf0, 0x03);

		msleep(20);
		regmap_read(state->demod_regmap, 0xf1, &tmp1);
		tmp1 &= 0x1f;
		dev_dbg(&client->dev, "ISI cnt = %d \n", tmp1);
		for (j = 0; j < tmp1; j++) {
			regmap_write(state->demod_regmap, 0xf2, j); //write the index of an ISI slot to read out
			regmap_read(state->demod_regmap, 0xf3, &tsid[j]); //read ISI at slot
			dev_dbg(&client->dev, "ISI = %d \n", tsid[j]);
		}
		for (j = 0; j < tmp1; j++)
			if (tsid[j] == isi) {
				mis = true;
				break;
			}

		if (mis)
			regmap_write(state->demod_regmap, 0xf5, isi);
		else
			regmap_write(state->demod_regmap, 0xf5, tsid[0]);
	}

	state->TsClockChecked = true;

	if(state->config.HAS_CI)
			state->newTP = true;
	if(state->has_lock) {
		/*
			retrieve information about modulation, frequency, symbol_rate
			but not CNR, BER (different from stid135)

			returns 1 if tuner frequency or bandwidth needs to be updated
		*/

		m88rs6060_get_signal_info(fe);
	}

		vprintk("[%d] locked=%d vit=%d sync=%d timeout=%d\n",
						state->adapterno, state->has_lock, state->has_viterbi, state->has_sync, state->has_timedout);
	return 0;

 err:
	dev_dbg(&client->dev, "failed = %d", ret);
	return ret;
}

static int m88rs6060_init(struct dvb_frontend *fe)
{
	struct m88rs6060_state *dev = fe->demodulator_priv;
	struct i2c_client *client = dev->demod_client;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;

	c->cnr.len = 1;
	c->cnr.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
	c->post_bit_error.len = 1;
	c->post_bit_error.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
	c->post_bit_count.len = 1;
	c->post_bit_count.stat[0].scale = FE_SCALE_NOT_AVAILABLE;

	dev_info(&client->dev, "%s finished\n", __func__);

	return 0;

}

static int m88rs6060_get_sym_rate(struct m88rs6060_state *dev,
						u32 *sym_rate_KSs)
{
	u16 tmp;
	u32 sym_rate_tmp;
	unsigned val_0x6d,val_0x6e;

	regmap_read(dev->demod_regmap,0x6d,&val_0x6d);
	regmap_read(dev->demod_regmap,0x6e,&val_0x6e);

	tmp = (u16)((val_0x6e<<8)|val_0x6d);
	sym_rate_tmp = tmp*dev->mclk;
	sym_rate_tmp /= (1<<16);
	*sym_rate_KSs = sym_rate_tmp;

	return 0;
}




static int rs6060_select_xm(struct m88rs6060_state* state, u32 *xm_KHz)
{
	u32 symbol_rate = 0;
	u8 reg16;
	u32 offset_KHz[8] = {0};
	u32 max_offset = 0;
	u8 i, xm_line, xm_cnt = 5;
	u32 xm_list_KHz[3][8] = {
							{96000, 102400, 107162, 109714, 115200, 128000, 135529, 144000},
							{93000,  99200, 111600, 117473, 124000, 139500, 144000, 148800},
							{99000, 105600, 108000, 110511, 118800, 132000, 144000, 148500}
						   };



	m88rs6060_get_sym_rate(state, &symbol_rate);
	xm_cnt = sizeof(xm_list_KHz) / sizeof(u32);
	xm_cnt /= 3;
	// C = (symbol * 1.35 / 2 + 2) * 1.1;
	symbol_rate *= 135;
	symbol_rate /= 200;
	symbol_rate += 2000;
	symbol_rate *= 110;
	symbol_rate /= 100;

	reg16 = rs6060_get_tuner_reg(state, 0x16);

	if(reg16 == 92){
		xm_line = 1;
	}
	else if(reg16 == 100){
		xm_line = 2;
	}
	else{//if(reg16 == 96)
		xm_line = 0;
	}

	for(i = 0; i < xm_cnt; i ++){
		if(*xm_KHz > xm_list_KHz[xm_line][i])
		{
			continue;
		}
		offset_KHz[i] = (state->frequency % xm_list_KHz[xm_line][i]);
		if(offset_KHz[i] > (xm_list_KHz[xm_line][i] / 2))
			offset_KHz[i] = xm_list_KHz[xm_line][i] - offset_KHz[i];
		if(offset_KHz[i] > symbol_rate)
		{
			*xm_KHz = xm_list_KHz[xm_line][i];
			break;
		}
		if(offset_KHz[i] > max_offset)
		{
			max_offset = offset_KHz[i];
			*xm_KHz = xm_list_KHz[xm_line][i];
		}
	 }

	if(i == xm_cnt)
	{
		*xm_KHz = xm_list_KHz[xm_line][xm_cnt - 1];
	}

	return 0;
}

static int m88rs6060_set_clock_ratio(struct m88rs6060_state*dev )
{
	unsigned mod_fac,tmp1,tmp2,val;
	u32 input_datarate,locked_sym_rate_KSs;
	u32 Mclk_KHz = 96000,iSerialMclkHz;
	u16 divid_ratio = 0;
	struct MT_FE_CHAN_INFO_DVBS2 p_info;
	dprintk("Called\n");
	m88rs6060_get_sym_rate(dev,&locked_sym_rate_KSs);

	regmap_read(dev->demod_regmap,0x9d,&val);
	regmap_write(dev->demod_regmap,0x9d,val|0x08);

	m88rs6060_get_channel_info(dev,&p_info);

	if(p_info.type==MtFeType_DvbS2){

       switch(p_info.mod_mode)
        {
            case MtFeModMode_8psk:
            case MtFeModMode_8Apsk_L:       mod_fac = 3; break;
            case MtFeModMode_16Apsk:
            case MtFeModMode_16Apsk_L:      mod_fac = 4; break;
            case MtFeModMode_32Apsk:
            case MtFeModMode_32Apsk_L:      mod_fac = 5; break;
            case MtFeModMode_64Apsk:
            case MtFeModMode_64Apsk_L:      mod_fac = 6; break;
            case MtFeModMode_128Apsk:
            case MtFeModMode_128Apsk_L:     mod_fac = 7; break;
            case MtFeModMode_256Apsk:
            case MtFeModMode_256Apsk_L:     mod_fac = 8; break;
            case MtFeModMode_Qpsk:
            default:                        mod_fac = 2; break;
        }
		switch(p_info.code_rate)
		{
			case MtFeCodeRate_1_4:		input_datarate = locked_sym_rate_KSs*mod_fac/8/4;		break;
			case MtFeCodeRate_1_3:		input_datarate = locked_sym_rate_KSs*mod_fac/8/3;		break;
			case MtFeCodeRate_2_5:		input_datarate = locked_sym_rate_KSs*mod_fac*2/8/5;		break;
			case MtFeCodeRate_1_2:		input_datarate = locked_sym_rate_KSs*mod_fac/8/2;		break;
			case MtFeCodeRate_3_5:		input_datarate = locked_sym_rate_KSs*mod_fac*3/8/5;		break;
			case MtFeCodeRate_2_3:		input_datarate = locked_sym_rate_KSs*mod_fac*2/8/3;		break;
			case MtFeCodeRate_3_4:		input_datarate = locked_sym_rate_KSs*mod_fac*3/8/4;		break;
			case MtFeCodeRate_4_5:		input_datarate = locked_sym_rate_KSs*mod_fac*4/8/5;		break;
			case MtFeCodeRate_5_6:		input_datarate = locked_sym_rate_KSs*mod_fac*5/8/6;		break;
			case MtFeCodeRate_8_9:		input_datarate = locked_sym_rate_KSs*mod_fac*8/8/9;		break;
			case MtFeCodeRate_9_10:		input_datarate = locked_sym_rate_KSs*mod_fac*9/8/10;	break;
			case MtFeCodeRate_5_9:		input_datarate = locked_sym_rate_KSs*mod_fac*5/8/9;		break;
			case MtFeCodeRate_7_9:		input_datarate = locked_sym_rate_KSs*mod_fac*7/8/9;		break;
			case MtFeCodeRate_4_15:		input_datarate = locked_sym_rate_KSs*mod_fac*4/8/15;	break;
			case MtFeCodeRate_7_15:		input_datarate = locked_sym_rate_KSs*mod_fac*7/8/15;	break;
			case MtFeCodeRate_8_15:		input_datarate = locked_sym_rate_KSs*mod_fac*8/8/15;	break;
			case MtFeCodeRate_11_15:	input_datarate = locked_sym_rate_KSs*mod_fac*11/8/15;	break;
			case MtFeCodeRate_13_18:	input_datarate = locked_sym_rate_KSs*mod_fac*13/8/18;	break;
			case MtFeCodeRate_9_20:		input_datarate = locked_sym_rate_KSs*mod_fac*9/8/20;	break;
			case MtFeCodeRate_11_20:	input_datarate = locked_sym_rate_KSs*mod_fac*11/8/20;	break;
			case MtFeCodeRate_23_36:	input_datarate = locked_sym_rate_KSs*mod_fac*23/8/36;	break;
			case MtFeCodeRate_25_36:	input_datarate = locked_sym_rate_KSs*mod_fac*25/8/36;	break;
			case MtFeCodeRate_11_45:	input_datarate = locked_sym_rate_KSs*mod_fac*11/8/45;	break;
			case MtFeCodeRate_13_45:	input_datarate = locked_sym_rate_KSs*mod_fac*13/8/45;	break;
			case MtFeCodeRate_14_45:	input_datarate = locked_sym_rate_KSs*mod_fac*14/8/45;	break;
			case MtFeCodeRate_26_45:	input_datarate = locked_sym_rate_KSs*mod_fac*26/8/45;	break;
			case MtFeCodeRate_28_45:	input_datarate = locked_sym_rate_KSs*mod_fac*28/8/45;	break;
			case MtFeCodeRate_29_45:	input_datarate = locked_sym_rate_KSs*mod_fac*29/8/45;	break;
			case MtFeCodeRate_31_45:	input_datarate = locked_sym_rate_KSs*mod_fac*31/8/45;	break;
			case MtFeCodeRate_32_45:	input_datarate = locked_sym_rate_KSs*mod_fac*32/8/45;	break;
			case MtFeCodeRate_77_90:	input_datarate = locked_sym_rate_KSs*mod_fac*77/8/90;	break;
			default:					input_datarate = locked_sym_rate_KSs*mod_fac*2/8/3;		break;

		}
		rs6060_get_ts_mclk(dev,&Mclk_KHz);
		if(dev->config.ts_mode==MtFeTsOutMode_Serial){
			u32 target_mclk = Mclk_KHz;
			input_datarate*=8;

		//	target_mclk = input_datarate;
				rs6060_select_xm(dev,&target_mclk);
			if(target_mclk != Mclk_KHz){
				regmap_write(dev->demod_regmap,0x06,0xe0);
				rs6060_set_ts_mclk(dev,target_mclk);
				regmap_write(dev->demod_regmap,0x06,0x00);
			}

			rs6060_get_ts_mclk(dev,&iSerialMclkHz);
			if(iSerialMclkHz>116000)
				regmap_write(dev->demod_regmap,0x0a,0x01);
			else
				regmap_write(dev->demod_regmap,0x0a,0x00);


		}else{

			iSerialMclkHz = input_datarate*49/5;
			input_datarate = input_datarate *105/100;
			if(iSerialMclkHz>115200)
				iSerialMclkHz = 144000;
			else if(iSerialMclkHz>96000)
				iSerialMclkHz = 115200;
			else if(iSerialMclkHz>72000)
				iSerialMclkHz = 96000;
			else
				iSerialMclkHz = 72000;
			if(input_datarate<6000)
				input_datarate= 6000;
			if(input_datarate != 0)
				divid_ratio = (u16) (Mclk_KHz/input_datarate);
			else
				divid_ratio = 0xff;
			printk("MClk_KHz = %d,divid_ratio = %d \n",Mclk_KHz,divid_ratio);

			if(divid_ratio<8)
				divid_ratio = 8;
			if(dev->config.ts_mode == MtFeTsOutMode_Common){
				if(divid_ratio>27)
					divid_ratio = 27;
				if((divid_ratio == 14)||(divid_ratio==15))
					divid_ratio = 13;
				if((divid_ratio == 19)||(divid_ratio == 20))
					divid_ratio = 18;
			}else{
				if(divid_ratio>24)
					divid_ratio = 24;
				if((divid_ratio == 12)||(divid_ratio==13))
					divid_ratio = 11;
				if((divid_ratio == 19)||(divid_ratio == 20))
					divid_ratio = 18;
			}
			tmp1 = (u8) ((divid_ratio/2)-1);
			tmp2 = DIV_ROUND_UP(divid_ratio,2)-1;


		tmp1 &= 0x3f;
		tmp2 &= 0x3f;
		val = (tmp1 >>2)&0x0f;
		regmap_update_bits(dev->demod_regmap,0xfe,0x0f,val);
		val = (u8)(((tmp1&0x3)<<6)|tmp2);
		regmap_write(dev->demod_regmap,0xea,val);
		}
	}
	else{    //dvbs
	  mod_fac = 2;

	  switch(p_info.code_rate){
	  	case MtFeCodeRate_1_2:		input_datarate = locked_sym_rate_KSs*mod_fac/2/8;		break;
		case MtFeCodeRate_2_3:		input_datarate = locked_sym_rate_KSs*mod_fac*2/3/8;		break;
		case MtFeCodeRate_3_4:		input_datarate = locked_sym_rate_KSs*mod_fac*3/4/8;		break;
		case MtFeCodeRate_5_6:		input_datarate = locked_sym_rate_KSs*mod_fac*5/6/8;		break;
		case MtFeCodeRate_7_8:		input_datarate = locked_sym_rate_KSs*mod_fac*7/8/8;		break;
		default:		input_datarate = locked_sym_rate_KSs*mod_fac*3/4/8;		break;

	  }
		rs6060_get_ts_mclk(dev,&Mclk_KHz);

		if(dev->config.ts_mode==MtFeTsOutMode_Serial){
			u32 target_mclk = Mclk_KHz;
			input_datarate*=8;

		//	target_mclk = input_datarate;
				rs6060_select_xm(dev,&target_mclk);
			if(target_mclk != Mclk_KHz){
				regmap_write(dev->demod_regmap,0x06,0xe0);
				rs6060_set_ts_mclk(dev,target_mclk);
				regmap_write(dev->demod_regmap,0x06,0x00);
			}

			rs6060_get_ts_mclk(dev,&iSerialMclkHz);
			if(iSerialMclkHz>116000)
				regmap_write(dev->demod_regmap,0x0a,0x01);
			else
				regmap_write(dev->demod_regmap,0x0a,0x00);

		  }else{
				iSerialMclkHz = input_datarate*46/5;
				input_datarate = input_datarate *105/100;

				if(iSerialMclkHz>72000)
					iSerialMclkHz = 96000;
				else
					iSerialMclkHz = 72000;

				if(input_datarate<6000)
					input_datarate = 6000;
				if(input_datarate != 0)
					divid_ratio = (u16)(Mclk_KHz/input_datarate);
				else
					divid_ratio = 0xff;
				if(divid_ratio<8)
					divid_ratio = 8;
				if(dev->config.ts_mode == MtFeTsOutMode_Common){
					if(divid_ratio>27)
						divid_ratio=27;
					}
				else {
					if (divid_ratio>24)
						divid_ratio =24;
					}
				tmp1 = (u8)((divid_ratio/2)-1);
				tmp2 = DIV_ROUND_UP(divid_ratio,2)-1;

				tmp1 &= 0x3f;
				tmp2 &= 0x3f;
				val = (tmp1 >>2)&0x0f;
				regmap_update_bits(dev->demod_regmap,0xfe,0x0f,val);
				val = (u8)(((tmp1&0x3)<<6)|tmp2);
				regmap_write(dev->demod_regmap,0xea,val);
			}
	}
	return 0;
}

static void init_signal_quality(struct dvb_frontend* fe,	struct dtv_frontend_properties *p)
{
	s32 gain;
	struct m88rs6060_state* state = fe->demodulator_priv;
		/*power of rf signal */
	m88rs6060_get_gain(state, p->frequency / 1000, &gain);
	dprintk("Gain =%d\n", gain);
	p->strength.len = 2;
	p->strength.stat[0].scale = FE_SCALE_DECIBEL;
	p->strength.stat[0].svalue = -gain * 10;

	p->strength.stat[1].scale = FE_SCALE_RELATIVE;
	p->strength.stat[1].svalue = (100 + (-gain / 100)) * 656;

	p->cnr.len = p->post_bit_error.len = p->post_bit_count.len = 1;
	p->cnr.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
	p->post_bit_error.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
	p->post_bit_count.stat[0].scale = FE_SCALE_NOT_AVAILABLE;

}

/*
	read rf level, snr, signal quality, lock_status
 */
static int m88rs6060_read_status(struct dvb_frontend *fe, enum fe_status *status)
{
	struct m88rs6060_state* state = fe->demodulator_priv;
	struct i2c_client* client = state->demod_client;
	struct dtv_frontend_properties* p = &fe->dtv_property_cache;
	int ret, i, itmp;
	unsigned int utmp;
	u8 buf[3];
	s32 gain;
	u16 temp;
	dev_dbg(&client->dev, "%s\n", __func__);
	init_signal_quality(fe, p);

	//todo: if not locked, and signal is too low FE_HAS_SIGNAL should be removed
	*status = FE_HAS_SIGNAL;

	switch (p->delivery_system) {
	case SYS_DVBS:
		ret = regmap_read(state->demod_regmap, 0x0d, &utmp);  //get lock bits; bits 4..6 are reserved
		if (ret)
			goto err;

		if ((utmp & 0xf7) == 0xf7)
			*status = FE_HAS_SIGNAL | FE_HAS_CARRIER |
			    FE_HAS_VITERBI | FE_HAS_SYNC | FE_HAS_LOCK;
		break;
	case SYS_DVBS2:
		ret = regmap_read(state->demod_regmap, 0x0d, &utmp); //get lock bits; bits 4..6 are reserved
		if (ret)
			goto err;

		if ((utmp & 0x8f) == 0x8f)
			*status = FE_HAS_SIGNAL | FE_HAS_CARRIER |
			    FE_HAS_VITERBI | FE_HAS_SYNC | FE_HAS_LOCK;
		break;
	default:
		dev_dbg(&client->dev, "invalid delivery_system\n");
		ret = -EINVAL;
		goto err;
	}

	state->fe_status = *status;
	dev_dbg(&client->dev, "lock=%02x status=%02x\n", utmp, *status);

	if ((state->fe_status & FE_HAS_LOCK)&&(state->TsClockChecked)){
		state->TsClockChecked = false;
		m88rs6060_set_clock_ratio(state);
	}

	if((state->config.HAS_CI)&&(state->fe_status & FE_HAS_LOCK)&&(state->newTP))
	{
		u32 clock = 0;
		u32 value = 0;
		int stat = 0;
		u32 speed = 0;
		msleep(50);
		state->config.SetSpeedstatus(client->adapter,state->config.num);
		msleep(50);
		while(!stat){
			stat=state->config.GetSpeedstatus(client->adapter,state->config.num);
			msleep(10);
		}
		speed = state->config.GetSpeed(client->adapter,state->config.num);
		clock = ((speed*4)*204*8/1024)+500; //khz
		if(clock<42000)
			clock = 42000;
		value = (clock/8*204/188*25000/6)+500;
		si5351_set_freq(state,value,0,SI5351_CLK0);
		state->newTP = false;
	}

	//get signal to noise
	p->cnr.len = 1;
	p->cnr.stat[0].scale = FE_SCALE_NOT_AVAILABLE;

	/* CNR */
	if (state->fe_status & FE_HAS_VITERBI) {
		unsigned int cnr, noise, signal, noise_tot, signal_tot;

		cnr = 0;
		/* more iterations for more accurate estimation */
#define M88rs6060_SNR_ITERATIONS 10

		switch (p->delivery_system) {
		case SYS_DVBS:
			itmp = 0;

			for (i = 0; i < M88rs6060_SNR_ITERATIONS; i++) {
				ret = regmap_read(state->demod_regmap, 0xff, &utmp);
				if (ret)
					goto err;

				itmp += utmp;
			}
			temp = (u16) (itmp / 80);
			if (temp > 32)
				temp = 32;
			if (temp > 1)
				cnr = (mes_loge[temp - 1] * 10) / 23;
			else
				cnr = 0;
			dprintk("temp=%d cnr=%d\n", temp, cnr);
			break;
		case SYS_DVBS2:
			noise_tot = 0;
			signal_tot = 0;

			for (i = 0; i < M88rs6060_SNR_ITERATIONS; i++) {
				ret =
				    regmap_bulk_read(state->demod_regmap, 0x8c, buf, 3);
				if (ret)
					goto err;

				noise = buf[1] << 6;	/* [13:6] */
				noise |= buf[0] & 0x3f;	/*  [5:0] */
				noise >>= 2;
				signal = buf[2] * buf[2];
				signal >>= 1;

				noise_tot += noise;
				signal_tot += signal;
			}

			noise = noise_tot / M88rs6060_SNR_ITERATIONS;
			signal = signal_tot / M88rs6060_SNR_ITERATIONS;
			if (signal == 0)
				cnr = 0;
			else if (noise == 0)
				cnr = 19;
			else if (signal > noise) {
				itmp = signal / noise;
				if (itmp > 80)
					itmp = 80;
				cnr = mes_log10[itmp - 1];
			} else if (signal < noise) {
				itmp = noise / signal;
				if (itmp > 80)
					itmp = 80;
				cnr = mes_log10[itmp - 1];
			}
			dprintk("itmp=%d signal=%d noise=%d cnr=%d\n", itmp, signal, noise, cnr);
			break;
		default:
			dev_dbg(&client->dev, "invalid delivery_system\n");
			ret = -EINVAL;
			goto err;
		}

		p->cnr.len = 2;
		p->cnr.stat[0].scale = FE_SCALE_DECIBEL;
		p->cnr.stat[0].svalue = cnr;
		p->cnr.stat[1].scale = FE_SCALE_RELATIVE;
		p->cnr.stat[1].uvalue = (cnr/100) * 328;
		if (p->cnr.stat[1].uvalue > 0xffff)
			p->cnr.stat[1].uvalue = 0xffff;
	}

	/* BER */
	if (state->fe_status & FE_HAS_LOCK) {
		unsigned int utmp, post_bit_error, post_bit_count;

		switch (p->delivery_system) {
		case SYS_DVBS:
			//deepthought: to check -> confusion with dvbs2?
			ret = regmap_read(state->demod_regmap, 0xd5, &utmp);
			if (ret)
				goto err;

			/* measurement ready? */
			if (!(utmp & 0x10)) {
				ret =
				    regmap_bulk_read(state->demod_regmap, 0xd6, buf, 2);
				if (ret)
					goto err;

				post_bit_error = buf[1] << 8 | buf[0] << 0;
				post_bit_count = 0x800000;
				state->post_bit_error += post_bit_error;
				state->post_bit_count += post_bit_count;
				state->dvbv3_ber = post_bit_error;

				/* restart measurement */
				ret = regmap_write(state->demod_regmap, 0xd5, 0x82);
				if (ret)
					goto err;
			}
			break;
		case SYS_DVBS2:
			ret = regmap_bulk_read(state->demod_regmap, 0xd5, buf, 3);
			if (ret)
				goto err;
			//dev_info(&client->dev,"buf =%*ph\n",3,buf);
			utmp = buf[2] << 16 | buf[1] << 8 | buf[0] << 0;

			/* enough data? */
			if (utmp > 4000) {
				ret =
				    regmap_bulk_read(state->demod_regmap, 0xf7, buf, 2);
				if (ret)
					goto err;

				post_bit_error = buf[1] << 8 | buf[0] << 0;
				post_bit_count = 32 * utmp;	/* TODO: FEC */
				state->post_bit_error += post_bit_error;
				state->post_bit_count += post_bit_count;
				state->dvbv3_ber = post_bit_error;

				/* restart measurement */
				ret = regmap_write(state->demod_regmap, 0xd1, 0x01);
				if (ret)
					goto err;

				ret = regmap_write(state->demod_regmap, 0xf9, 0x01);
				if (ret)
					goto err;

				ret = regmap_write(state->demod_regmap, 0xf9, 0x00);
				if (ret)
					goto err;

				ret = regmap_write(state->demod_regmap, 0xd1, 0x00);
				if (ret)
					goto err;
			}
			break;
		default:
			dev_dbg(&client->dev, "invalid delivery_system\n");
			ret = -EINVAL;
			goto err;
		}

		p->post_bit_error.stat[0].scale = FE_SCALE_COUNTER;
		p->post_bit_error.stat[0].uvalue = state->post_bit_error;

		p->post_bit_count.stat[0].scale = FE_SCALE_COUNTER;
		p->post_bit_count.stat[0].uvalue = state->post_bit_count;
	} else {
		p->post_bit_error.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
		p->post_bit_count.stat[0].scale = FE_SCALE_NOT_AVAILABLE;
	}

	p->post_bit_error.len = 1;
	p->post_bit_count.len = 1;



	return 0;
 err:
	dev_dbg(&client->dev, "failed=%d\n", ret);
	return ret;
}

static int m88rs6060_read_snr(struct dvb_frontend *fe, u16 * snr)
{
	struct dtv_frontend_properties *p = &fe->dtv_property_cache;
	int i;

	*snr = 0;
	for (i=0; i < p->cnr.len; i++)
		if (p->cnr.stat[i].scale == FE_SCALE_RELATIVE)
		  *snr = (u16)p->cnr.stat[i].uvalue;

	return 0;
}

static int m88rs6060_read_ber(struct dvb_frontend *fe, u32 * ber)
{
	struct dtv_frontend_properties *p = &fe->dtv_property_cache;

	if ( p->post_bit_error.stat[0].scale == FE_SCALE_COUNTER &&
		p->post_bit_count.stat[0].scale == FE_SCALE_COUNTER )
		*ber = (u32)p->post_bit_count.stat[0].uvalue ? (u32)p->post_bit_error.stat[0].uvalue /
					(u32)p->post_bit_count.stat[0].uvalue : 0;

	return 0;
}

static int m88rs6060_read_signal_strength(struct dvb_frontend *fe,
					  u16 * strength)
{
	struct dtv_frontend_properties *p = &fe->dtv_property_cache;
	int i;

	*strength = 0;
	for (i=0; i < p->strength.len; i++)
		if (p->strength.stat[i].scale == FE_SCALE_RELATIVE)
			*strength = (u16)p->strength.stat[i].uvalue;

	return 0;
}

static int m88rs6060_set_voltage(struct dvb_frontend*fe,
								enum fe_sec_voltage voltage)
{
	struct m88rs6060_state *dev = fe->demodulator_priv;
	struct i2c_client *client = dev->demod_client;
	int ret;
	u8 utmp;
	bool voltage_sel, lnb_power;


	switch(voltage){
		case SEC_VOLTAGE_18:
			voltage_sel = 1;
			lnb_power = 1;
			break;
		case SEC_VOLTAGE_13:
			voltage_sel = 0;
			lnb_power = 1;
			break;
		case SEC_VOLTAGE_OFF:
			voltage_sel = 0;
			lnb_power = 0;
			break;
	}
	utmp = lnb_power << 1 | voltage_sel << 0;
	ret = m88rs6060_update_bits(dev, 0xa2, 0x03, utmp);
	if (ret)
		goto err;

	return 0;
err:
	dev_dbg(&client->dev, "failed=%d\n", ret);
	return ret;

}

static int m88rs6060_set_tone(struct dvb_frontend *fe,
			      enum fe_sec_tone_mode fe_sec_tone_mode)
{
	struct m88rs6060_state *dev = fe->demodulator_priv;
	struct i2c_client *client = dev->demod_client;
	int ret;
	unsigned int utmp, tone, reg_a1_mask;

	dev_dbg(&client->dev, "fe_sec_tone_mode=%d\n", fe_sec_tone_mode);

	switch (fe_sec_tone_mode) {
	case SEC_TONE_ON:
		tone = 0;
		reg_a1_mask = 0x47;
		break;
	case SEC_TONE_OFF:
		tone = 1;
		reg_a1_mask = 0x00;
		break;
	default:
		dev_dbg(&client->dev, "invalid fe_sec_tone_mode\n");
		ret = -EINVAL;
		goto err;
	}

	utmp = tone << 7 | dev->config.envelope_mode << 5;
	ret = m88rs6060_update_bits(dev, 0xa2, 0xe0, utmp);
	if (ret)
		goto err;

	utmp = 1 << 2;
	ret = m88rs6060_update_bits(dev, 0xa1, reg_a1_mask, utmp);
	if (ret)
		goto err;

	return 0;
 err:
	dev_dbg(&client->dev, "failed=%d\n", ret);
	return ret;
}

static int m88rs6060_diseqc_send_master_cmd(struct dvb_frontend *fe, struct dvb_diseqc_master_cmd
					    *diseqc_cmd)
{
	struct m88rs6060_state *dev = fe->demodulator_priv;
	struct i2c_client *client = dev->demod_client;
	int ret;
	unsigned int utmp;
	unsigned long timeout;

	dev_dbg(&client->dev, "msg=%*ph\n",
		diseqc_cmd->msg_len, diseqc_cmd->msg);

	if (diseqc_cmd->msg_len < 3 || diseqc_cmd->msg_len > 6) {
		ret = -EINVAL;
		goto err;
	}

	utmp = dev->config.envelope_mode << 5;
	ret = m88rs6060_update_bits(dev, 0xa2, 0xe0, utmp);
	if (ret)
		goto err;

	ret = regmap_bulk_write(dev->demod_regmap, 0xa3, diseqc_cmd->msg,
				diseqc_cmd->msg_len);
	if (ret)
		goto err;

	ret = regmap_write(dev->demod_regmap, 0xa1,
			   (diseqc_cmd->msg_len - 1) << 3 | 0x07);
	if (ret)
		goto err;

	/* wait DiSEqC TX ready */
#define SEND_MASTER_CMD_TIMEOUT 120
	timeout = jiffies + msecs_to_jiffies(SEND_MASTER_CMD_TIMEOUT);

	/* DiSEqC message period is 13.5 ms per byte */
	utmp = diseqc_cmd->msg_len * 13500;
	usleep_range(utmp - 4000, utmp);

	for (utmp = 1; !time_after(jiffies, timeout) && utmp;) {
		ret = regmap_read(dev->demod_regmap, 0xa1, &utmp);
		if (ret)
			goto err;
		utmp = (utmp >> 6) & 0x1;
	}

	if (utmp == 0) {
		dev_dbg(&client->dev, "diseqc tx took %u ms\n",
			jiffies_to_msecs(jiffies) -
			(jiffies_to_msecs(timeout) - SEND_MASTER_CMD_TIMEOUT));
	} else {
		dev_dbg(&client->dev, "diseqc tx timeout\n");

		ret = m88rs6060_update_bits(dev, 0xa1, 0xc0, 0x40);
		if (ret)
			goto err;
	}

	ret = m88rs6060_update_bits(dev, 0xa2, 0xc0, 0x80);
	if (ret)
		goto err;

	if (utmp == 1) {
		ret = -ETIMEDOUT;
		goto err;
	}

	return 0;
 err:
	dev_dbg(&client->dev, "failed=%d\n", ret);
	return ret;
}

static int m88rs6060_diseqc_send_burst(struct dvb_frontend *fe,
				       enum fe_sec_mini_cmd fe_sec_mini_cmd)
{
	struct m88rs6060_state *dev = fe->demodulator_priv;
	struct i2c_client *client = dev->demod_client;
	int ret;
	unsigned int utmp, burst;
	unsigned long timeout;

	dev_dbg(&client->dev, "fe_sec_mini_cmd=%d\n", fe_sec_mini_cmd);

	utmp = dev->config.envelope_mode << 5;
	ret = m88rs6060_update_bits(dev, 0xa2, 0xe0, utmp);
	if (ret)
		goto err;

	switch (fe_sec_mini_cmd) {
	case SEC_MINI_A:
		burst = 0x02;
		break;
	case SEC_MINI_B:
		burst = 0x01;
		break;
	default:
		dev_dbg(&client->dev, "invalid fe_sec_mini_cmd\n");
		ret = -EINVAL;
		goto err;
	}

	ret = regmap_write(dev->demod_regmap, 0xa1, burst);
	if (ret)
		goto err;

	/* wait DiSEqC TX ready */
#define SEND_BURST_TIMEOUT 40
	timeout = jiffies + msecs_to_jiffies(SEND_BURST_TIMEOUT);

	/* DiSEqC ToneBurst period is 12.5 ms */
	usleep_range(8500, 12500);

	for (utmp = 1; !time_after(jiffies, timeout) && utmp;) {
		ret = regmap_read(dev->demod_regmap, 0xa1, &utmp);
		if (ret)
			goto err;
		utmp = (utmp >> 6) & 0x1;
	}

	if (utmp == 0) {
		dev_dbg(&client->dev, "diseqc tx took %u ms\n",
			jiffies_to_msecs(jiffies) -
			(jiffies_to_msecs(timeout) - SEND_BURST_TIMEOUT));
	} else {
		dev_dbg(&client->dev, "diseqc tx timeout\n");

		ret = m88rs6060_update_bits(dev, 0xa1, 0xc0, 0x40);
		if (ret)
			goto err;
	}

	ret = m88rs6060_update_bits(dev, 0xa2, 0xc0, 0x80);
	if (ret)
		goto err;

	if (utmp == 1) {
		ret = -ETIMEDOUT;
		goto err;
	}

	return 0;
 err:
	dev_dbg(&client->dev, "failed=%d\n", ret);
	return ret;
}

static void m88rs6060_spi_read(struct dvb_frontend *fe,
			       struct ecp3_info *ecp3inf)
{
	struct m88rs6060_state *dev = fe->demodulator_priv;
	struct i2c_client *client = dev->demod_client;

	if (dev->read_properties)
		dev->read_properties(client->adapter, ecp3inf->reg,
				     &(ecp3inf->data));

	return;
}

static void m88rs6060_spi_write(struct dvb_frontend *fe,
				struct ecp3_info *ecp3inf)
{
	struct m88rs6060_state *dev = fe->demodulator_priv;
	struct i2c_client *client = dev->demod_client;

	if (dev->write_properties)
		dev->write_properties(client->adapter, ecp3inf->reg,
				      ecp3inf->data);
	return;
}

static int m88rs6060_stop_task(struct dvb_frontend *fe)
{
	struct m88rs6060_state* state = fe->demodulator_priv;
	struct m88rs6060_spectrum_scan_state* ss = &state->scan_state;
	struct m88rs6060_constellation_scan_state* cs = &state->constellation_scan_state;
	if(ss->freq)
		kfree(ss->freq);
	if(ss->spectrum)
		kfree(ss->spectrum);
	if(cs->samples)
		kfree(cs->samples);
	memset(ss, 0, sizeof(*ss));
	memset(cs, 0, sizeof(*cs));
	dprintk("Freed memory\n");
	return 0;
}


static int m88rs6060_tune(struct dvb_frontend *fe, bool re_tune,
													unsigned int mode_flags, unsigned int *delay, enum fe_status *status)
{
	struct m88rs6060_state* state = fe->demodulator_priv;
	struct dtv_frontend_properties *p = &fe->dtv_property_cache;
	int r = -1;
	bool blind = (p->algorithm == ALGORITHM_BLIND ||p->algorithm == ALGORITHM_BLIND_BEST_GUESS);
	if(blind) {
		if(p->delivery_system == SYS_UNDEFINED)
			p->delivery_system = SYS_AUTO;
	}

	*delay = HZ / 5;

	p->symbol_rate =
		(p->symbol_rate==0) ? 27500000: p->symbol_rate; //determines tuner bandwidth set during blindscan


	state->satellite_scan = false;

	if (re_tune) {
		dprintk("tune called with freq=%d srate=%d re_tune=%d\n", p->frequency, p->symbol_rate, re_tune);
		m88rs6060_stop_task(fe);
		m88rs6060_tune_once(fe);
#ifdef TODO
		state->tune_time = jiffies;
#endif
	}
#if 0 //already done in 	m88rs6060_tune_once
	m88rs6060_get_signal_info(fe);
#endif
	/*
		read rf level, snr, signal quality, lock_status
	*/
	r = m88rs6060_read_status(fe, status);

#ifdef TODO
	{
		int max_num_samples = state->symbol_rate /5 ; //we spend max 500 ms on this
		if(max_num_samples > 1024)
			max_num_samples = 1024; //also set an upper limit which should be fast enough
		stv091x_constellation_start(fe, &p->constellation, max_num_samples);
	}
#endif
	if (r)
		return r;

	if (*status & FE_HAS_LOCK) {
		return 0;
	} else
		*delay = HZ;
	return 0;
}


static enum dvbfe_algo m88rs6060_get_algo(struct dvb_frontend *fe)
{
	//dprintk("%s()\n", __func__);
	return DVBFE_ALGO_HW;
}



static const struct dvb_frontend_ops m88rs6060_ops = {
	.delsys = {SYS_DVBS, SYS_DVBS2, SYS_AUTO},
	.info = {
		 .name = "Montage m88rs6060",
		 .frequency_min_hz = 950 * MHz,
		 .frequency_max_hz = 2150 * MHz,
		 .symbol_rate_min = 100000,
		 .symbol_rate_max = 45000000,
		.caps			= FE_CAN_INVERSION_AUTO |
					  FE_CAN_FEC_AUTO       |
					  FE_CAN_QPSK           |
					  FE_CAN_RECOVER	|
					  FE_CAN_2G_MODULATION  |
					  FE_CAN_MULTISTREAM
	},

	.tuner_ops = {
		.info = {
			.frequency_min_hz = 950 * MHz,
			.frequency_max_hz = 2150 * MHz
		},
	},

	.init = m88rs6060_init,
	.release = m88rs6060_detach,
	.tune = m88rs6060_tune,

	.read_status = m88rs6060_read_status,
	.read_ber = m88rs6060_read_ber,
	.read_signal_strength = m88rs6060_read_signal_strength,
	.read_snr = m88rs6060_read_snr,
	.set_voltage = m88rs6060_set_voltage,
	.set_tone = m88rs6060_set_tone,
	.diseqc_send_burst = m88rs6060_diseqc_send_burst,
	.diseqc_send_master_cmd = m88rs6060_diseqc_send_master_cmd,
	.get_frontend_algo	= m88rs6060_get_algo,
	.spi_read = m88rs6060_spi_read,
	.spi_write = m88rs6060_spi_write,

};

static int m88rs6060_ready(struct m88rs6060_state *dev)
{
	struct i2c_client *client = dev->demod_client;
	int ret, len, rem;
	const struct firmware *firmware;
	const char *name = M88RS6060_FIRMWARE;
	unsigned val;

	dev_dbg(&client->dev, "%s", __func__);

	//rest the harware and wake up the demod and tuner;
	m88rs6060_hard_rest(dev);
	rs6060_init(dev);

	ret = regmap_write(dev->demod_regmap, 0x07, 0xe0);	//global reset ,diseqc and fec reset
	if (ret)
		goto err;
	ret = regmap_write(dev->demod_regmap, 0x07, 0x00);
	if (ret)
		goto err;

	/* cold state - try to download firmware */
	dev_info(&client->dev, "found a '%s' in cold state\n",
		 dev->fe.ops.info.name);

	/* request the firmware, this will block and timeout */
	ret = request_firmware(&firmware, name, &client->dev);
	if (ret) {
		dev_err(&client->dev, "firmware file '%s' not found\n", name);
		goto err;
	}

	dev_info(&client->dev, "downloading firmware from file '%s'\n", name);

	ret = regmap_write(dev->demod_regmap, 0xb2, 0x01);
	if (ret)
		goto err_release_firmware;

	dev_dbg(&client->dev, " firmware size  = %lu data %02x %02x %02x\n",
		 firmware->size, firmware->data[0], firmware->data[1],
		 firmware->data[2]);

	for (rem = firmware->size; rem > 0; rem -= (dev->config.i2c_wr_max - 1)) {
		len = min(dev->config.i2c_wr_max - 1, rem);
		ret = m88rs6060_fireware_download(dev, 0xb0,
						  &firmware->data[firmware->size - rem],
						  len);
		if (ret) {
			dev_err(&client->dev,
				"firmware download failed  len  %d  %d\n", len,
				ret);
			goto err_release_firmware;
		}
	}

	ret = regmap_write(dev->demod_regmap, 0xb2, 0x00);
	if (ret)
		goto err_release_firmware;

	release_firmware(firmware);

	ret = regmap_read(dev->demod_regmap, 0xb9, &val);
	if (ret)
		goto err;

	if (!val) {
		ret = -EINVAL;
		dev_info(&client->dev, "firmware did not run\n");
		goto err;
	}

	dev_info(&client->dev, "found a '%s' in warm state\n",
		 dev->fe.ops.info.name);
	dev_info(&client->dev, "firmware version:%X\n", val);
	msleep(5);
	m88res6060_set_ts_mode(dev);

	regmap_read(dev->demod_regmap, 0x4d, &val);
	regmap_write(dev->demod_regmap, 0x4d, val & 0xfd);

	return 0;
 err_release_firmware:
	release_firmware(firmware);
 err:
	dev_dbg(&client->dev, "failed=%d\n", ret);
	return ret;

}



static int m88rs6060_remove(struct i2c_client *client)
{
	struct m88rs6060_state *dev = i2c_get_clientdata(client);
	dev_dbg(&client->dev, "\n");
	dprintk("client=%p tuner_client=%p id=?? dev=%p dev->fe=%p tuner_client=%p\n", client,
					dev->tuner_client,
					dev, (dev ? &dev->fe : 0),  (dev ? &dev->tuner_client : 0));
	dprintk("dev=%p remove\n", &client->dev);
	if(dev->tuner_client)
	   i2c_unregister_device(dev->tuner_client);

	dev->fe.ops.release = NULL;
	dev->fe.demodulator_priv = NULL;

	kfree(dev);
	dprintk("Clearing client data");
	i2c_set_clientdata(client, 0);
	dprintk("returning");
	return 0;
}

static const struct i2c_device_id m88rs6060_id_table[] = {
	{"m88rs6060", 0},
	{}
};

void m88rs6060_detach(struct dvb_frontend* fe)
{
	struct m88rs6060_state* state = fe->demodulator_priv;
	struct i2c_client *client = state->demod_client;
	dprintk("Called fe=%p client=%p\n", fe, client);
	state->fe.ops.release = NULL;
}


/*
	adapterno is the index of the adapter on a card, starting at 0
 */
struct i2c_client* m88rs6060_attach(struct i2c_adapter* i2c, struct i2c_board_info* board_info, int adapterno)
{
	struct m88rs6060_state* state;
	struct i2c_client* client;
	int ret;
	unsigned tmp;


	static const struct regmap_config regmap_config = {
		.reg_bits = 8,
		.val_bits = 8,
	};

	struct m88rs6060_cfg* cfg = board_info->platform_data;
	dprintk("i2c=%p adapterno=%d\n", i2c, adapterno);

	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state) {
		ret = -ENOMEM;
		goto err_kfree;
	}

	client = i2c_new_client_device(i2c, board_info);
	if(!client) {
		ret = -ENOMEM;
		goto err_kfree;
	}
	state->adapterno = adapterno;


	dprintk("client=%p\n", client);
	dev_dbg(&client->dev, "\n");

	state->config.demod_adr = cfg->demod_adr;
	state->config.tuner_adr = cfg->tuner_adr;
	state->config.clk = cfg->clk;
	state->config.ts_mode = cfg->ts_mode;
	state->config.i2c_wr_max = cfg->i2c_wr_max;
	state->config.ts_pinswitch = cfg->ts_pinswitch;
	state->config.repeater_value = cfg->repeater_value;
	state->config.read_properties = cfg->read_properties;
	state->config.write_properties = cfg->write_properties;
	state->config.envelope_mode = cfg->envelope_mode;
	state->demod_client = client;
	state->TsClockChecked = false;

	//for ci clk si5351
	state->config.GetSpeed = cfg->GetSpeed;
	state->config.SetSpeedstatus = cfg->SetSpeedstatus;
	state->config.GetSpeedstatus = cfg->GetSpeedstatus;
	state->config.SetTimes= cfg->SetTimes;
	state->config.HAS_CI = cfg->HAS_CI;
	state->config.num = cfg->num;
	state->plla_freq = 0;
	state->pllb_freq = 0;
	state->newTP = 0;

	state->demod_regmap = devm_regmap_init_i2c(state->demod_client, &regmap_config);
	if (IS_ERR(state->demod_regmap)) {
		ret = PTR_ERR(state->demod_regmap);
		goto err_kfree;
	}
	/*check demod i2c */
	ret = regmap_read(state->demod_regmap, 0x00, &tmp);
	if (ret)
		goto err_regmap_0_regmap_exit;
//	if (tmp != 0xe2)
//		goto err_regmap_0_regmap_exit;

	state->tuner_client =
	    i2c_new_dummy_device(client->adapter, state->config.tuner_adr);
	if (IS_ERR(state->tuner_client)) {
		ret = PTR_ERR(state->tuner_client);
		dev_err(&client->dev, "I2c register failed \n");
		goto err_client_1_i2c_unregister_device;
	}

	state->mclk = 96000;

	memcpy(&state->fe.ops, &m88rs6060_ops, sizeof(struct dvb_frontend_ops));
	dprintk("copied options algo=%p\n", state->fe.ops.get_frontend_algo);
	*cfg->fe = &state->fe;
	state->fe_status = 0;
	state->write_properties = cfg->write_properties;
	state->read_properties = cfg->read_properties;

	state->fe.demodulator_priv = state;
	i2c_set_clientdata(client, state);

	dev_dbg(&client->dev, "found the chip of %s.", m88rs6060_ops.info.name);

	//ready chip
	m88rs6060_ready(state);
	if(state->config.HAS_CI){  //for 6910SECI
		si5351_init(state);
		si5351_set_freq(state,62500000,0,SI5351_CLK0);
	 }

	return client;

 err_client_1_i2c_unregister_device:
	i2c_unregister_device(state->tuner_client);
 err_regmap_0_regmap_exit:
	regmap_exit(state->demod_regmap);
 err_kfree:
	kfree(state);

	dev_warn(&client->dev, "probe failed = %d\n", ret);
	return NULL;

}

EXPORT_SYMBOL_GPL(m88rs6060_attach);
MODULE_AUTHOR("Davin zhang <Davin@tbsdtv.com>");
MODULE_DESCRIPTION("Montage M88RS6060 driver");
MODULE_LICENSE("GPL");
MODULE_FIRMWARE(M88RS6060_FIRMWARE);
