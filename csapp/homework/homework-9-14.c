#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(void) {
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	char temp;
	int fd;
	char *buff;
	char filename[20] = "hello.txt";
	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, mode);
	if (fd == -1) {
		fprintf(stderr, "Cannot open file '%s'.\n", filename);
	}
	write(fd, "Hello, world!\n", 14);
	printf("Write hello world to '%s' <Press Enter to continue>\n", filename);
	scanf("%c", &temp);
	buff = (char *) mmap(NULL, 1, PROT_WRITE, MAP_SHARED, fd, 0);
	buff[0] = 'J';
	munmap(buff, 14);
	close(fd);
	return 0;
}
