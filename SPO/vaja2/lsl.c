#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
	struct stat info;
	struct passwd *pw;
	struct group *gr;
	
	if (argc != 2) {
		printf("Napačna uporaba programa:");
		return -1;
	}
	
	if (lstat(argv[1], &info) == -1) {
		perror("Napaka pri branju dat.");
		return -1;
	}
	
	pw = getpwuid(info.st_uid);
	gr = getgrgid(info.st_gid);
	mode_t mode = info.st_mode;
	
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

	printf("%s", perms);	
	
	printf(" %lu %s %ld %s\n", info.st_nlink, (pw != NULL ? pw->pw_name : "?"), info.st_size, argv[1]);
	
	return 0;
}
