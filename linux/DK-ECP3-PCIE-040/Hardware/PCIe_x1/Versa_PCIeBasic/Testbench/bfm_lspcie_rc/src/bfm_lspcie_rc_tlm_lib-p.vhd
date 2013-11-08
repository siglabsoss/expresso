
--
--------------------------------------------------------------------------------
--
-- File ID     : $Id: bfm_lspcie_rc_tlm_lib-p.vhd 1396 2011-02-26 22:23:29Z  $
-- Generated   : $LastChangedDate: 2011-02-26 23:23:29 +0100 (Sa, 26. Feb 2011) $
-- Revision    : $LastChangedRevision: 1396 $
--
--------------------------------------------------------------------------------

Library IEEE;

Use IEEE.std_logic_1164.all;

Use WORK.bfm_lspcie_rc_constants_pkg.all;
Use WORK.bfm_lspcie_rc_types_pkg.all;

Package bfm_lspcie_rc_tlm_lib_pkg is
   function get_cpl_buffer(rv    : t_bfm_resp;
                           index : natural := 0) return std_logic_vector;

   function get_mem_buffer(rv    : t_bfm_resp;
                           index : natural := 0) return std_logic_vector;
                                                      
      -- Configuration read
   procedure cfgrd0(signal clk         : in    std_logic;
                    signal sv          : inout t_bfm_stim;
                    signal rv          : in    t_bfm_resp;
                           addr        : in    std_logic_vector;
                           data        : in    std_logic_vector;
                           be_first    : in    std_logic_vector := B"1111";
                           cpl_wait    : in    boolean := true;
                           cpl_sta     : in    std_logic_vector := c_cpl_sta_sc;
                           no_compare  : in    boolean := false);
                           
      -- Configuration write
   procedure cfgwr0(signal clk      : in    std_logic;
                    signal sv       : inout t_bfm_stim;
                    signal rv       : in    t_bfm_resp;
                           addr     : in    std_logic_vector;
                           data     : in    std_logic_vector;
                           be_first : in    std_logic_vector := B"1111";
                           cpl_wait : in    boolean := false;
                           cpl_sta  : in    std_logic_vector := c_cpl_sta_sc);

      -- Cpl. Intended as unexpected Completion
   procedure cpl(signal clk      : in    std_logic;        
                 signal sv       : inout t_bfm_stim;      
                 signal rv       : in    t_bfm_resp;
                        req_id   : in    std_logic_vector := X"0008";
                        tag      : in    std_logic_vector := X"00");
                         
      -- Cpld. Intended as unexpected Completion with one DW payload
   procedure cpld(signal clk     : in    std_logic;        
                  signal sv      : inout t_bfm_stim;      
                  signal rv      : in    t_bfm_resp;      
                         data    : in    std_logic_vector;
                         req_id  : in    std_logic_vector := X"0008";
                         tag     : in    std_logic_vector := X"00");
                          
      -- Cpld. Intended as unexpected Completion with payload
   procedure cpld(signal clk     : in    std_logic;        
                  signal sv      : inout t_bfm_stim;      
                  signal rv      : in    t_bfm_resp;      
                         data    : in    t_tlp_payload;
                         req_id  : in    std_logic_vector := X"0008";
                         tag     : in    std_logic_vector := X"00");
                                                     
   procedure idle(signal clk     : in    std_logic;
                         count   : in    natural := 1);

      -- I/O Space read
   procedure iord(signal clk        : in    std_logic;        
                  signal sv         : inout t_bfm_stim;      
                  signal rv         : in    t_bfm_resp;      
                         addr       : in    std_logic_vector; 
                         data       : in    std_logic_vector;
                         be_first   : in    std_logic_vector := B"1111";
                         cpl_wait   : in    boolean := true; 
                         cpl_sta    : in    std_logic_vector := c_cpl_sta_sc;
                         no_compare : in    boolean := false);
                           
      -- I/O Space write
   procedure iowr(signal clk        : in    std_logic;        
                  signal sv         : inout t_bfm_stim;      
                  signal rv         : in    t_bfm_resp;      
                         addr       : in    std_logic_vector;
                         data       : in    std_logic_vector;
                         be_first   : in    std_logic_vector := B"1111";
                         cpl_wait   : in    boolean := false; 
                         cpl_sta    : in    std_logic_vector := c_cpl_sta_sc);

      -- Memory read
   procedure memrd(signal clk          : in    std_logic;        
                   signal sv           : inout t_bfm_stim;      
                   signal rv           : in    t_bfm_resp;      
                          addr         : in    std_logic_vector; 
                          data         : in    std_logic_vector;
                          be_first     : in    std_logic_vector := B"1111";
                          cpl_wait     : in    boolean := true; 
                          cpl_sta      : in    std_logic_vector := c_cpl_sta_sc;
                          no_compare   : in    boolean := false);
                          
   procedure memrd(signal clk          : in    std_logic;        
                   signal sv           : inout t_bfm_stim;      
                   signal rv           : in    t_bfm_resp;      
                          addr         : in    std_logic_vector; 
                          data         : in    t_tlp_payload;
                          be_first     : in    std_logic_vector := B"1111";
                          be_last      : in    std_logic_vector := B"1111"; 
                          cpl_wait     : in    boolean := true; 
                          cpl_sta      : in    std_logic_vector := c_cpl_sta_sc;
                          no_compare   : in    boolean := false);

      -- Locked Memory read
   procedure memrd_lk(signal clk          : in    std_logic;        
                      signal sv           : inout t_bfm_stim;      
                      signal rv           : in    t_bfm_resp;      
                             addr         : in    std_logic_vector; 
                             data         : in    std_logic_vector;
                             be_first     : in    std_logic_vector := B"1111";
                             cpl_wait     : in    boolean := true; 
                             cpl_sta      : in    std_logic_vector := c_cpl_sta_sc;
                             no_compare   : in    boolean := false);
                          
   procedure memrd_lk(signal clk          : in    std_logic;        
                      signal sv           : inout t_bfm_stim;      
                      signal rv           : in    t_bfm_resp;      
                             addr         : in    std_logic_vector; 
                             data         : in    t_tlp_payload;
                             be_first     : in    std_logic_vector := B"1111";
                             be_last      : in    std_logic_vector := B"1111"; 
                             cpl_wait     : in    boolean := true; 
                             cpl_sta      : in    std_logic_vector := c_cpl_sta_sc;
                             no_compare   : in    boolean := false);
                                                     
      -- Memory write
   procedure memwr(signal clk       : in    std_logic;        
                   signal sv        : inout t_bfm_stim;      
                   signal rv        : in    t_bfm_resp;      
                          addr      : in    std_logic_vector; 
                          data      : in    std_logic_vector; 
                          be_first  : in    std_logic_vector := B"1111");
                          
   procedure memwr(signal clk       : in    std_logic;        
                   signal sv        : inout t_bfm_stim;      
                   signal rv        : in    t_bfm_resp;      
                          addr      : in    std_logic_vector; 
                          data      : in    t_tlp_payload; 
                          be_first  : in    std_logic_vector := B"1111";
                          be_last   : in    std_logic_vector := B"1111");

      -- Generic Message w/o payload
   procedure msg(signal clk      : in    std_logic;  
                 signal sv       : inout t_bfm_stim;
                 signal rv       : in    t_bfm_resp;
                        msg_code : in    std_logic_vector;
                        routing  : in    std_logic_vector := c_route_local_rcv_term;
                        hdr_dw2  : in    std_logic_vector := X"00000000";
                        hdr_dw3  : in    std_logic_vector := X"00000000");
                                                                 
      -- PM Active State NAK Message
   procedure msg_pm_active_state_nak(signal clk : in    std_logic;  
                                     signal sv  : inout t_bfm_stim;
                                     signal rv  : in    t_bfm_resp);

      -- PME Turn Off Message
   procedure msg_pme_turn_off(signal clk  : in    std_logic;  
                              signal sv   : inout t_bfm_stim;
                              signal rv   : in    t_bfm_resp);
                              
      -- Unlock Message
   procedure msg_unlock(signal clk  : in    std_logic;  
                        signal sv   : inout t_bfm_stim;
                        signal rv   : in    t_bfm_resp);

      -- Vendor defined Message w/o Payload
   procedure msg_vendor_defined(signal clk      : in    std_logic;  
                                signal sv       : inout t_bfm_stim;
                                signal rv       : in    t_bfm_resp;
                                       vend_id  : in    std_logic_vector;
                                       vend_dw  : in    std_logic_vector := X"0000";
                                       routing  : in    std_logic_vector := c_route_local_rcv_term;
                                       type_id  : in    natural := 1);

      -- Generic Message with single DWord payload
   procedure msgd(signal clk        : in    std_logic;                                  
                  signal sv         : inout t_bfm_stim;                                 
                  signal rv         : in    t_bfm_resp;                                 
                         msg_code   : in    std_logic_vector;                           
                         data       : in    std_logic_vector; 
                         routing    : in    std_logic_vector := c_route_local_rcv_term; 
                         hdr_dw2    : in    std_logic_vector := X"00000000";            
                         hdr_dw3    : in    std_logic_vector := X"00000000");
                         
      -- Generic Message with payload
   procedure msgd(signal clk        : in    std_logic;                                  
                  signal sv         : inout t_bfm_stim;                                 
                  signal rv         : in    t_bfm_resp;                                 
                         msg_code   : in    std_logic_vector;                           
                         data       : in    t_tlp_payload; 
                         routing    : in    std_logic_vector := c_route_local_rcv_term; 
                         hdr_dw2    : in    std_logic_vector := X"00000000";            
                         hdr_dw3    : in    std_logic_vector := X"00000000");
                                                
      -- Set Slot Power Limit Message
   procedure msgd_set_slot_power_limit(signal clk     : in    std_logic;  
                                       signal sv      : inout t_bfm_stim;
                                       signal rv      : in    t_bfm_resp;
                                              scale   : in    std_logic_vector := "00";
                                              value   : in    std_logic_vector := X"00");                  

      -- Vendor defined Message with payload packet
   procedure msgd_vendor_defined(signal clk     : in    std_logic;  
                                 signal sv      : inout t_bfm_stim;
                                 signal rv      : in    t_bfm_resp;
                                        vend_id : in    std_logic_vector;
                                        data    : in    std_logic_vector; 
                                        vend_dw : in    std_logic_vector := X"0000";
                                        routing : in    std_logic_vector := c_route_local_rcv_term;
                                        type_id : in    natural := 1);
                                        
      -- Vendor defined Message with payload packet
   procedure msgd_vendor_defined(signal clk     : in    std_logic;  
                                 signal sv      : inout t_bfm_stim;
                                 signal rv      : in    t_bfm_resp;
                                        vend_id : in    std_logic_vector;
                                        data    : in    t_tlp_payload; 
                                        vend_dw : in    std_logic_vector := X"0000";
                                        routing : in    std_logic_vector := c_route_local_rcv_term;
                                        type_id : in    natural := 1);
                                       
   procedure set_dut_id(signal sv      : inout t_bfm_stim;
                               bus_nr  : in natural := 0;
                               dev_nr  : in natural := 1;
                               func_nr : in natural := 0);

      -- Show credits currently advertised from DUT
   procedure show_credits(signal rv    : in  t_bfm_resp);

      -- Update completion credit info
   procedure svc_ca_cplx(signal clk    : in    std_logic;
                         signal sv     : inout t_bfm_stim;
                                cplh   : natural := 0;
                                cpld   : natural := 0);
                              
      -- Update non-posted credit info
   procedure svc_ca_np(signal clk   : in    std_logic;
                       signal sv    : inout t_bfm_stim;
                              nph   : natural := 0;
                              npd   : natural := 0);
                       
      -- Update posted credit info
   procedure svc_ca_p(signal clk : in    std_logic;
                      signal sv  : inout t_bfm_stim;
                             ph  : natural := 0;
                             pd  : natural := 0);

   procedure sys_memrd(signal clk         : in    std_logic;      
                       signal sv          : inout t_bfm_stim;     
                       signal rv          : in    t_bfm_resp;     
                              addr        : in    std_logic_vector; 
                              data        : in    std_logic_vector; 
                              be_first    : in    std_logic_vector := B"1111";
                              no_compare  : in    boolean := false);
                          
   procedure sys_memrd(signal clk         : in    std_logic;     
                       signal sv          : inout t_bfm_stim;   
                       signal rv          : in    t_bfm_resp;   
                              addr        : in    std_logic_vector; 
                              data        : in    t_tlp_payload; 
                              be_first    : in    std_logic_vector := B"1111";
                              be_last     : in    std_logic_vector := B"1111";
                              no_compare  : in    boolean := false);
                              
   procedure sys_memwr(signal clk      : in    std_logic;      
                       signal sv       : inout t_bfm_stim;     
                       signal rv       : in    t_bfm_resp;     
                              addr     : in    std_logic_vector; 
                              data     : in    std_logic_vector; 
                              be_first : in    std_logic_vector := B"1111");
                          
   procedure sys_memwr(signal clk      : in    std_logic;     
                       signal sv       : inout t_bfm_stim;   
                       signal rv       : in    t_bfm_resp;   
                              addr     : in    std_logic_vector; 
                              data     : in    t_tlp_payload; 
                              be_first : in    std_logic_vector := B"1111";
                              be_last  : in    std_logic_vector := B"1111");
                                                
      -- Wait until all non-posted requests have completed
   procedure wait_all_cplx_pending(signal clk   : in std_logic;
                                   signal rv    : in    t_bfm_resp);                      
End bfm_lspcie_rc_tlm_lib_pkg;
