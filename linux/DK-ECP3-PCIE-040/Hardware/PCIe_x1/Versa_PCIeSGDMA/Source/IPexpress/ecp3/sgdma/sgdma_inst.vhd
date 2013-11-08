--===========================================================================
-- VHDL instance generated by ispLever    01/20/2010    14:16:54             
-- Filename: sgdma_inst.vhd                                        
-- Copyright(c) 2006 Lattice Semiconductor Corporation. All rights reserved.   
--=============================================================================

sgdma_inst: entity work.sgdma(str)
    port map (
        clk            => clk,
        rstn           => rst,
        rst_core       => rst_core,
        a_cyc          => a_cyc,
        a_stb          => a_stb,
        a_lock         => a_lock,
        a_we           => a_we,
        a_cti          => a_cti(2 downto 0),
        a_sel          => a_sel(7 downto 0),
        a_addr         => a_addr(31 downto 0),
        a_wdat         => a_wdat(63 downto 0),
        a_rdat         => a_rdat(63 downto 0),
        a_ack          => a_ack,
        a_err          => a_err,
        a_retry        => a_retry,
        a_eod          => a_eod,
        b_cyc          => b_cyc,
        b_stb          => b_stb,
        b_lock         => b_lock,
        b_we           => b_we,
        b_cti          => b_cti(2 downto 0),
        b_sel          => b_sel(7 downto 0),
        b_addr         => b_addr(31 downto 0),
        b_wdat         => b_wdat(63 downto 0),
        b_rdat         => b_rdat(63 downto 0),
        b_ack          => b_ack,
        b_err          => b_err,
        b_retry        => b_retry,
        b_eod          => b_eod,
        b_active       => b_active,
        saddr          => saddr(31 downto 0),
        swdat          => swdat(31 downto 0),
        srdat          => srdat(31 downto 0),
        scyc           => scyc,
        sstb           => sstb,
        ssel           => ssel(3 downto 0),
        swe            => swe,
        sack           => sack,
        serr           => serr,
        dma_req        => dma_req(1 downto 0),
        dma_ack        => dma_ack(1 downto 0),
        eventx         => eventx(1 downto 0),
        errorx         => errorx(1 downto 0),
        actchan        => actchan(0 downto 0),
        subchan        => subchan(0 downto 0),
        a_active       => a_active,
        max_burst_size => max_burst_size(15 downto 0),
        bd_waddr       => bd_waddr(9 downto 0),
        bd_raddr       => bd_raddr(9 downto 0),
        bd_wdat        => bd_wdat(31:0),
        bd_rdat        => bd_rdat(31:0),
        bd_we          => bd_we,
        bd_re          => bd_re,
        bd_rval        => bd_rval
);

