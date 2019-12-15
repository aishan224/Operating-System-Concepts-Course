#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#define _NR_mycall 335 // Set mycall(335) to _NR_mycall

int main(){

	syscall(_NR_mycall);

	return 0;
}
