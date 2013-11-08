
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


#define EBR_SIZE    (64 * 1024)
#define EBR_DW_SIZE (EBR_SIZE / 4)
	
#define PCI_BAR_SIZE 0x20000  // 128KB defined for DMA demo
#define EBR_BASE_ADDR 0x10000
#define GPIO_BASE_ADDR 0x00000
#define INTR_BASE_ADDR 0x00100


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
	printf("Testing opening the driver and accessing IP modules\n");
	printf("in the FPGA design.\n\n");
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



	printf("\nioctl() get Resources...\n");
		
	if (ioctl(Board_fd, IOCTL_LSCPCIE_GET_RESOURCES, &info) != 0)
	{
		perror("ioctl");
		exit(-1);
	}
	else
	{
		printf("PCIinfo.hasInterrupt = %d\n", info.hasInterrupt);
		printf("PCIinfo.intrVector = %d\n", info.intrVector);
		printf("PCIinfo.numBARs = %d\n", info.numBARs);
		printf("CFG0:");
		for (i = 0; i < 0x40; i++)
		{
			if ((i % 16) == 0)
				printf("\n%02x: ", i);
			printf("%02x ", info.PCICfgReg[i]);
		}
		printf("\n");

		for (i = 0; i < MAX_PCI_BARS; i++)
		{
			printf("BAR[%d]:\n", info.BAR[i].nBAR);
			printf("\tphyStartAddr = %x\n", info.BAR[i].physStartAddr);
			printf("\tsize = %d\n", info.BAR[i].size);
			printf("\tmemMapped = %d\n", info.BAR[i].memMapped);
			printf("\tflags = %d\n", info.BAR[i].flags);
			printf("\ttype = %d\n", info.BAR[i].type);
		}

	}




	printf("\nioctl() get DMA Info...\n");
		
	if (ioctl(Board_fd, IOCTL_LSCDMA_GET_DMA_INFO, &extra) != 0)
	{
		perror("ioctl: extra info");
		exit(-1);
	}
	else
	{
		printf("devID = %d\n", extra.devID);
		printf("busNum = %d\n", extra.busNum);
		printf("deviceNum = %d\n", extra.deviceNum);
		printf("functionNum = %d\n", extra.functionNum);
		printf("UINumber = %d\n", extra.UINumber);
		printf("hasDmaBuf = %d\n", extra.hasDmaBuf);
		printf("DmaBufSize = %d (0x%x)\n", extra.DmaBufSize, extra.DmaBufSize);
		printf("DmaAddr64 = %d\n", extra.DmaAddr64);
		printf("DmaPhyAddrHi = 0x%08x\n", extra.DmaPhyAddrHi);
		printf("DmaPhyAddrLo = 0x%08x\n", extra.DmaPhyAddrLo);
	}


	printf("\nDisplaying first 8 GPIO registers\n");
	for (i = 0; i < 8; i++)
	{
		if ((i % 4) == 0)
			printf("\n%04x: ", i*4);
		printf("%08x  ", p0[i]);
	}
	printf("\n");

	printf("\nBlinking LEDs...");
	fflush(stdout);
	for (i = 0; i < 16; i++)
	{
		p0[2] = 1<<i;
		Sleep(500);
	}
	printf("\n\n");
	


	// Memory Test
	memTest(p0 + (EBR_BASE_ADDR/4));




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


void memTest(uint32_t *pMem)
{
	int i;

	printf("EBR Memory Access Tests\n");

	printf("Clearing to 0's...\n");
	memset(pMem, 0, EBR_SIZE);

	printf("Verifying 0x00...\n");
	for (i = 0; i < EBR_DW_SIZE; i++)
	{
		if (pMem[i] != 0)
		{
			printf("ERROR! EBR not cleared!\n");
			return;
		}
	}
	printf("PASS\n\n");

	printf("Setting to 0xa5...\n");
	memset(pMem, 0xa5, EBR_SIZE);

	printf("Verifying 0xa5...\n");
	for (i = 0; i < EBR_DW_SIZE; i++)
	{
		if (pMem[i] != 0xa5a5a5a5)
		{
			printf("ERROR! EBR not set to 0xa5!\n");
			return;
		}
	}
	printf("PASS\n\n");


	printf("Setting to pattern...\n");
	for (i = 0; i < EBR_DW_SIZE; i++)
	{
		pMem[i] = 0xcafe0000 | i;
	}

	printf("Verifying pattern...\n");
	for (i = 0; i < EBR_DW_SIZE; i++)
	{
		if (pMem[i] != (0xcafe0000 | i))
		{
			printf("ERROR! EBR not set to pattern!\n");
			return;
		}
	}

	printf("PASS\n\n");
}
			





