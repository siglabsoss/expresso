/** @file lscdma.h */

#ifndef LSC_LSCDMA_H
#define LSC_LSCDMA_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <linux/tty.h>
#include <linux/selection.h>
#include <linux/kmod.h>

/* For devices that use/implement shared memory (mmap) */
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/pci.h>
#include <linux/mman.h>

#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/wait.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/pgalloc.h>

#include "Ioctl.h"
#include "Devices.h"
#include "GPIO.h"
#include "SGDMA.h"




// Some Useful defines
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef OK
#define OK 0
#endif
#ifndef ERROR
#define ERROR -1
#endif
#ifndef WAITING
#define WAITING -2
#endif

/* Change these defines to increase number of boards supported by the driver */

#define NUM_BARS MAX_PCI_BARS  /* defined in Ioctl.h */
#define NUM_BOARDS 4          // 4 PCIe boards per system is a lot of PCIe slots and eval boards to have on hand
#define MAX_BOARDS (NUM_BOARDS) 
#define MINORS_PER_BOARD 4     // 3 minor number per discrete device
#define MAX_MINORS (MAX_BOARDS * MINORS_PER_BOARD)  


#define DMA_BUFFER_SIZE (64 * 1024)


// Note: used as indexes into lists of strings
#define SC_BOARD    1
#define ECP2M_BOARD 2
#define ECP3_BOARD  3
#define DMA_DEMO  1

// NOTE: these assume the first minor number is always 0, which is probably safe ;-)
#define  LSCDMA_MINOR_TO_BOARD(a)     (a / MINORS_PER_BOARD)
#define  LSCDMA_MINOR_TO_FUNCTION(a)  (a & (MINORS_PER_BOARD - 1))


// The minor number tells which function of the device to open
#define MEM_ACCESS_NUM 0
#define COLOR_BARS_NUM 1
#define IMAGE_MOVE_NUM 2

// IP Register defines
#define STAT_CMD_REG  1
#define CAP_PTR_REG  13


/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*           DATA TYPES
 */
/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*-------------------------------------------------*/

// Current configuration of the SGDMA IP core
#define NUM_SGDMA_CHANNELS 2
#define DMA_WR_CHAN 0
#define DMA_RD_CHAN 1



/**
 * Info about the device used for the read or write operation.
 * DMA parameters to program into channel and BDs to perform
 * the operation.
 */
typedef struct
{
        ULONG baseAddr;   /**< Wishbone base address */
        ULONG dataWidth;  /**< 0=8bit, 1=16bit, 2=32bit, 3=64bit */
        ULONG addrMode;   /**<  0=FIFO, 1=linear, 2=loop */
        ULONG maxAddrRange;  /**< maximum number of bytes that can be read or written */
} DevMemParams_t;

/**
 * SGDMA channel information for keeping track of what's being transferred
 */
typedef struct
{
	struct semaphore dmaMutex;   /**< only let one thread run one read/write at a time */

        bool doneDMA;       /**< set to true by ISR when DMA is completed.  Wakes user task. */
        bool cancelDMA;     /**< set to true if read() or write() aborts to notify ISR to do nothing */
	bool xferStatus;    /**< marked as ERROR or OK by ISR */

        ULONG elapsedTime;      /**< GPIO 125 MHz count of time to complete xfer */
        ULONG totalXferSize;    /**< the original, full amount the user wants moved */
        ULONG xferedSoFar;      /**< accumulating count of what's been moved so far */
        ULONG thisXferSize;      /**< count of what's being moved right now by DMA */
        void *bufBaseVirtualAddr;  /**< base virtual address of caller's buffer */

        bool isDmaAddr64;       /**< set to true if asked for 64 bit address support */

        ULONG numBDs;    /**< how many buffer descriptors this channel has */
        ULONG startBD;   /**< whats the starting BD  */

        DevMemParams_t DevMem;   /**< access config to device memory - base Addr, len, size, etc.
                                      used in setting up the BDs for xfer */

	struct page **pageList;   /**< list of virtual pages that make up the user space buffer */
	uint32_t numPages;        /**< length of pageList */
	struct scatterlist *sgList;  /**< the list of physical pages to program into the BD's */
	uint32_t sgLen;              /**< length of sgList */
	int	direction;           /**< PCI_DMA_TODEVICE or PCI_DMA_FROMDEVICE */


} DMAChannel_t;


/**
 * Information to manage the Scatter Gather DMA operations.
 */
typedef struct
{

        bool DmaOk;    /**< true if adapters and everything allocated, false if don't use it */

        ULONG WriteBurstSize;  /**< the maximum TLP size for a write (normally 128) */
        ULONG ReadBurstSize;  /**< the maximum read request size (normally 512) */

        bool Reading;   /**< set to true when DMA is running, DPC processing SG list */
        bool Writing;   /**< set to true when DMA is running, DPC processing SG list */
        bool ReadChanOpen;  /**< true if CreateFile opens a device that uses the read channel (chan1) */
        bool WriteChanOpen; /**<  true if CreateFile opens a device that uses the write channel (chan0) */

        DMAChannel_t Chan[NUM_SGDMA_CHANNELS];   /**< track the SGDMA BDs used for the tranfer */

	wait_queue_head_t ReadWaitQ;  /**< park user's read() request here while DMA'ing */
	wait_queue_head_t WriteWaitQ; /**< park user's write() request here while DMA'ing */

	ULONG ipVer;  /**< SGDMA IP core version for register map changes */

} SGDMAOperations_t;


/**
 * Information to manage the Common Buffer DMA operations.
 * The Common Buffer is a contiguous range of kernel memory that can be DMA'ed 
 * directly to from the hardware device.  It doesn't require page locking and mapping.
 * Important info is the size, hardware address (to progrma into PCI device) and CPU
 * address for the driver to access the memory.
 */
typedef struct
{
	bool		hasDMA;        /**< true = allocated a buffer for DMA transfers by SFIF */
        bool            isDmaAddr64;   /**< set to true if asked for 64 bit address support */
	size_t		dmaBufSize;    /**< size in bytes of the allocated kernel buffer */	
	dma_addr_t 	dmaPCIBusAddr; /**< PCI bus address to access the DMA buffer - program into board */
	void		*dmaCPUAddr;   /**< CPU (software) address to access the DMA buffer - use in driver */

} CBDMAOperations_t;



/**
 * This is the private data for each board's BAR that is mapped in.
 * NOTE: each structure MUST have minor as the first entry because it
 * it tested by a void * to see what BAR it is - See mmap()
 */
typedef struct PCI_Dev_BAR
{
	int      	 bar;
	void         *pci_addr; /**< the physical bus address of the BAR (assigned by PCI system), used in mmap */
	void         *kvm_addr; /**< the virtual address of a BAR mapped into kernel space, used in ioremap */
	int          memType;
	int          dataSize;
	unsigned long len;
	unsigned long pci_start;   /* info gathered from pci_resource_*() */
	unsigned long pci_end;
	unsigned long pci_flags;
} pci_dev_bar_t;


/**
 * The data about each unique Board (device) in the system controlled by
 * this driver.
 */
typedef struct PCIE_Board
{
        //========== Driver and Device Information =============
	u32     ID;            /**< PCI device ID of the board (0x5303, 0xe235, etc) */
	u32     demoID;        /**< PCI subsys device ID of the board (0x3030, 0x5303, etc) */
	u32 	demoType;      /**< DMA demo ID only for this driver */
	u32     function;      /**< 0=mem access (DMAtest), 1=ColorBars, 2=ImageMove */
	u32	boardType;     /**< SC or ECP2M device ID */
	u32     instanceNum;   /**< tracks number of identical board,demo devices in system */

        //========== Driver Mutex Controls =============
	atomic_t OpenToken;    /**< atomic counter to ensure only 1 user opens board at a time */
	spinlock_t hdwAccess;  /**< spinlock to provide mutex protection when modifying SGMDA and IntCtrl regs */


        //========== PCI Enumeration and Link Info =========
	u32	majorNum;      /**< copy of driver's Major number for use in places where only device exists */
	u32	minorNum;      /**< specific Minor number asigned to this board */
	u32	PCIeMaxPayloadSize;  /**< PCIe max payload size for MWr into PC memory */
	u32	PCIeMaxReadReqSize;  /**< PCIe max read size for MRd from PC memory */
	u32	PCIeRCBSize;         /**< Root Complex Read Completion size (normally 64 bytes) */
	u32	PCIeLinkWidth;       /**< PCIe link width: x1, x4 */
	u32	PCICfgRegs[256/4];  /**< store the standard PCI config registers to parse for link settings */



        //=========== INTERRUPT INFO ===============
	int     IRQ;           /**< -1 if no interrupt support, otherwise the interrupt line/vector */
	bool	msi;		/**< true if MSI interrupts have been enabled, false if regular INTx */
        ULONG	InterruptCounter;   /**< incremented in ISR for each interrupt serviced */
        bool	InterruptsEnabled;  /**< true if device interrupts are turned on */
	struct tasklet_struct  isrTasklet;  /**< used to defer processing to thread in kernel context */



	//================ BAR Assignments =============
	pci_dev_bar_t Dev_BARs[NUM_BARS];  /**< database of valid, mapped BARs belonging to this board */
	u32     numBars;       /**< number of valid BARs this board has, used to limit access into Dev_BARs[] */
	u32	mmapBAR;          /**< which BAR is used for mmap into user space.  Can change with IOCTL call */
	u32	ctrlBAR;          /**< which BAR is used for control access, i.e. lighting LEDs, masking Intrs */
	void	*ctrlBARaddr;	  /**< above BAR memory ioremap'ed into driver space to access */



        //========= LINUX DEVICE TYPES =============
	struct  pci_dev *pPciDev;  /**< pointer to the PCI core representation of the board */

	struct cdev	charDev; /**< the character device implemented by this driver */


        //========= SCATTER GATHER DMA INFO =============
        SGDMAOperations_t SGDMA;

        //========= COMMON BUFFER DMA INFO =============
        CBDMAOperations_t CBDMA;



} pcie_board_t;



/**
 * The main container of all the data structures and elements that comprise the
 * lscdma device driver.  Main elements are the array of detected eval boards,
 * the sysfs class, the assigned driver number.
 */
struct LSCDMA
{

	dev_t 	drvrDevNum;      /**> starting [MAJOR][MINOR] device number for this driver */
	u32 	numBoards;       /**> total number of boards controlled by driver */

	u8	numSC_Basic;     /**> count of number of SC Basic boards found */
	u8	numSC_SFIF;      /**> count of number of SC SFIF boards found */
	u8	numECP2M_Basic;      /**> count of number of ECP2M Basic boards found */
	u8	numECP2M_SFIF;      /**> count of number of ECP2M SFIF boards found */
	u8	numECP3_Basic;      /**> count of number of ECP3 Basic boards found */
	u8	numECP3_SFIF;      /**> count of number of ECP3 SFIF boards found */
	

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12))
	struct class *sysClass;   /**> the top entry point of lscdma in /sys/class */
#else
	struct class_simple *sysClass;   /**> the top entry point of lscdma in /sys/class */
#endif


	pcie_board_t Board[NUM_BOARDS];  /**> Database of LSC PCIe Eval Boards Installed */

};




/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*
 *            FUNCTION PROTOTYPES
 */
/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*-------------------------------------------------*/
extern int DrvrDebug;


bool IsrEnableInterrupts(pcie_board_t *pBrd);
bool IsrDisableInterrupts(pcie_board_t *pBrd);
irqreturn_t	lscdma_ISR(int irq, void *dev_id, struct pt_regs *regs);
void lscdma_isr_tasklet(unsigned long);


void StartDmaWrite(pcie_board_t *pBrd,
		   DMAChannel_t *pChan);
void StartDmaRead(pcie_board_t *pBrd,
		   DMAChannel_t *pChan);
int initDMAChan(pcie_board_t *pBrd,
		DMAChannel_t *pChan,
		unsigned long userAddr,
		size_t len,
		int direction);
int releaseDMAChan(pcie_board_t *pBrd,
		DMAChannel_t *pChan);



UCHAR rdReg8(pcie_board_t *pBrd, ULONG offset);
USHORT rdReg16(pcie_board_t *pBrd, ULONG offset);
ULONG rdReg32(pcie_board_t *pBrd, ULONG offset);
void wrReg8(pcie_board_t *pBrd, ULONG offset, UCHAR val);
void wrReg16(pcie_board_t *pBrd, ULONG offset, USHORT val);
void wrReg32(pcie_board_t *pBrd, ULONG offset, ULONG val);
int ReadPCIConfigRegs(pcie_board_t *pBrd);
int ParsePCIeLinkCap(pcie_board_t *pBrd);
int GPIO_Setup(pcie_board_t *pBrd);
int IntrCtrl_Setup(pcie_board_t *pBrd);
int SGDMA_Init(pcie_board_t *pBrd);
void SGDMA_Disable(pcie_board_t *pBrd);
void SGDMA_EnableChan(pcie_board_t *pBrd, ULONG chan, ULONG startBD);
void SGDMA_DisableChan(pcie_board_t *pBrd, ULONG chan);
void SGDMA_ConfigWrite(pcie_board_t *pBrd,
			ULONG chan,
			ULONG nBDs,
			struct scatterlist *pSGlist,
			ULONG burstSize);
void SGDMA_ConfigRead(pcie_board_t *pBrd,
			ULONG chan,
			ULONG nBDs,
			struct scatterlist *pSGlist,
			ULONG burstSize);
bool SGDMA_StartWriteChan(pcie_board_t *pBrd);
bool SGDMA_StartReadChan(pcie_board_t *pBrd);


#endif
