
#define POSIX_SOURCE 199309

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include <errno.h>
#include <sys/mman.h>

#include <linux/version.h>

#include "Ioctl.h"


	
#define PCI_BAR_SIZE 0x20000  // 128KB defined for DMA demo
#define EBR_BASE_ADDR 0x10000
#define GPIO_BASE_ADDR 0x00000
#define INTR_BASE_ADDR 0x00100
#define SGDMA_BASE_ADDR 0x2000

#define SGDMA(a) p0[(SGDMA_BASE_ADDR + a)/sizeof(uint32_t)]


uint32_t *lscdma_open(char *region, size_t l);
int lscdma_close(void *pmem);
void memTest(uint32_t *pMem);

size_t BarSize;
int Board_fd = -1;



int main(int argc, char *argv[])
{
	int i;
	char s[1024];
	uint32_t *p0;
	uint16_t *ps;
	char *brd;
	char filename[256];
	int fdBoard;
	PCIResourceInfo_t info;
	DMAResourceInfo_t extra;
	char verStr[MAX_DRIVER_VERSION_LEN];

	
	printf("\n\n=====================================================\n");
	printf("Lattice PCIe DMA device driver test\n");
	printf("Dumping SGDMA IP Registers for debug.\n");
	printf("=====================================================\n\n");

	if (argc < 2)
	{
		printf("usage: open <brd>_<demo>_<num> <BAR_size>\n");
		brd = "ECP2M_DMA_1";
		printf("Using Default: Board %s\n", brd);
	}

	if (argc >= 2)
	{
		brd = argv[1];
		printf("Board: %s\n", brd);
	}

	if (argc == 3)
	{
		BarSize = atoi(argv[2]);
		printf("BAR size %d\n", BarSize);
	}
	else
	{
		BarSize = PCI_BAR_SIZE;   // default for our demo BAR0
	}

	printf("Opening access to lscdma/%s\n", brd);
	sprintf(filename, "/dev/lscdma/%s", brd);
	p0 = lscdma_open(filename, BarSize);

	if (Board_fd == -1)
	{
		printf("ERROR opening driver!\n");
		exit(-1);
	}

	printf("*p0 = %x\n", (long)p0);

	if (p0 == NULL)
	{
		perror("mmap error");
		exit(-1);
	}

	printf("\nioctl() get Driver Version...\n");
		
	if (ioctl(Board_fd, IOCTL_LSCPCIE_GET_VERSION_INFO , verStr) != 0)
	{
		perror("ioctl");
		exit(-1);
	}
	else
	{
		printf("Driver Version: %s\n", verStr);
	}



	printf("\nSGDMA IP Registers\n");
	printf("======================\n");
	printf("Global Registers:\n");
	printf("IPID: %08x\n", SGDMA(0));
	printf("IPVER: %08x\n", SGDMA(4));
	printf("GCONTROL: %08x\n", SGDMA(8));
	printf("GSTATUS: %08x\n", SGDMA(0x0c));
	printf("GEVENT: %08x\n", SGDMA(0x10));
	printf("GERROR: %08x\n", SGDMA(0x14));
	printf("GARB: %08x\n", SGDMA(0x18));
	printf("GAUX: %08x\n", SGDMA(0x1c));
		
	printf("\nChannel[0] (Wr) Registers:\n");
	printf("CONTROL: %08x\n", SGDMA(0x200));
	printf("STATUS: %08x\n", SGDMA(0x204));
	printf("CUR_SRC: %08x\n", SGDMA(0x208));
	printf("CUR_DST: %08x\n", SGDMA(0x20c));
	printf("XFERCNT: %08x\n", SGDMA(0x210));
	printf("PBOFF: %08x\n", SGDMA(0x214));

	printf("\nChannel[1] (Rd) Registers:\n");
	printf("CONTROL: %08x\n", SGDMA(0x220));
	printf("STATUS: %08x\n", SGDMA(0x224));
	printf("CUR_SRC: %08x\n", SGDMA(0x228));
	printf("CUR_DST: %08x\n", SGDMA(0x22c));
	printf("XFERCNT: %08x\n", SGDMA(0x230));
	printf("PBOFF: %08x\n", SGDMA(0x234));

	printf("\n\nWrite Channel BD's\n");
	for (i = 0; i < 16; i++)
	{
		printf("\nBufferDescriptor[%d] Registers:\n", i);
		printf("CONFIG0: %08x\n", SGDMA(0x400 + (i * 16) + 0));	
		printf("CONFIG1: %08x\n", SGDMA(0x400 + (i * 16) + 4));	
		printf("SRC_ADDR: %08x\n", SGDMA(0x400 + (i * 16) + 8));	
		printf("DST_ADDR: %08x\n", SGDMA(0x400 + (i * 16) + 12));	
	}

	printf("\n\nRead Channel BD's\n");
	for (i = 16; i < 16 + 16; i++)
	{
		printf("\nBufferDescriptor[%d] Registers:\n", i);
		printf("CONFIG0: %08x\n", SGDMA(0x400 + (i * 16) + 0));	
		printf("CONFIG1: %08x\n", SGDMA(0x400 + (i * 16) + 4));	
		printf("SRC_ADDR: %08x\n", SGDMA(0x400 + (i * 16) + 8));	
		printf("DST_ADDR: %08x\n", SGDMA(0x400 + (i * 16) + 12));	
	}

	printf("Press Enter to close...\n");
	fgets(s,2,stdin);


	lscdma_close(p0);

	return(0);
}


uint32_t	*lscdma_open(char *region, size_t len)
{
	int fd;
	int sysErr;
	uint32_t *pmem;

	/* Open the kernel mem object to gain access
	 */
	fd = open(region, O_RDWR, 0666); 
	if (fd == -1)
	{
		perror("ERROR open(): ");
		return(NULL);
	}
	printf("fd = %d\n");


	pmem = mmap(0,             /* choose any user address */
		    len,           /* This big */
		    PROT_READ | PROT_WRITE, /* access control */
		    MAP_SHARED,             /* access control */
		    fd,                  /* the object */
		    0);                  /* the offset from beginning */

// Need for ioctl calls so why close????
	// close(fd);  /* not needed any more */
	if (pmem == MAP_FAILED)
	{
		perror("mmap: ");
		return(NULL);
	}
	printf("pmem=0x%x\n", (long)pmem);

	Board_fd = fd;


	return(pmem);
}


int lscdma_close(void *pmem)
{
	int sysErr;


	/* Release the shared memory.  It won't go away but we're done with it */
	sysErr = munmap(pmem, BarSize);
	if (sysErr == -1)
	{
		perror("munmap: ");
		return(-1);
	}

	close(Board_fd);

	return(0);
}


