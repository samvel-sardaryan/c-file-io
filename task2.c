#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    const char *text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int fd = open("data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    ssize_t w = write(fd, text, 26);
    if (w == -1) {
        perror("write");
        close(fd);
        return 1;
    }
    if (w != 26) {
        fprintf(stderr, "Error: only %zd of 26 bytes were written\n", w);
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    fd = open("data.txt", O_RDWR);
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
    printf("Size before truncation: %ld bytes\n", (long)size);

    if (ftruncate(fd, 10) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }
    printf("Size after truncation: %ld bytes\n", (long)size);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("Content: ");
    fflush(stdout);

    char buf[64];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, n) == -1) {
            perror("write");
            close(fd);
            return 1;
        }
    }
    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }
    printf("\n");

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}