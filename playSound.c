#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/kd.h>
int play(int freq, int length, int reps) {
	int i; /* loop counter */
	char buf[25];
	sprintf(buf, "play -n synth %d sin %d", length, freq);

	for (i = 0; i < reps; i++) {
		system(buf);
		usleep(200);                                    
	}                                                     
	return 0;
}

