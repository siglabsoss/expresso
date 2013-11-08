
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


#define EBR_SIZE    (16384)
#define EBR_DW_SIZE (16384 / 4)
	
#define PCI_DEV_MEM_SIZE 0x5000  // Basic: GPIO + 16KB EBR

uint32_t *lscpcie_open(char *region, size_t l);
int lscpcie_close(void *pmem);
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
	ExtraResourceInfo_t extra;

	printf("User Space Lattice PCIe device driver using mmap\n");

	if (argc < 2)
	{
		printf("usage: test <brd>_<demo>_<num> <size>\n");
		exit(-1);
	}

	if (argc >= 2)
	{
		brd = argv[1];
		printf("Board %s\n", brd);
	}
	else
	{
		brd = "SC_Basic_1";
		printf("Using Default: Board %s\n", brd);
	}

	if (argc == 3)
	{
		BarSize = atoi(argv[2]);
		printf("BAR size %d\n", BarSize);
	}
	else
	{
		BarSize = PCI_DEV_MEM_SIZE;   // default for our demo BAR1
	}

	printf("Opening access to lscpcie/%s\n", brd);
	sprintf(filename, "/dev/LSC_PCIe/%s", brd);
	p0 = lscpcie_open(filename, BarSize);
	printf("*p0 = %x\n", (long)p0);

	if (p0 == NULL)
		exit(-1);

	printf("Displaying first 8 GPIO registers\n");
	for (i = 0; i < 8; i++)
	{
		if ((i % 4) == 0)
			printf("\n%04x: ", i*4);
		printf("%08x  ", p0[i]);
	}
	printf("\n");

	printf("Blink LEDs\n");
	for (i = 0; i < 16; i++)
	{
		p0[2] = 1<<i;
		sleep(1);
	}
	


	printf("\n\nEBR Memory access tests\n");
	memTest(p0 + (0x1000/4));


	printf("ioctl() get Resources...\n");
	if (Board_fd != -1)
	{
		
		if (ioctl(Board_fd, IOCTL_LSCPCIE_GET_RESOURCES, &info) != 0)
			perror("ioctl");
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


	}


	printf("\n\nioctl() get ExtraInfo...\n");
	if (Board_fd != -1)
	{
		
		if (ioctl(Board_fd, IOCTL_LSCPCIE2_GET_EXTRA_INFO, &extra) != 0)
			perror("ioctl: extra info");
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
			printf("DriverName = %s\n", extra.DriverName);
		}

	}



	printf("Press Enter to close...\n");
	fgets(s,2,stdin);


	lscpcie_close(p0);

	return(0);
}


uint32_t	*lscpcie_open(char *region, size_t len)
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


int lscpcie_close(void *pmem)
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

	for (i = 0; i < EBR_DW_SIZE; i++)
	{
		if (pMem[i] != 0)
		{
			printf("ERROR! EBR not cleared!\n");
			return;
		}
	}

	printf("Setting to 0xa5...\n");
	memset(pMem, 0xa5, EBR_SIZE);

	for (i = 0; i < EBR_DW_SIZE; i++)
	{
		if (pMem[i] != 0xa5a5a5a5)
		{
			printf("ERROR! EBR not set to 0xa5!\n");
			return;
		}
	}


	printf("Setting to pattern...\n");
	for (i = 0; i < EBR_DW_SIZE; i++)
	{
		pMem[i] = 0xcafe0000 | i;
	}

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
			





