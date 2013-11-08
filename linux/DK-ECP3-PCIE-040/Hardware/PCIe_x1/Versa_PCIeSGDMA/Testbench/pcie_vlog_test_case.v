`timescale 1ns / 1 ps
module pcie_vlog_test_case;
`include "bfm_lspcie_rc_tlm_lib.v"

         // define the base address register settings to use in this test                       
   localparam  [31:0] c_pcie_rsrc_base_0  = 32'hfedc0000; 
   localparam  [31:0] c_pcie_rsrc_base_1  = 32'h9abc0000; 
   
   integer     ix;
   
   initial begin
      #1;
         // useful for checking in the console window that the correct language 
         // is on use
      $display("\nUsing Verilog Test Case");

         // Set up the bus no, func no. and device no in the DUT
         // The function no. should always be set to zero 
         // (no multi-function device)
         // This should be varied when running multiple tests to check that
         // the user hardware is picking up the correct values from the Lattice
         // core
      set_dut_id(2, 1, 0);
      
         // increase the default values for credit-based flow control in the BFM
         // (More on this in session 3. For session 2, these lines can be omitted)
      svc_ca_cplx(7, 56);
      svc_ca_p(7, 56);
      svc_ca_np(7, 0);

         // To communicate with an end-point, the end-point resources (e.g. registers)
         // must be mapped into PCI Express memory or I/O space
         // This will be discussed in more detail in session 5
         // To summarise the initialisation scenario as it runs in the real world, 
         // First, all Base address registers are written with all ones
         // Note: the very first PCI Express communication to a device MUST be a 
         // configuration space write (sets Bus no, dev no, funcno in device) 
      cfgwr0(c_csreg_bar0, 32'hffffffff, c_be_all, c_wait_cpl, c_cpl_sta_sc);
      cfgwr0(c_csreg_bar1, 32'hffffffff, c_be_all, c_wait_cpl, c_cpl_sta_sc);
      cfgwr0(c_csreg_bar2, 32'hffffffff, c_be_all, c_wait_cpl, c_cpl_sta_sc);
      cfgwr0(c_csreg_bar3, 32'hffffffff, c_be_all, c_wait_cpl, c_cpl_sta_sc);
      cfgwr0(c_csreg_bar4, 32'hffffffff, c_be_all, c_wait_cpl, c_cpl_sta_sc);
      cfgwr0(c_csreg_bar5, 32'hffffffff, c_be_all, c_wait_cpl, c_cpl_sta_sc);    

         // Typically, the BIOS /System SW would try and identify the device
         // Here, we don't want to check against an expected value so use
         // cfg_poll() instead of cfgrd0() 
      cfg_poll(c_csreg_vend_id, c_be_all, c_wait_cpl);

         // Typically, the BIOS /System SW would read back the Base Address registers
         // written above to determine the resource window size (again, more on this in
         // session 5)
      cfg_poll(c_csreg_bar0, c_be_all, c_wait_cpl);
      cfg_poll(c_csreg_bar1, c_be_all, c_wait_cpl);
      cfg_poll(c_csreg_bar2, c_be_all, c_wait_cpl);
      cfg_poll(c_csreg_bar3, c_be_all, c_wait_cpl);
      cfg_poll(c_csreg_bar4, c_be_all, c_wait_cpl);
      cfg_poll(c_csreg_bar5, c_be_all, c_wait_cpl);  
      
         // BIOS or system sw must then locate the device within the system memory map
         // by writing a base address to the base-address register.
         // In this program, the base address is a constant assigned at the head of the 
         // test program. To rerun the test with different BAR settings, you only need
         // to modify the constant above.
      cfgwr0(c_csreg_bar0, c_pcie_rsrc_base_0, c_be_all, c_wait_cpl, c_cpl_sta_sc);
      cfgwr0(c_csreg_bar1, c_pcie_rsrc_base_1, c_be_all, c_wait_cpl, c_cpl_sta_sc);

      // see that the value of bar0 is now updated to	"c_pcie_rsrc_base_0  = 32'hfedc0000" 
      cfg_poll(c_csreg_bar0, c_be_all, c_wait_cpl);

         // Modern Bioses /operating systems send the Set Slot Power Limit message after 
         // initial configuration is complete. (More on this in session 3 and 4)
      msgd_set_slot_power_limit(8'd2, 2'd1);

         // Before a device can react to memory space or I/O space accesses, the 
         // corresponding address space must be enabled in the PCI command register.
         // (Again, more on this session 5) 
         // Otherwise an end-point must reject every request with status UR 
         // (PCI device would not activate DEVSEL)
      cfgwr0(c_csreg_command, c_bsel_mem_space_en | c_bsel_bus_mst_en, 
             c_be_all, c_wait_cpl, c_cpl_sta_sc);
             
         // For Basic demo, assign a reload value to the counter at memory space
         // offset 0x14
      memwr(c_pcie_rsrc_base_0 + 8'h14, 32'h00000fff, 1, c_be_all, c_be_none);

         // Check that an unexpected completion does not hang up the device
      cpld({32'h340fe841, 32'haac2be27, 32'h9d0c757d, 32'hde32f3a0, 
            32'hdfcb6cb5, 32'h007e19ec, 32'h4ba5b212, 32'hf1ada55c}, 8, 16'hff08, 8'd9);
      
         // Start the timer in the basic demo by writing to the 'Down Counter
         // Control' register at offset 0x0C
      memwr(c_pcie_rsrc_base_0 + 8'h0c, 32'h00000003, 1, c_be_all, c_be_none);
      
         // Read the counter register at offset 0x10 a few times and observe
         // the counter running. We don't know what the expected values should
         // be (counter is running) so use mem_poll() instead of memrd()
      for (ix = 0; ix < 8; ix++) 
         mem_poll(c_pcie_rsrc_base_0 + 8'h10, 1, 
                  c_be_all, c_be_none, c_wait_cpl);
                      
      idle(8);
      show_credits;
      $finish(2);
      end
endmodule
