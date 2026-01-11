#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	char buf[BUFSIZ];
	int n = 5;
	int fd;

	if (argc == 4 && strcmp(argv[1], "-n") == 0){
		n = atoi(argv[2]);
		fd = open(argv[3], O_RDONLY);
	}
	else if (argc == 2){
		fd = open(argv[1], O_RDONLY);
	}
	else {
		perror("Napačna uporaba programa");
		return -1;
	}

	if(fd<0){
		perror("fd open error");
		return -1;
	}

	lseek(fd, 0, SEEK_END);
	lseek(fd, -1, SEEK_CUR);
	int count = 0, temp_place = 300;

	while(count <= n && temp_place>0) {
		read(fd, buf, 1);
		if(buf[0] == '\n'){
			count ++;
		}
		temp_place = lseek(fd, -2, SEEK_CUR);
	}
	if(count >= n) lseek(fd, 2, SEEK_CUR);	
	int t = read(fd, buf, BUFSIZ);
	write(STDOUT_FILENO, buf, t);

	return 0;
}
