#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("/dev/qread", O_RDWR);
    char write_buf[] = "To je dolg testni niz za preverjanje delovanja kvantiziranega branja.";
    char read_buf[64];
    
    // Pisanje
    write(fd, write_buf, strlen(write_buf));
    lseek(fd, 0, SEEK_SET);

    // Branje (naj bi prebralo samo BLK_SIZE, npr. 32 bajtov)
    int n = read(fd, read_buf, sizeof(read_buf));
    read_buf[n] = '\0';
    
    printf("Prebrano (%d bajtov): %s\n", n, read_buf);
    
    close(fd);
    return 0;
}