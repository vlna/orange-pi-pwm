/// /gcc pwm.c -o pwm ///
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>


#define SW_PORTC_IO_BASE  0x01c20800 //dataregister 
#define PWM_CH_CTRL  0x01c21400 //pwm control register

int main() {
	unsigned int * pc;
	int fd, i;
	char * ptr;
	unsigned int addr_start, addr_offset, PageSize, PageMask, data;

	PageSize = sysconf(_SC_PAGESIZE);
	PageMask = (~(PageSize-1));
	addr_start = SW_PORTC_IO_BASE & PageMask;
	addr_offset = SW_PORTC_IO_BASE & ~PageMask;

	fd = open("/dev/mem", O_RDWR);
	if(fd < 0) {
		perror("Unable to open /dev/mem");
		return(-1);
	}

	pc = mmap(0, PageSize*2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr_start);

	if(pc == MAP_FAILED) {
		perror("Unable to mmap file");
		printf("pc:%lx\n", (unsigned long)pc);
		return(-1);
	}
	ptr = (char *)pc + addr_offset;
	data = *(unsigned int *)(ptr+0x00); //offset for controller register
	// PA_CFGO_REG  0x01c20800 page 317 H3 datasheet
	data |= (1<<20);                             //set port PA5 to pwm (011) 
	data |= (1<<21); 
	data &= ~(1<<22);                             
	*(unsigned int *)(ptr+0x00) = data;
	//PWM control register
	PageSize = sysconf(_SC_PAGESIZE);
	PageMask = (~(PageSize-1));
	addr_start = PWM_CH_CTRL & PageMask;
	addr_offset = PWM_CH_CTRL & ~PageMask;


	pc = mmap(0, PageSize*2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr_start);

	if(pc == MAP_FAILED) {
		perror("Unable to mmap file");
		printf("pc:%lx\n", (unsigned long)pc);
		return(-1);
	}
	ptr = (char *)pc + addr_offset;
	data = *(unsigned int *)(ptr+0x00); //offset for controller register

	//       data |= (1<<0);                              //prescale = 360
	//      data |= (1<<1);                              //

	//       data |= (1<<2);                              //prescale = 72K
	//      data |= (1<<3);                              //prescal

	data |= (1<<4);                              //pwm channel 0 enable
	data |= (1<<5);                              //low - high level
	data |= (1<<6);                              //clock gating
	//       data |= (1<<7);                              //cycle mode / pulse mode
	data |= (1<<8);                              //control register
	//       data |= (1<<9);                              //pwm_bypass 24Mhz 1 is enable
	*(unsigned int *)(ptr+0x00) = data;

	//PWM_CH0_PERIOD PWM CHANNEL 0 Period Register offset 0x04
	ptr = (char *)pc + addr_offset + 0x04;

	data = (0x6EEE00EE);                              //control register 987 hz (no prescaler)
	//  data = (0x000E0006);                              //control register 1.6 Mhz max
	//   data = (0x00020001);                              //control register 8 Mhz max
	*(unsigned int *)(ptr+0x00) = data;

	return 0;
}
