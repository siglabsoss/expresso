Library IEEE;
Library ieee_proposed ;

Use IEEE.std_logic_1164.all;
Use ieee_proposed.env.all;    -- provides the call to stop(). See end of test below
Use IEEE.numeric_std.all;     -- provides e.g. add for std_logic_vector + constant 

Package Body pcie_vhdl_test_case_pkg is
       
   procedure run_test(signal clk : in    std_logic;
                      signal sv  : inout t_bfm_stim;
                      signal rv  : in    t_bfm_resp;
                             id  : in    natural := 0) is
                             
         -- define the base address register settings to use in this test   
      constant c_all_zero           : std_logic_vector(31 downto 0) := (others => '0');
      constant c_pcie_rsrc_base_0   : std_logic_vector(31 downto 0) := X"FEDC0000";
      constant c_pcie_rsrc_base_1   : std_logic_vector(31 downto 0) := X"9ABC0000";
      
      variable v_addr_start   : std_logic_vector(31 downto 0);
   begin
      wait for 0 ns;
      
         -- useful for checking in the console window that the correct language 
         -- is on use
      report "Running VHDL Test Case" & LF;

         -- Set up the bus no, func no. and device no in the DUT
         -- The function no. should always be set to zero 
         -- (no multi-function device)
         -- This should be varied when running multiple tests to check that
         -- the user hardware is picking up the correct values from the Lattice
         -- core
      set_dut_id(sv, 2, 1, 0);

         -- increase the default values for credit-based flow control in the BFM
         -- (More on this in session 3. For session 2, these lines can be omitted)
      svc_ca_p(clk, sv, 7, 56);
      svc_ca_np(clk, sv, 7, 0);
      svc_ca_cplx(clk, sv, 7, 56);
            
         -- Get a bit of distance from link up   
      idle(clk, 128);

         -- To communicate with an end-point, the end-point resources (e.g. registers)
         -- must be mapped into PCI Express memory or I/O space
         -- This will be discussed in more detail in session 5
         -- To summarise the initialisation scenario as it runs in the real world, 
         -- First, all Base address registers are written with all ones
         -- Note: the very first PCI Express communication to a device MUST be a 
         -- configuration space write (sets Bus no, dev no, funcno in device) 
      cfgwr0(clk, sv, rv, c_csreg_bar0, X"FFFFFFFF", cpl_wait => true);
      cfgwr0(clk, sv, rv, c_csreg_bar1, X"FFFFFFFF", cpl_wait => true);
      cfgwr0(clk, sv, rv, c_csreg_bar2, X"FFFFFFFF", cpl_wait => true);
      cfgwr0(clk, sv, rv, c_csreg_bar3, X"FFFFFFFF", cpl_wait => true);
      cfgwr0(clk, sv, rv, c_csreg_bar4, X"FFFFFFFF", cpl_wait => true);
      cfgwr0(clk, sv, rv, c_csreg_bar5, X"FFFFFFFF", cpl_wait => true);

         -- Typically, the BIOS /System SW would try and identify the device
      cfgrd0(clk, sv, rv, c_csreg_vend_id, X"FFFFFFFF", no_compare => true);

         -- Typically, the BIOS /System SW would read back the Base Address registers
         -- written above to determine the resource window size (again, more on this in
         -- session 5)
      cfgrd0(clk, sv, rv, c_csreg_bar0, X"FFFFFFFF", no_compare => true);
      cfgrd0(clk, sv, rv, c_csreg_bar1, X"FFFFFFFF", no_compare => true);
      cfgrd0(clk, sv, rv, c_csreg_bar2, X"FFFFFFFF", no_compare => true);
      cfgrd0(clk, sv, rv, c_csreg_bar3, X"FFFFFFFF", no_compare => true);
      cfgrd0(clk, sv, rv, c_csreg_bar4, X"FFFFFFFF", no_compare => true);
      cfgrd0(clk, sv, rv, c_csreg_bar5, X"FFFFFFFF", no_compare => true);

         -- BIOS or system sw must then locate the device within the system memory map
         -- by writing a base address to the base-address register.
         -- In this program, the base address is a constant assigned at the head of the 
         -- test program. To rerun the test with different BAR settings, you only need
         -- to modify the constant above.
      cfgwr0(clk, sv, rv, c_csreg_bar0, c_pcie_rsrc_base_0);
      cfgwr0(clk, sv, rv, c_csreg_bar1, c_all_zero);
      cfgwr0(clk, sv, rv, c_csreg_bar2, c_pcie_rsrc_base_1);

         -- Modern Bioses /operating systems send the Set Slot Power Limit message after 
         -- initial configuration is complete. (More on this in session 3 and 4)
      msgd_set_slot_power_limit(clk, sv, rv, value => X"02", scale => "01");

         -- Test that access on non-enabled address spaces result in UR
      iowr(clk, sv, rv, c_pcie_rsrc_base_1, X"2468_ACE0", cpl_sta => c_cpl_sta_ur);
      iord(clk, sv, rv, c_pcie_rsrc_base_1, X"2468_ACE0", cpl_sta => c_cpl_sta_ur);
      memwr(clk, sv, rv, c_pcie_rsrc_base_0, X"DEAD_BEEF");
      memrd(clk, sv, rv, c_pcie_rsrc_base_1, X"9999_99EF", cpl_sta => c_cpl_sta_ur);
      wait_all_cplx_pending(clk, rv);
      
         -- Before a device can react to memory space or I/O space accesses, the 
         -- corresponding address space must be enabled in the PCI command register.
         -- (Again, more on this session 5) 
         -- Otherwise an end-point must reject every request with status UR 
         -- (PCI device would not activate DEVSEL)
      cfgwr0(clk, sv, rv, c_csreg_command, c_bsel_mem_space_en or c_bsel_io_space_en or c_bsel_bus_mst_en);

         -- Test I/O Space access
      iowr(clk, sv, rv, c_pcie_rsrc_base_1, X"2468_ACE0");
      iord(clk, sv, rv, c_pcie_rsrc_base_1, X"2468_ACE0");

         -- Test I/O Space access which should return UR status (not an I/O region)
      iowr(clk, sv, rv, c_pcie_rsrc_base_0, X"2468_ACE0", cpl_sta => c_cpl_sta_ur);
      iord(clk, sv, rv, c_pcie_rsrc_base_0, X"2468_ACE0", cpl_sta => c_cpl_sta_ur);
            
         -- Check that an unexpected completion does not hang up the device
      cpld(clk, sv, rv,
           (X"AAC2BE27", X"9D0C757D", X"DE32F3A0", X"340FE841", 
            X"007E19EC", X"DFCB6CB5", X"4BA5B212", X"F1ADA55C"));

         -- Test Memory Space access which should return UR status (not a mem region)
         -- Mem wr is a posted request and should just be ignored
      memwr(clk, sv, rv, c_pcie_rsrc_base_1, X"DEAD_BEEF");
      memrd(clk, sv, rv, c_pcie_rsrc_base_1, X"9999_99EF", cpl_sta => c_cpl_sta_ur);

      memwr(clk, sv, rv, c_pcie_rsrc_base_0, X"DEAD_BEEF");
      memwr(clk, sv, rv,
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), 
            X"FEDCBA98");

         --    ---      ---      ---      ---      ---          
         -- 1 DW Write / Reads, all BE combinations 
      memrd(clk, sv, rv, c_pcie_rsrc_base_0, X"9999_99EF", be_first => B"0001");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0, X"9999_BE99", be_first => B"0010");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0, X"99AD_9999", be_first => B"0100");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0, X"DE99_9999", be_first => B"1000");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0, X"9999_BEEF", be_first => B"0011");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0, X"DEAD_9999", be_first => B"1100");
       
         --    ---      ---      ---      ---      ---          
         -- 2 DW Write / Reads                            
      memwr(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"01234567", X"89ABCDEF"), 
            be_first => B"1100", be_last => B"0111");

      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"89ABCDEF"), 
            be_first => B"1100", be_last => B"0111");

      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1111", be_last => B"0111");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1110", be_last => B"1111");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1100", be_last => B"1111");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1000", be_last => B"1111");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1111", be_last => B"0011");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1111", be_last => B"0001");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1000", be_last => B"0001");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1100", be_last => B"0011");
      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"0123BEEF", X"FEABCDEF"), 
            be_first => B"1110", be_last => B"0111");

         --    ---      ---      ---      ---      ---          
         --    Modify Background
      memwr(clk, sv, rv, c_pcie_rsrc_base_0,  
            c_membg_4kb_0(0 to 2), 
            be_first => B"1000", be_last => B"0001");

      memrd(clk, sv, rv, c_pcie_rsrc_base_0,  
            c_membg_4kb_0(0 to 2), 
            be_first => B"1000", be_last => B"0001");
      
         --    ---      ---      ---      ---      ---          
         -- Zero length read
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4),
            X"00010203", be_first => B"0000");

      memwr(clk, sv, rv, c_pcie_rsrc_base_0,  
            c_membg_4kb_0(0 to 31),  
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, c_pcie_rsrc_base_0,  
            (X"01234567", X"89ABCDEF", X"12345678", X"9ABCDEF0"));
                              
          -- Single Byte writes        
      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"00000000", be_first => B"0001");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"89ABCD00", be_first => B"1111");

      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"000000FF", be_first => B"0010");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"89AB0000",  be_first => B"1111");

      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"0000FFFF", be_first => B"0100");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"89000000", be_first => B"1111");

      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"00FFFFFF", be_first => B"1000");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"00000000", be_first => B"1111");

         -- Null Byte Write
      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"FFFFFFFF", be_first => B"0000");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 4), X"00000000", be_first => B"1111");      

          -- Two Byte writes        
      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"00000000", be_first => B"0011");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"12340000", be_first => B"1111");

      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"FFFFFFFF", be_first => B"0110");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"12FFFF00", be_first => B"1111");

      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"5A5A5A5A", be_first => B"1100");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"5A5AFF00", be_first => B"1111");

      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"A5A5A5A5", be_first => B"1001");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 8), X"A55AFFA5", be_first => B"1111");
      
          -- Three Byte writes        
      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 16#C#), X"00000000", be_first => B"1110");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 16#C#), X"000000F0", be_first => B"1111");

      memwr(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 16#C#), X"FFFFFFFF", be_first => B"0111");
      memrd(clk, sv, rv, 
            std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 16#C#), X"00FFFFFF", be_first => B"1111");
                  

         -- 8-Byte Accesses
      v_addr_start   := std_logic_vector(unsigned(c_pcie_rsrc_base_0) + 16#1C#);
      memwr(clk, sv, rv, v_addr_start, 
            (X"BCDEF012",  X"456789AB"),
            be_first => B"1111", be_last => B"1111");
            
      memwr(clk, sv, rv, v_addr_start, 
            (X"00000000", X"00000000"),
            be_first => B"0001", be_last => B"1000");
      memrd(clk, sv, rv, v_addr_start, 
            (X"BCDEF000", X"006789AB"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"FFFFFFFF", X"FFFFFFFF"),
            be_first => B"1000", be_last => B"0001");
      memrd(clk, sv, rv, v_addr_start, 
            (X"FFDEF000", X"006789FF"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"A5A5A5A5", X"A5A5A5A5"),
            be_first => B"0101", be_last => B"1010");
      memrd(clk, sv, rv, v_addr_start, 
            (X"FFA5F0A5", X"A567A5FF"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"5A5A5A5A", X"C3C3C33C"),
            be_first => B"0110", be_last => B"1001");
      memrd(clk, sv, rv, v_addr_start, 
            (X"FF5A5AA5", X"C367A53C"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"96699669", X"1E711E71"),
            be_first => B"1001", be_last => B"0110");
      memrd(clk, sv, rv, v_addr_start, 
            (X"965A5A69", X"C3711E3C"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"27763731", X"B4DA390F"),
            be_first => B"1110", be_last => B"0001");
      memrd(clk, sv, rv, v_addr_start, 
            (X"27763769", X"C3711E0F"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"A19429C6", X"743E6827"),
            be_first => B"1000", be_last => B"0111");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A1763769", X"C33E6827"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"4F237DD1", X"479A13BD"),
            be_first => B"0001", be_last => B"0100");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A17637D1", X"C39A6827"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"8D479A4F", X"5874AF72"),
            be_first => B"0001", be_last => B"0010");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A176374F", X"C39AAF27"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"6724736B", X"84ABCC41"),
            be_first => B"0001", be_last => B"0001");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A176376B", X"C39AAF41"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"248C76B3", X"11A62537"),
            be_first => B"0010", be_last => B"0001");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A176766B", X"C39AAF37"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"8423BCFF", X"39F2E726"),
            be_first => B"0010", be_last => B"0010");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A176BC6B", X"C39AE737"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"A46DF133", X"774B2645"),
            be_first => B"0010", be_last => B"0100");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A176F16B", X"C34BE737"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"774FEE96", X"517455A9"),
            be_first => B"0010", be_last => B"1000");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A176EE6B", X"514BE737"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"28645D23", X"F7BDACE1"),
            be_first => B"0100", be_last => B"1000");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A164EE6B", X"F74BE737"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"667D5A26", X"E38A6DDD"),
            be_first => B"0100", be_last => B"0100");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A17DEE6B", X"F78AE737"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"1746229A", X"45256A23"),
            be_first => B"0100", be_last => B"0010");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A146EE6B", X"F78A6A37"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"39847A27", X"CC49A1FE"),
            be_first => B"0100", be_last => B"0001");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A184EE6B", X"F78A6AFE"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"7456224A", X"728B3D73"),
            be_first => B"1000", be_last => B"0001");
      memrd(clk, sv, rv, v_addr_start, 
            (X"7484EE6B", X"F78A6A73"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"FD892333", X"94FF3782"),
            be_first => B"1000", be_last => B"0010");
      memrd(clk, sv, rv, v_addr_start, 
            (X"FD84EE6B", X"F78A3773"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"58D1093F", X"7DBC1A92"),
            be_first => B"1000", be_last => B"0010");
      memrd(clk, sv, rv, v_addr_start, 
            (X"5884EE6B", X"F78A1A73"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"A94833DF", X"A5393B47"),
            be_first => B"1000", be_last => B"0100");
      memrd(clk, sv, rv, v_addr_start, 
            (X"A984EE6B", X"F7391A73"),
            be_first => B"1111", be_last => B"1111");

      memwr(clk, sv, rv, v_addr_start, 
            (X"4762D1B6", X"8827B13D"),
            be_first => B"1000", be_last => B"1000");
      memrd(clk, sv, rv, v_addr_start, 
            (X"4784EE6B", X"88391A73"),
            be_first => B"1111", be_last => B"1111");
            
         -- Block Read                  
      memwr(clk, sv, rv, c_pcie_rsrc_base_0,  
            c_membg_4kb_0(0 to 15), 
            be_first => B"1111", be_last => B"1111");
                       
      for i in 0 to 63 loop      
         memwr(clk, sv, rv, 
               std_logic_vector(unsigned(c_pcie_rsrc_base_0) + (16#0200# - (i * 4))),  
               c_membg_4kb_0(0 to i)); 
         memrd(clk, sv, rv,
               std_logic_vector(unsigned(c_pcie_rsrc_base_0) + (16#0200# - (i * 4))),  
               c_membg_4kb_0(0 to i));                    
      end loop;
                  
         -- Wait until all UpdateFCs have been sent (max 4095 Clock Cycles)                    
      idle(clk, 5192);
      show_credits(rv);
      --stop(0);
   end procedure; 

End pcie_vhdl_test_case_pkg;      
