/*
inspired by
http://net.doit.wisc.edu/~plonka/fincore/
http://net.doit.wisc.edu/~plonka/fadvise/
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

long pagesize = 0;

void *do_malloc(size_t size) {
	void *p;
	/* printf("malloc(%d)\n",size); */
	if(p = malloc(size)) { return(p); }
	perror("malloc");
	exit(1);
}

size_t get_file_size(char *filename) {
	struct stat buf;
	if(!stat(filename,&buf)) { return(buf.st_size); }
	perror("stat");
	exit(1);
}

unsigned count_pages(char *vec,unsigned maxpages) {
	int i;
	unsigned npages = 0;
	for(i=0;i<maxpages;i++) {
		if(*(vec+i) & 0x01) {
			++npages;
		}
	}
	return(npages);
}

void fill_pagevec(unsigned char *vec,unsigned maxpages,unsigned *pagevec) {
	int i;
	int j = 0;

	for(i=0;i<maxpages;i++) {
		if(*(vec+i) & 0x01) {
			*(pagevec + j++) = i;
		}
	}
}

void *do_mmap(char *filename,size_t len) {
	void *start;
	int fd;
	off_t offset = 0;

	if(-1 == (fd = open(filename,O_RDONLY|O_EXCL))) {
		perror("open");
		exit(1);
	}

	if(MAP_FAILED == (start = mmap(NULL,len,PROT_NONE,MAP_SHARED,fd,offset))) {
		perror("mmap");
		exit(1);
	}

	if(-1 == close(fd)) {
		perror("close");
		exit(1);
	}

	return(start);
}

void gotcache(char *filename,unsigned *maxpages,unsigned *npages,unsigned **pagevec) {
	size_t len;
	unsigned char *vec;
	void *start;

	len = get_file_size(filename);

	start = do_mmap(filename,len);

	*maxpages = (len + pagesize - 1) / pagesize;
	vec = do_malloc(*maxpages);
	if(mincore(start,len,vec)) {
		fprintf(stderr,"mincore(0,%u,%#p)\n",len,vec);
		perror("mincore");
		free(vec);
		exit(1);
	}

	if(munmap(start,len)) {
		perror("munmap");
		exit(1);
	}

	*npages = count_pages(vec,*maxpages);
	*pagevec = do_malloc(*npages);

	fill_pagevec(vec,*npages,*pagevec);

	free(vec);
}

void print_pagevec(char *filename,unsigned maxpages,unsigned npages,unsigned *pagevec) {
	int i;

	printf("%s:",filename);
	printf("%u:",maxpages);
	printf("%u:",npages);

	for(i=0;i<npages;i++) {
		if(i) { printf(" "); }
		printf("%u",*(pagevec+i));
	}

	printf("\n");
}

int main(int argc,char *argv[]) {
	int i;

	if(argc < 2) {
		fprintf(stderr,"usage: %s file ...\n",argv[0]);
		exit(1);
	}

	pagesize = sysconf(_SC_PAGE_SIZE);
	if(pagesize == -1) {
		perror("sysconf");
		exit(1);
	}

	for(i=1;i<argc;i++) {
		unsigned maxpages;
		unsigned npages;
		unsigned *pagevec;
		char *filename = argv[i];

		gotcache(filename,&maxpages,&npages,&pagevec);
		print_pagevec(filename,maxpages,npages,pagevec);

/* bug here
		printf("3 pagevec=%x\n",pagevec);
		fflush(stdout);
		free(pagevec);
*/
	}
}
