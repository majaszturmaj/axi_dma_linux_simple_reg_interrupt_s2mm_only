#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <zlib.h>

#define S2MM_CONTROL_REGISTER       0x30
#define S2MM_STATUS_REGISTER        0x34
#define S2MM_DST_ADDRESS_REGISTER   0x48//0x48
#define S2MM_BUFF_LENGTH_REGISTER   0x58

#define IOC_IRQ_FLAG                1<<12
#define IDLE_FLAG                   1<<1

#define STATUS_HALTED               0x00000001
#define STATUS_IDLE                 0x00000002
#define STATUS_SG_INCLDED           0x00000008
#define STATUS_DMA_INTERNAL_ERR     0x00000010
#define STATUS_DMA_SLAVE_ERR        0x00000020
#define STATUS_DMA_DECODE_ERR       0x00000040
#define STATUS_SG_INTERNAL_ERR      0x00000100
#define STATUS_SG_SLAVE_ERR         0x00000200
#define STATUS_SG_DECODE_ERR        0x00000400
#define STATUS_IOC_IRQ              0x00001000
#define STATUS_DELAY_IRQ            0x00002000
#define STATUS_ERR_IRQ              0x00004000

#define HALT_DMA                    0x00000000
#define RUN_DMA                     0x00000001
#define RESET_DMA                   0x00000004
#define ENABLE_IOC_IRQ              0x00001000
#define ENABLE_DELAY_IRQ            0x00002000
#define ENABLE_ERR_IRQ              0x00004000
#define ENABLE_ALL_IRQ              0x00007000

#define RESET_ALL_IRQ			   	0x0000F000
#define NUMBER_OF_TRANSFERS         10



unsigned int write_dma(unsigned int *virtual_addr, int offset, unsigned int value)
{
    virtual_addr[offset>>2] = value;

    return 0;
}

unsigned int read_dma(unsigned int *virtual_addr, int offset)
{
    return virtual_addr[offset>>2];
}


int dma_s2mm_sync(unsigned int *virtual_addr)
{
    unsigned int s2mm_status = read_dma(virtual_addr, S2MM_STATUS_REGISTER);

	while(!(s2mm_status & IOC_IRQ_FLAG) || !(s2mm_status & IDLE_FLAG))
	{
        s2mm_status = read_dma(virtual_addr, S2MM_STATUS_REGISTER);
    }

	return 0;
}

void print_mem(void *virtual_address, int byte_count)
{
	char *data_ptr = virtual_address;

	for(int i=0;i<byte_count;i++){
		printf("%02X", data_ptr[i]);

		if(i%4==3){
			printf(" ");
		}
	}

	printf("\n");
}

int main()
{
    int Index;
    int Tries = NUMBER_OF_TRANSFERS; // when you want only specific number of transfers
	int ddr_memory = open("/dev/mem", O_RDWR | O_SYNC);

    unsigned int *dma_virtual_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x40400000);

    unsigned int *virtual_dst_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x0f000000);

    // for(Index = 0; Index < Tries; Index ++) {
    while (1) {

		write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, RESET_DMA);
	    	write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, HALT_DMA);

	    	memset(virtual_dst_addr, 0, 64);

	    	write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, RESET_DMA);
		write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, HALT_DMA);

	    	write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, RUN_DMA);
	    	write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, ENABLE_ALL_IRQ|RUN_DMA);

	    	write_dma(dma_virtual_addr, S2MM_DST_ADDRESS_REGISTER, 0x0f000000);
	    	write_dma(dma_virtual_addr, S2MM_BUFF_LENGTH_REGISTER, 64);

		dma_s2mm_sync(dma_virtual_addr);

		printf("Received data from the DMA: \n");
		print_mem(virtual_dst_addr, 32);

		write_dma(dma_virtual_addr, S2MM_STATUS_REGISTER, RESET_ALL_IRQ);
    }

    return 0;
}
