$display("#####################################");
$display("####### Set G/A/B Enables ###########", testnum );
$display("#####################################");
command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GSTATUS_ADDR,
		 (1<<AENABLE_OFF)+(1<<BENABLE_OFF)+(1<<GENABLE_OFF), 0 );

$display("#####################################");
$display("####### Initialize PBOFFSETs ########");
$display("#####################################");
for( c=0; c<16; c=c+1 ) begin
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR + c*32 + PBOFFSETN_ADDR, 0, 0 );
end

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### SIMPLE DMA           ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### 128 bytes, 4 per cyc ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, 'haaaaaaaa >> c, 0 );
	end
	$display("#######   verify slave contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_VF, c+'h80, 'haaaaaaaa >> c, 'hffffffff );
	end

	$display("#######   enable channel 1 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffd<<CHMASK_OFF)+(2<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffd<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 'haaaaaaaa >> c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d             #########", testnum );
$display("####### Packet Buffer Xfers #########");
$display("####### SLAVE0 to PB, then  #########");
$display("####### PB to SLAVE1        #########");
$display("####### channel 0           #########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end
	$display("#######   SLAVE0 to packet buffer  ######");
		$display("#######   set up buffer descriptor ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (2<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (16<<XFER_SIZE_OFF)
				 + (16<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h00000000, 0 );

		$display("#######   enable channel 0 ##########");
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

		$display("#######   set bd base   #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	$display("#######   packet buffer to SLAVE1   ######");
		$display("#######   set up buffer descriptor ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (2<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 + (1<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (16<<XFER_SIZE_OFF)
				 + (16<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h00000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Multiple BDs         ########");
$display("####### SLAVE0 to PB, then   ########");
$display("####### PB to SLAVE1         ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, -c, 0 );
	end

	$display("#######   set up buffer descriptors ######");
	$display("#######   SLAVE0 to packet buffer  ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (0<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (2<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (16<<XFER_SIZE_OFF)
				 + (16<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h00000000, 0 );

		$display("#######   enable channel 0 ##########");
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

	$display("#######   packet buffer to SLAVE1   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (2<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG1_ADDR,
				   (16<<XFER_SIZE_OFF)
				 + (16<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+SRCADDR_ADDR, 'h00000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, -c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### 128 bytes            ########");
$display("####### non-burst slave      ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, c, 0 );
	end

	$display("#######   enable channel 1 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffd<<CHMASK_OFF)+(2<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffd<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE1 to SLAVE0         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   disable slave burst   #####");
		command( WBSLAV1, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 0, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	$display("#######   enable slave burst    #####");
	command( WBSLAV1, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 1, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Multi-burst transfer ########");
$display("####### SLAVE1 to SLAVE0     ########");
$display("####### 1024 bytes, 64 per cyc ######");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<256; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_WR, c, 'haaaaaaaa << c, 0 );
	end

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE1 to SLAVE0         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (1<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (0<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (1024<<XFER_SIZE_OFF)
				 + (64<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h02000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000000, 0 );

		$display("#######   disable slave burst   #####");
		command( WBSLAV1, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 0, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE0 contents #########");
	for( c=0; c<256; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_VF, c, 'haaaaaaaa << c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	$display("#######   enable slave burst    #####");
	command( WBSLAV1, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 1, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Multi-burst          ########");
$display("####### classic cycles only  ########");
$display("####### SLAVE1 to SLAVE0     ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<256; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_WR, c, 'haaaaaaaa << c, 0 );
	end

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE1 to SLAVE0         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (1<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (0<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (1024<<XFER_SIZE_OFF)
				 + (64<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h02000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000000, 0 );

		$display("#######   disable slave burst   #####");
		command( WBSLAV1, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 0, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE0 contents #########");
	for( c=0; c<256; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_VF, c, 'haaaaaaaa << c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	$display("#######   enable slave burst    #####");
	command( WBSLAV1, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 1, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### split transaction    ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<8; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE1 to SLAVE0         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (32<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<8; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d #####################", testnum );
$display("####### split transaction    ########");
$display("####### bus A to bus A       ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<16; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE1 to SLAVE0         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (1<<SPLIT_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (0<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000040, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE0 contents #########");
	for( c=0; c<16; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### bus A to bus A       ########");
$display("####### (autonomous split)   ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<16; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, -c, 0 );
	end

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SPLIT_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (0<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000040, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE0 contents #########");
	for( c=0; c<16; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_VF, c, -c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### bus A to bus A       ########");
$display("####### (autonomous split)   ########");
$display("####### non-burst slaves     ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, 'haced0000+c, 0 );
	end

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SPLIT_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (0<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (64<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000080, 0 );

		$display("#######   disable slave burst   #####");
		command( WBSLAV0, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 0, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE0 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_VF, c, 'haced0000+c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	$display("#######   enable slave burst    #####");
	command( WBSLAV0, WBSLAV_CTL_WR, WBSLAV_ENABLE_BURST, 1, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### BURST AUTORETRY TEST ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave 0 memory #######");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c*c, 0 );
	end

	$display("#######   SLAVE0 to SLAVE1         ######");
		$display("#######   set up buffer descriptor ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SPLIT_OFF)
				 + (1<<AUTORETRY_OFF)
				 + (0<<RETRYTHRESH_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (16<<XFER_SIZE_OFF)
				 + (16<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("#######   enable channel 0 ##########");
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

		$display("#######   set SLAVE0 retry count ####");
		command( WBSLAV0, WBSLAV_CTL_WR, WBSLAV_RETRY_VALUE, 1, 0 );

		$display("#######   unmask retry error    #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 'hf7<<ERRMASK_OFF, 0 );
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GERROR_ADDR, 'hfffe<<CHERRMSK_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

		wait( sa_rty );
		$display("####### RETRY DETECTED!! ############");

	wait( errorx );

	$display("#######   verify error registers   #######");
	command( WBMAST0, WBMAST_BUS_VF, SGDMAC_BASE+GERROR_ADDR, 'h00000001, 'h00000001 );
	command( WBMAST0, WBMAST_BUS_VF, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 8<<ERRORS_OFF, 8<<ERRORS_OFF );

	$display("#######   clear retry error       #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 8<<ERRORS_OFF, 0 );

	$display("#######   verify clear            #######");
	command( WBMAST0, WBMAST_BUS_VF, SGDMAC_BASE+GERROR_ADDR, 'h00000000, 'h00000001 );
	command( WBMAST0, WBMAST_BUS_VF, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 0, 8<<ERRORS_OFF );

	$display("#######   modify retry threshold  #######");
	$display("#######   in buffer descriptor    #######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SPLIT_OFF)
				 + (1<<AUTORETRY_OFF)
				 + (4<<RETRYTHRESH_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		$display("#######   and SLAVE0 retry count ####");
		command( WBSLAV0, WBSLAV_CTL_WR, WBSLAV_RETRY_VALUE, 3, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c*c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### HARDWARE RETRY TEST  ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave 0 memory #######");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, 5<<c, 0 );
	end

	$display("#######   SLAVE0 to SLAVE1         ######");
		$display("#######   set up buffer descriptor ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SPLIT_OFF)
				 + (0<<AUTORETRY_OFF)
				 + (0<<RETRYTHRESH_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (16<<XFER_SIZE_OFF)
				 + (16<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("#######   enable channel 0 ##########");
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

		$display("#######   set SLAVE0 retry count ####");
		command( WBSLAV0, WBSLAV_CTL_WR, WBSLAV_RETRY_VALUE, 1, 0 );

		$display("#######   unmask retry error    #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 'hf7<<ERRMASK_OFF, 0 );
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GERROR_ADDR, 'hfffe<<CHERRMSK_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

		wait( ma_rty );
		$display("####### RETRY DETECTED!! ############");
		wait( !ma_rty );

	$display("#######   verify error registers   #######");
	command( WBMAST0, WBMAST_BUS_VF, SGDMAC_BASE+GERROR_ADDR, 'h00000000, 'h00000001 );
	command( WBMAST0, WBMAST_BUS_VF, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 0, 8<<ERRORS_OFF );

	$display("#######   assert external request  #######");
	dma_req[0] = 1;

	wait( dma_ack );

	$display("#######   clear external request  #######");
	dma_req[0] = 0;

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<4; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 5<<c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### End-Of-Data          ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, 'haaaaaaaa >> c, 0 );
	end

	$display("#######   fill slave1 memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_WR, c, 0, 0 );
	end

	$display("#######   enable channel 1 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffd<<CHMASK_OFF)+(2<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffd<<CHARBMSK_OFF), 0 );

	$display("#######   SLAVE0 to SLAVE1         ######");
		$display("#######   set up buffer descriptor ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("####### set slave0 eod count    #####");
		command( WBSLAV0, WBSLAV_CTL_WR, WBSLAV_EODCNT, 13, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<13; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 'haaaaaaaa >> c, 'hffffffff );
	end
	command( WBSLAV1, WBSLAV_MEM_VF, 14, 0, 'hffffffff );

	$display("#######   verify eod set           #######");
	command( WBMAST0, WBMAST_BUS_VF, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<EOD_OFF, 1<<EOD_OFF );

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Hardware Request     ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 11           ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, 'h55555555 >> c, 0 );
	end

	$display("#######   enable channel 11 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hf7ff<<CHMASK_OFF)+('h800<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hf7ff<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(44*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(44*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(44*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(44*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(11*32)+CONTROLN_ADDR, 44<<BDBASE_OFF, 0 );

	$display("#######   set channel request   #####");
	dma_req[11] = 1;
	wait( dma_ack[11] );
	dma_req[11] = 0;

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 'h55555555 >> c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+11*32, 'h00000010, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Bus Sizing Test      ########");
$display("####### one byte at a time   ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, 'haaaaaaaa >> c, 0 );
	end

	$display("#######   enable channel 1 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffd<<CHMASK_OFF)+(2<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffd<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (0<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (0<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 'haaaaaaaa >> c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Bus Sizing Test 2    ########");
$display("####### two bytes at a time  ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end

	$display("#######   enable channel 0 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (1<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (1<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### DISSIMILAR BUS SIZES ########");
$display("####### two bytes to four    ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, -c, 0 );
	end

	$display("#######   enable channel 0 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (1<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, -c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Dissimilar Bus Sizes ########");
$display("####### one byte to four     ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end

	$display("#######   enable channel 0 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (0<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Dissimilar Bus Sizes ########");
$display("####### four bytes to one    ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, -c, 0 );
	end

	$display("#######   enable channel 0 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffe<<CHMASK_OFF)+(1<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (0<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000000, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h02000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, -c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Multiple BDs         ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<64; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end

	$display("#######   set up buffer descriptors ######");
	for( c=0; c<4; c=c+1 ) begin
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+CONFIG0_ADDR,
				   ((c==3)<<EOL_OFF)
				 + (0 << SRC_BUS_OFF)
				 + (2 << SRCBUS_SIZE_OFF)
				 + (1 << SRCINCR_OFF)
				 + (1 << DST_BUS_OFF)
				 + (2 << DSTBUS_SIZE_OFF)
				 + (1 << DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+CONFIG1_ADDR,
				   (64 << XFER_SIZE_OFF)
				 + (16 << BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+SRCADDR_ADDR, 'h01000000 + c*64, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+DSTADDR_ADDR, 'h02000000 + c*64, 0 );
	end

	$display("####### set BD base address     #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   write channel request #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<64; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 'h00000010, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### 2-beat transfer      ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<2; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end

	$display("#######   set up buffer descriptors ######");
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
			   (1 << EOL_OFF)
			 + (0 << SRC_BUS_OFF)
			 + (2 << SRCBUS_SIZE_OFF)
			 + (1 << SRCINCR_OFF)
			 + (1 << DST_BUS_OFF)
			 + (2 << DSTBUS_SIZE_OFF)
			 + (1 << DSTINCR_OFF)
			 + (1<<BD_STATUS_OFF)
			 ,0 );
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
			   (8 << XFER_SIZE_OFF)
			 + (8 << BURST_SIZE_OFF)
			 ,0 );
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000000, 0 );
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h02000000, 0 );

	$display("####### set BD base address     #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   write channel request #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<2; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 'h00000010, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### 1-beat transfer      ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   set slave memory #########");
	command( WBSLAV0, WBSLAV_MEM_WR, 0, 'ha5a5a5a5, 0 );

	$display("#######   set up buffer descriptors ######");
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
			   (1 << EOL_OFF)
			 + (0 << SRC_BUS_OFF)
			 + (2 << SRCBUS_SIZE_OFF)
			 + (1 << SRCINCR_OFF)
			 + (1 << DST_BUS_OFF)
			 + (2 << DSTBUS_SIZE_OFF)
			 + (1 << DSTINCR_OFF)
			 + (1<<BD_STATUS_OFF)
			 ,0 );
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
			   (4 << XFER_SIZE_OFF)
			 + (4 << BURST_SIZE_OFF)
			 ,0 );
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000000, 0 );
	command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h02000000, 0 );

	$display("####### set BD base address     #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   write channel request #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	command( WBSLAV1, WBSLAV_MEM_VF, 0, 'ha5a5a5a5, 'hffffffff );

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 'h00000010, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### Multiple BDs         ########");
$display("####### BD_NEXT test         ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### channel 0            ########");
$display("#####################################");

	$display("#######   fill slave0 memory #########");
	for( c=0; c<128; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c, c, 0 );
	end

	$display("#######   clear slave1 memory #########");
	for( c=0; c<128; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_WR, c, 0, 0 );
	end

	$display("#######   set up buffer descriptors ######");
	$display("#######   2 consecutive sets of BDs ######");
	for( c=0; c<8; c=c+1 ) begin
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+CONFIG0_ADDR,
				   (((c==3)||(c==7))<<EOL_OFF)
				 + (0 << SRC_BUS_OFF)
				 + (2 << SRCBUS_SIZE_OFF)
				 + (1 << SRCINCR_OFF)
				 + (1 << DST_BUS_OFF)
				 + (2 << DSTBUS_SIZE_OFF)
				 + (1 << DSTINCR_OFF)
				 + ((c==3) << BD_NEXT_OFF)	// after 4th BD, start next transfer at 5th
				 + (1<<BD_STATUS_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+CONFIG1_ADDR,
				   (64 << XFER_SIZE_OFF)
				 + (16 << BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+SRCADDR_ADDR, 'h01000000 + c*64, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(c*16)+DSTADDR_ADDR, 'h02000000 + c*64, 0 );
	end

	$display("####### set BD base address     #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   write channel request #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify first half SLAVE1 contents #########");
	for( c=0; c<64; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end
	$display("#######   verify second half SLAVE1 contents still zeros #########");
	for( c=64; c<128; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 0, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 'h00000010, 0 );

	$display("#######   write channel request #####");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<128; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 'h00000010, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### BUFFER_STATUS TEST   ########");
$display("####### SLAVE0 to SLAVE1     ########");
$display("####### 128 bytes, 4 per cyc ########");
$display("####### channel 1            ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, 'haaaaaaaa >> c, 0 );
	end

	$display("#######   enable channel 1 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfffd<<CHMASK_OFF)+(2<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfffd<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptor ######");
	$display("#######   SLAVE0 to SLAVE1         ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (1<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (128<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000000, 0 );

		$display("#######   set BD base address and unmask retry error    #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+CONTROLN_ADDR, ('hef<<ERRMASK_OFF) + (4<<BDBASE_OFF), 0 );
		command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GERROR_ADDR, 'hfffd<<CHERRMSK_OFF, 0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( errorx );

	$display("#######   verify error registers   #######");
	command( WBMAST0, WBMAST_BUS_VF, SGDMAC_BASE+GERROR_ADDR, 'h00000002, 'h00000002 );
	command( WBMAST0, WBMAST_BUS_VF, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 16<<ERRORS_OFF, 16<<ERRORS_OFF );

	$display("#######   clear bd error           #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 16<<ERRORS_OFF, 0 );

	$display("#######   verify clear             #######");
	command( WBMAST0, WBMAST_BUS_VF, SGDMAC_BASE+GERROR_ADDR, 'h00000000, 'h00000002 );
	command( WBMAST0, WBMAST_BUS_VF, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 0, 16<<ERRORS_OFF );

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	$display("#######   set bd to available      #######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (1<<BD_STATUS_OFF)
				 + (1<<BD_STATUS_EN_OFF)
				 ,0 );

		$display("#######   write channel request #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 'haaaaaaaa >> c, 'hffffffff );
	end

	$display("#######   clear transfer complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(1*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### SIMPLE DMA           ########");
$display("####### SIMULTANEOUS REQUESTS #######");
$display("####### channels 0 and 8     ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, 'haaaaaaaa >> c, 0 );
	end

	$display("#######   enable channels 0 and 8 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfefe<<CHMASK_OFF)+('h0101<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfefe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptors ######");
	$display("#######   channel 0                 ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (64<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h01000000, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   channel 8                 ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (64<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000240, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000040, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("#######   write channel requests #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+STATUSN_ADDR, 1<<REQUEST_OFF, 0 );

	wait( eventx == 1 );
	wait( eventx != 1 );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, 'haaaaaaaa >> c, 'hffffffff );
	end

	$display("#######   clear transfers complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### SIMPLE DMA           ########");
$display("####### SIMULTANEOUS DMA_REQS #######");
$display("####### channels 0 and 8     ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<6; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, c, 0 );
	end

	$display("#######   enable channels 0 and 8 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfefe<<CHMASK_OFF)+('h0101<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfefe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptors ######");
	$display("#######   channel 0, descriptor 0   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (0<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (8<<XFER_SIZE_OFF)
				 + (8<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h01000000, 0 );
	$display("#######   channel 0, descriptor 1   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG1_ADDR,
				   (4<<XFER_SIZE_OFF)
				 + (4<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+SRCADDR_ADDR, 'h01000208, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+DSTADDR_ADDR, 'h01000008, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   channel 8, descriptor 0   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (0<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (8<<XFER_SIZE_OFF)
				 + (8<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h0100020c, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h0100000c, 0 );
	$display("#######   channel 8, descriptor 1   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+CONFIG1_ADDR,
				   (4<<XFER_SIZE_OFF)
				 + (4<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+SRCADDR_ADDR, 'h01000214, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+DSTADDR_ADDR, 'h01000014, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("####### hardware requests #####");
		dma_req[0] = 1;
		wait( ma_cyc[1] );
		dma_req[8] = 1;

	wait( eventx == 16'h0101 );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<6; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, c, 'hffffffff );
	end

	$display("####### clear hardware requests #####");
	dma_req = 16'h0000;
	$display("#######   clear transfers complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### SIMPLE DMA           ########");
$display("####### SIMULTANEOUS DMA_REQS #######");
$display("####### MULTIPLE BURSTS      ########");
$display("####### channels 0 and 8     ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, ~c, 0 );
	end

	$display("#######   enable channels 0 and 8 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfefe<<CHMASK_OFF)+('h0101<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfefe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptors ######");
	$display("#######   channel 0, descriptor 0   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (0<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h01000000, 0 );
	$display("#######   channel 0, descriptor 1   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+SRCADDR_ADDR, 'h01000240, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+DSTADDR_ADDR, 'h01000040, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   channel 8, descriptor 0   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (0<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000280, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000080, 0 );
	$display("#######   channel 8, descriptor 1   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+SRCADDR_ADDR, 'h010002c0, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+DSTADDR_ADDR, 'h010000c0, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("####### hardware requests #####");
		dma_req[0] = 1;
		wait( ma_cyc[1] );
		dma_req[8] = 1;

	wait( eventx == 16'h0101 );

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, ~c, 'hffffffff );
	end

	$display("####### clear hardware requests #####");
	dma_req = 16'h0000;
	$display("#######   clear transfers complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;

$display("#####################################");
$display("####### TEST %d              ########", testnum );
$display("####### SIMPLE DMA           ########");
$display("####### OFFSET DMA_REQS      #######");
$display("####### MULTIPLE BURSTS      ########");
$display("####### channels 0 and 8     ########");
$display("#####################################");

	$display("#######   fill slave memory #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV0, WBSLAV_MEM_WR, c+'h80, ~c, 0 );
	end

	$display("#######   enable channels 0 and 8 ##########");
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GCONTROL_ADDR, ('hfefe<<CHMASK_OFF)+('h0101<<CHENABLE_OFF), 0 );
	command( WBMAST0, WBMAST_BUS_WR, SGDMAC_BASE+GARBITER_ADDR, ('hfefe<<CHARBMSK_OFF), 0 );

	$display("#######   set up buffer descriptors ######");
	$display("#######   channel 0, descriptor 0   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+CONFIG1_ADDR,
				   (128<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+SRCADDR_ADDR, 'h01000200, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(0*16)+DSTADDR_ADDR, 'h01000000, 0 );
	$display("#######   channel 0, descriptor 1   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+SRCADDR_ADDR, 'h01000240, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(1*16)+DSTADDR_ADDR, 'h01000040, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+CONTROLN_ADDR, 0<<BDBASE_OFF, 0 );

	$display("#######   channel 8, descriptor 0   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+CONFIG1_ADDR,
				   (32<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+SRCADDR_ADDR, 'h01000280, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(4*16)+DSTADDR_ADDR, 'h01000080, 0 );
	$display("#######   channel 8, descriptor 1   ######");
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+CONFIG0_ADDR,
				   (1<<EOL_OFF)
				 + (0<<SRC_BUS_OFF)
				 + (2<<SRCBUS_SIZE_OFF)
				 + (1<<SRCINCR_OFF)
				 + (1<<DST_BUS_OFF)
				 + (2<<DSTBUS_SIZE_OFF)
				 + (1<<DSTINCR_OFF)
				 + (0<<BD_STATUS_OFF)
				 + (0<<BD_STATUS_EN_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+CONFIG1_ADDR,
				   (64<<XFER_SIZE_OFF)
				 + (32<<BURST_SIZE_OFF)
				 ,0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+SRCADDR_ADDR, 'h010002c0, 0 );
		command( WBMAST0, WBMAST_BUS_WR, BD_BASE_ADDR+(5*16)+DSTADDR_ADDR, 'h010000c0, 0 );

		$display("####### set BD base address     #####");
		command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+CONTROLN_ADDR, 4<<BDBASE_OFF, 0 );

		$display("####### hardware requests #####");
		dma_req[0] = 1;
		wait( ma_cyc[1] );
		wait( !ma_cyc[1] );
		//wait( ma_cyc[1] );	// wait until second transfer for channel 0 begins
		dma_req[8] = 1;

	wait( dma_ack[8] );
		dma_req[8] = 0;
	wait( dma_ack[0] );
		dma_req[0] = 0;

	$display("#######   verify SLAVE1 contents #########");
	for( c=0; c<32; c=c+1 ) begin
		command( WBSLAV1, WBSLAV_MEM_VF, c, ~c, 'hffffffff );
	end

	$display("####### clear hardware requests #####");
	dma_req = 16'h0000;
	$display("#######   clear transfers complete  #######");
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(0*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );
	command( WBMAST0, WBMAST_BUS_WR, CHAN_BASE_ADDR+(8*32)+STATUSN_ADDR, 1<<CLRCOMP_OFF, 0 );

	testnum = testnum + 1;
