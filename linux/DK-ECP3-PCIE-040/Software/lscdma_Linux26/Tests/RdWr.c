
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
	ssize_t ret;

	uint32_t *pUsrBuf;

	
	printf("\n\n=====================================================\n");
	printf("Lattice PCIe DMA device driver test\n");
	printf("Testing opening the driver and doing a write() system call.\n");
	printf("This generates an SGDMA read transfer to EBR memory.\n\n");
	printf("=====================================================\n\n");

	if (argc < 2)
	{
		printf("usage: open <brd>_<demo>_<num> <BAR_size>\n");
		brd = "ECP2M_DMA_1_IM";
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


	if (posix_memalign(&pUsrBuf, 4096, EBR_SIZE) != 0)
	{
		perror("posix_memalign");
		goto END;
	}

#if 1
	printf("Trying to do a write() of %d bytes...\n", EBR_SIZE);
	memset(pUsrBuf, 0xa5, EBR_SIZE);
	ret = write(Board_fd, pUsrBuf, EBR_SIZE); 

	printf("write returned: %d\n", ret);
#endif



#if 1
	printf("Trying to do a read() of %d bytes...\n", EBR_SIZE);
	memset(pUsrBuf, 0x00, EBR_SIZE);
	ret = read(Board_fd, pUsrBuf, EBR_SIZE); 

	printf("read returned: %d\n", ret);

	// Display data read from board
	printf("Data read from EBR64:\n");
	for (i = 0; i < 64; i++)
	{
		if ((i % 4) == 0)
			printf("\n%04x: ", i * 4);
		printf("%08x ", pUsrBuf[i]);
	}
#endif

	printf("Press Enter to close...\n");
	fgets(s,2,stdin);

END:

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





