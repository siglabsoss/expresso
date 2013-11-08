
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
void memAccess(uint32_t *pMem);

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


	printf("\nRegister Read/Write Access\n");
	printf("r - read a 32 bit register\n");
	printf("w - write a 32 bit value into a register\n");
	printf("addresses are on 4 byte boundaries\n");
	printf("q,x - exit\n");


	memAccess(p0);


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




void memAccess(uint32_t *pMem)
{
	char cmd[256];
	int done;
	int addr, data;
	int args;
	char action;

	done = 0;


	while (!done)
	{
		printf("r|w <addr> [data]: ");
		fgets(cmd, 250, stdin);

		args = sscanf(cmd, "%c %x %x", &action, &addr, &data);
		if (args > 0)
		{
			tolower(action);
			switch (action)
			{
				case 'r':
					if (args == 1)
						addr = addr + 4;
					data = *(pMem + addr/4);
					printf("%08x: 0x%x\n", addr, data);
				
					break;

				case 'w':
					if (args == 1)
						addr = addr + 4;
					*(pMem + addr/4) = data;
					break;

			        case 'q':
				case 'x':
					done = 1;
					break;
			}
		}
	}
}
			





