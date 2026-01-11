#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
	char buf[BUFSIZ];
	int n, fd;

	if (argc == 3 && strcmp(argv[1], "-a") == 0){
		fd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0666);
	}
	else if (argc == 2){
		fd = open(argv[1], O_WRONLY | O_CREAT, 0666);
	}
	else {
		perror("Napačna uporaba programa");
		return -1;
	}

	if(fd<0){
		perror("fd open error");
		return -1;
	}

	while((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0) {
		if(write(STDOUT_FILENO, buf, n) != n) {
			err("write");
		}
		if(write(fd, buf, n) != n) {
			err("write2");
		}
	}

	return 0;
}
