
--
--------------------------------------------------------------------------------
--
-- File ID     : $Id: pcie_vhdl_test_case-pb.vhd 1277 2010-09-27 19:49:13Z  $
-- Generated   : $LastChangedDate: 2010-09-27 21:49:13 +0200 (Mo, 27. Sep 2010) $
-- Revision    : $LastChangedRevision: 1277 $
--
--------------------------------------------------------------------------------
Library IEEE;

Use IEEE.std_logic_1164.all;

Use WORK.bfm_lspcie_rc_types_pkg.all;

Package Body pcie_vhdl_test_case_pkg is
   procedure run_test(signal clk : in    std_logic;
                      signal sv  : inout t_bfm_stim;
                      signal rv  : in    t_bfm_resp;
                             id  : in    natural := 0) is
   begin
      wait;
   end procedure; 

End pcie_vhdl_test_case_pkg;
