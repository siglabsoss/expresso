// =============================================================================
//                           COPYRIGHT NOTICE
// Copyright 2010 (c) Lattice Semiconductor Corporation
// ALL RIGHTS RESERVED
// This confidential and proprietary software may be used only as authorised by
// a licensing agreement from Lattice Semiconductor Corporation.
// The entire notice above must be reproduced on all authorized copies and
// copies may only be made to the extent permitted by a licensing agreement from
// Lattice Semiconductor Corporation.
//
// Lattice Semiconductor Corporation        TEL : 1-800-Lattice (USA and Canada)
// 5555 NE Moore Court                            408-826-6000 (other locations)
// Hillsboro, OR 97124                     web  : http://www.latticesemi.com/
// U.S.A                                   email: techsupport@latticesemi.com
// =============================================================================
//                         FILE DETAILS
// Project          : DDR3 IOPB Demo
// File             : ddr3_test_params.v
// Title            : Parameters defined by user logic
// =============================================================================
//                        REVISION HISTORY
// Version          : 1.0
// Mod. Date        : June, 2010 / Kyoho Lee
// Changes Made     : Initial Creation
// =============================================================================

`include "ddr3_sdram_mem_params.v"
`ifdef RTL_SIM
 `include "tb_config_params.v"
`endif

//== Define LED polarity
`define LED_ON					0
`define LED_OFF					1
//`define ddr3
//`define	WrRqDDelay_2

`define UsrCmdBrstCnt			16	// 2,4,8,16,32 allowed, default=2
									// 1 can be used but dynamic OTF change from BC4 to BL8 may not work




