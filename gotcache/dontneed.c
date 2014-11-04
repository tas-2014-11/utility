/*
inspired by
http://net.doit.wisc.edu/~plonka/fincore/
http://net.doit.wisc.edu/~plonka/fadvise/
*/

#define	_XOPEN_SOURCE	600
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void gotcache(char *filename) {
	int fd;
	int advice = POSIX_FADV_DONTNEED;

	fd = open(filename,0);
	if(fd < 0) {
		perror("open");
		exit(1);
	}

	if(fsync(fd)) {
		perror("fsync");
		exit(1);
	}

	if(posix_fadvise(fd,0,0,advice)) {
		perror("posix_fadvise");
	}
}

int main(int argc,char *argv[]) {
	int i;

	if(argc < 2) {
		fprintf(stderr,"usage: %s file ...\n",argv[0]);
		exit(1);
	}

	for(i=1;i<argc;i++) {
		gotcache(argv[i]);
	}
}
