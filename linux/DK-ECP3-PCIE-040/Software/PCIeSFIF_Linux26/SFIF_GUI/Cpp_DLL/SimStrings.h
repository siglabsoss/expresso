#ifndef SIM_STRINGS_H
#define SIM_STRINGS_H

char *Sim_DriverVersionStr = "*** Simulated Driver Hardware Access v0.1 ***";         


char *Sim_CfgRegStr = \
"*** SIMULATION ***\n"
"00000000:  04  12  03  53  07  00  10  00    01  00  00  00  00  00  00  00\n"
"00000010:  00  00  ec  d7  00  00  e8  d7    00  00  00  00  00  00  00  00\n"
"00000020:  00  00  00  00  00  00  00  00    00  00  00  00  04  12  30  30\n"
"00000030:  00  00  00  00  50  00  00  00    00  00  00  00  12  01  00  00\n"
"[00]Vendor ID: 1204\n"
"[02]Device ID: 5303\n"
"[04]Command: 7 = [ BusMstr Mem IO]\n"
"[06]Status: 10 = [ CapList]\n"
"[08]Rev ID: 1\n"
"[09]Class Code: 000\n"
"[0c]Cache Line Size: 0\n"
"[0d]Latency Timer: 0\n"
"[0e]Header Type: 0\n"
"[0f]BIST: 0\n"
"[10]BAR0: d7ec0000\n"
"[14]BAR1: d7e80000\n"
"[18]BAR2: 0\n"
"[1c]BAR3: 0\n"
"[20]BAR4: 0\n"
"[24]BAR5: 0\n"
"[28]Cardbus CIS Ptr: 0\n"
"[2c]Subsystem Vendor ID: 1204\n"
"[2e]Subsystem ID: 3030\n"
"[30]Exp ROM: 0\n"
"[34]Capabilities Ptr: 50\n"
"[3c]Interrupt Line: 12\n"
"[3d]Interrupt Pin: 1\n"
"[3e]Min Grant: 0\n"
"[3f]Max Latency: 0\n";


char *Sim_CapRegStr = \
"*** SIMULATION ***\n"
"Power Management Capability Structure @ 0x50\n"
"	PwrCap: 0x3 = [PME: ver=3]\n"
"	PMCSR: 0x0 = [ state=D0]\n"
"	Data: 0x0\n\n"
"MSI Capability Structure @ 0x70\n"
"	MsgCtrlReg: 0x80 = [64bitAddr numMsgs=1 reqMsgs=1 ]\n"
"	MsgAddr: 0x00\n"
"	MsgData: 0x0\n\n"
"PCI Express Capability Structure @ 0x90\n"
"	PCIe_Cap: 0x1 = [ IntMsg#=0 type=0 ver=1]\n"
"	Dev_Cap: 0x8002 = [ L1Lat=1us L0sLat=64ns MaxTLPSize=512]\n"
"	Dev_Ctrl: 0x2010 = [ MaxRdSize=512 MaxTLPSize=128 RlxOrd]\n"
"	Dev_Stat: 0x0 = []\n"
"	Link_Cap: 0xc41 = [ Port#=0 L1Lat=1us L0sLat=64ns ASPM:L1+L0s Width=x4 2.5G]\n"
"	Link_Ctrl: 0xc41 = [ RCB=64 ASPM:Disabled]\n"
"	Link_Stat: 0x0 = [ Width=x4 2.5G]\n";




char *Sim_ExtrInfoStr = \
"*** SIMULATION ***\n"
"lscpcie Driver Extra Info:\n"
"DevID=1\n"
"PCI Bus#=2\n"
"PCI Dev#=0\n"
"PCI Func#=0\n"
"UINumber=0\n"
"hasDMA=1\n"
"DmaBufSize=20480\n"
"DmaAddr64=0\n"
"DmaPhyAddrHi=0\n"
"DmaPhyAddrLo=57c0000\n\n"
"Root Complex Initial Credits:\n"
"PD_CA (Wr): 32\n"
"NPH_CA(Rd): 16\n";
 
char *Sim_PCIResourceStr = "Simulated PCI BAR and IRQ";

char *Sim_DriverResourcesStr = \
"*** SIMULATION ***\n"
"Number of BARs: 2\n"
"BAR0:  Addr: d7ec0000  Size: 262144  Mapped:1\n"
"BAR1:  Addr: d7e80000  Size: 262144  Mapped:1\n"
"Interrupt Vector: 355\n";



#endif
