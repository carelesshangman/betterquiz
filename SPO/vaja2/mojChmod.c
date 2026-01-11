#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

void print_mode(mode_t mode) {
	char perms[11];
		//type	
	if (S_ISREG(mode)) perms[0] = '-';
	else if(S_ISDIR(mode)) perms[0] = 'd';
	else if(S_ISDIR(mode)) perms[0] = 'l';
	else if(S_ISDIR(mode)) perms[0] = 'c';
	else if(S_ISDIR(mode)) perms[0] = 'b';
	else if(S_ISDIR(mode)) perms[0] = 'p';
	else if(S_ISDIR(mode)) perms[0] = 's';
	else perms[0] = '?';
	
	
	//User perm
	perms[1] = (mode & S_IRUSR) ? 'r' : '-';
	perms[2] = (mode & S_IWUSR) ? 'w' : '-';
	perms[3] = (mode & S_IXUSR) ? 'x' : '-';

	//Group perm
	perms[4] = (mode & S_IRGRP) ? 'r' : '-';
	perms[5] = (mode & S_IWGRP) ? 'w' : '-';
	perms[6] = (mode & S_IXGRP) ? 'x' : '-';
	
	//Other perm
	perms[7] = (mode & S_IROTH) ? 'r' : '-';
	perms[8] = (mode & S_IWOTH) ? 'w' : '-';
	perms[9] = (mode & S_IXOTH) ? 'x' : '-';
	perms[10] = '\0';
	
	//Special perm
	if(mode & S_ISUID) perms[3] = (perms[3] == 'x') ? 's' : 'S';
	if(mode & S_ISGID) perms[6] = (perms[6] == 'x') ? 's' : 'S';
	if(mode & S_ISVTX) perms[9] = (perms[9] == 'x') ? 't' : 'T';
	
	printf("Nova dovoljenja: %s\n", perms);
	
}

int main(int argc, char *argv[]){
	char *mode_str = argv[1];
	char *filename = argv[2];
	struct stat info;
	stat(filename, &info);
	mode_t new_mode = info.st_mode;
	bool check = true;
	
	if (argc != 3) {
		printf("Napačna uporaba programa:");
		return -1;
	}
	
	if (stat(filename, &info) == -1) {
		perror("Napaka pri branju dat.");
		return -1;
	}
	
	if (strcmp(mode_str, "u+s") == 0) {
		new_mode |= S_ISUID;
	}
	else if (strcmp(mode_str, "u-s") == 0) {
		new_mode &= ~S_ISUID;
	}
	else if (strcmp(mode_str, "g+s") == 0) {
		new_mode |= S_ISGID;
	}
	else if (strcmp(mode_str, "a+rX") == 0) {
		if (info.st_mode & S_IRUSR) {
			new_mode |= S_IRGRP;
			new_mode |= S_IROTH;
		}
	}
	else {
		long int mode_oct;
		char *endptr;
		mode_oct = strtol(mode_str, &endptr, 8);
		if(endptr != mode_str) {
			check = false;
			if (chmod (filename, mode_oct) < 0) {
				perror(argv[0]);
				exit(1);
			}
		}
		else { printf("napaka");}
	}
	
	if(check) chmod(filename, new_mode);
	
	stat(filename, &info);
	print_mode(info.st_mode);	
	return 0;
}
