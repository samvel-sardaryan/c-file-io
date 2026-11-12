#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("sparse.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (write(fd, "START", 5) == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    if (lseek(fd, 1024 * 1024, SEEK_CUR) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    if (write(fd, "END", 3) == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    fd = open("sparse.bin", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }
    printf("Apparent file size: %ld bytes\n", (long)size);
    /*
     * du is much smaller than the apparent size because the 1 MiB between
     * "START" and "END" is a hole. lseek moved past the end of the file and
     * nothing was written there, so the file system gives that part no disk
     * blocks, and reading it just returns zeros. The apparent size (ls -l,
     * lseek) counts every byte up to the end: 5 + 1,048,576 + 3 = 1,048,584.
     * du counts only the blocks really used: one for "START" and one for
     * "END". With 4 KB blocks that's 8 KB.
     */

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}