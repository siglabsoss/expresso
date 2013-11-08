
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
#include "GPIO.h"


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
	printf("Testing Interrupt handling in the driver.\n");
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
	printf("*p0 = %x\n", (long)p0);


	if (Board_fd == -1)
	{
		perror("open driver error");
		exit(-1);
	}

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
	}


	printf("\nDisplaying Interrupt Controller registers\n");
	printf("INTRCTL_ID_REG (0x100): %08x\n", p0[INTRCTL_ID_REG/4]);
	printf("INTRCTL_CTRL   (0x104): %08x\n", p0[INTRCTL_CTRL/4]);
	printf("INTRCTL_STATUS (0x108): %08x\n", p0[INTRCTL_STATUS/4]);
	printf("INTRCTL_ENABLE (0x10C): %08x\n", p0[INTRCTL_ENABLE/4]);


	printf("Press Enter to start interrupt test...");
	fflush(stdout);
	fgets(s,2,stdin);

	p0[INTRCTL_CTRL/4] = 0;
	p0[INTRCTL_ENABLE/4] = 0;
	
	p0[INTRCTL_CTRL/4] = INTRCTL_TEST_MODE | INTRCTL_OUTPUT_EN;
	p0[INTRCTL_ENABLE/4] = INTRCTL_TEST1_EN;
	for (i = 0; i < 8; i++)
	{
		p0[INTRCTL_CTRL/4] = INTRCTL_INTR_TEST1 | INTRCTL_TEST_MODE | INTRCTL_OUTPUT_EN;
		printf("%d...", i+1);
		fflush(stdout);
		sleep(1);
	}

	printf("\nPress Enter to close...");
	fflush(stdout);
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






