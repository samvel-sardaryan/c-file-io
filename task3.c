#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define PATH_SIZE 256

int main(void) {
    char path[PATH_SIZE];

    printf("Enter the path to a text file: ");
    fflush(stdout);
    if (fgets(path, PATH_SIZE, stdin) == NULL) {
        fprintf(stderr, "Error: could not read the path\n");
        return 1;
    }
    path[strcspn(path, "\n")] = '\0';

    int fd = open(path, O_RDONLY);
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

    char c;
    for (off_t pos = size - 1; pos >= 0; pos--) {
        if (lseek(fd, pos, SEEK_SET) == -1) {
            perror("lseek");
            close(fd);
            return 1;
        }
        ssize_t n = read(fd, &c, 1);
        if (n == -1) {
            perror("read");
            close(fd);
            return 1;
        }
        if (n == 0) {
            break;
        }
        if (write(STDOUT_FILENO, &c, 1) == -1) {
            perror("write");
            close(fd);
            return 1;
        }
    }

    if (write(STDOUT_FILENO, "\n", 1) == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}