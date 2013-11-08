--=============================================================================
-- VHDL component generated by ispLever    01/20/2010    14:16:54             
-- Filename: sgdma_pkg.vhd                                        
-- Copyright(c) 2005 Lattice Semiconductor Corporation. All rights reserved.   
--=============================================================================

library IEEE; 
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
PACKAGE sgdma_pkg IS
    COMPONENT sgdma
    PORT (
        clk: in std_logic;
        rstn: in std_logic;
        rst_core: in std_logic;
        a_cyc: out std_logic;
        a_stb: out std_logic;
        a_lock: out std_logic;
        a_we: out std_logic;
        a_cti: out std_logic_vector (2 downto 0);
        a_sel: out std_logic_vector (7 downto 0);
        a_addr: out std_logic_vector (31 downto 0);
        a_wdat: out std_logic_vector (63 downto 0);
        a_rdat: in std_logic_vector (63 downto 0);
        a_ack: in std_logic;
        a_err: in std_logic;
        a_retry: in std_logic;
        a_eod: in std_logic;
        b_cyc: out std_logic;
        b_stb: out std_logic;
        b_lock: out std_logic;
        b_we: out std_logic;
        b_cti: out std_logic_vector (2 downto 0);
        b_sel: out std_logic_vector (7 downto 0);
        b_addr: out std_logic_vector (31 downto 0);
        b_wdat: out std_logic_vector (63 downto 0);
        b_rdat: in std_logic_vector (63 downto 0);
        b_ack: in std_logic;
        b_err: in std_logic;
        b_retry: in std_logic;
        b_eod: in std_logic;
        b_active: out std_logic;
        saddr: in std_logic_vector (31 downto 0);
        swdat: in std_logic_vector (31 downto 0);
        srdat: out std_logic_vector (31 downto 0);
        scyc: in std_logic;
        sstb: in std_logic;
        ssel: in std_logic_vector (3 downto 0);
        swe: in std_logic;
        sack: out std_logic;
        serr: out std_logic;
        dma_req: in std_logic_vector (1 downto 0);
        dma_ack: out std_logic_vector (1 downto 0);
        eventx: out std_logic_vector (1 downto 0);
        errorx: out std_logic_vector (1 downto 0);
        actchan: out std_logic_vector (0 downto 0);
        subchan: out std_logic_vector (0 downto 0);
        a_active: out std_logic;
        max_burst_size: out std_logic_vector (15 downto 0);
        bd_waddr: out std_logic_vector (9 downto 0) ;
        bd_raddr: out std_logic_vector (9 downto 0) ;
        bd_wdat: out std_logic_vector (31 downto 0) ;
        bd_rdat: in std_logic_vector (31 downto 0) ;
        bd_we: out  std_logic;
        bd_re: out  std_logic;
        bd_rval: in  std_logic
    );

    END COMPONENT;

END PACKAGE sgdma_pkg;

