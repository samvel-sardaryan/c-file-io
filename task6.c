#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("numbers.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    char line[16];
    for (int i = 1; i <= 10; i++) {
        int len = snprintf(line, sizeof(line), "%d\n", i);
        if (write(fd, line, len) == -1) {
            perror("write");
            close(fd);
            return 1;
        }
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    fd = open("numbers.txt", O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    char buf[64];
    off_t pos = 0;
    off_t start = -1;
    off_t end = -1;
    int newlines = 0;
    ssize_t n;

    while (end == -1 && (n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                newlines++;
                if (newlines == 3) {
                    start = pos + i + 1;
                }
                if (newlines == 4) {
                    end = pos + i + 1;
                    break;
                }
            }
        }
        pos += n;
    }
    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }
    if (end == -1) {
        fprintf(stderr, "Error: the file has fewer than 4 lines\n");
        close(fd);
        return 1;
    }

    /* read everything after line 4 into memory */
    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    off_t rest_len = size - end;
    char *rest = malloc(rest_len + 1);
    if (rest == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }

    if (lseek(fd, end, SEEK_SET) == -1) {
        perror("lseek");
        free(rest);
        close(fd);
        return 1;
    }

    off_t got = 0;
    while (got < rest_len) {
        n = read(fd, rest + got, rest_len - got);
        if (n == -1) {
            perror("read");
            free(rest);
            close(fd);
            return 1;
        }
        if (n == 0) {
            break;
        }
        got += n;
    }

    const char *new_line = "100\n";
    off_t new_len = strlen(new_line);

    if (lseek(fd, start, SEEK_SET) == -1) {
        perror("lseek");
        free(rest);
        close(fd);
        return 1;
    }
    if (write(fd, new_line, new_len) == -1 || write(fd, rest, got) == -1) {
        perror("write");
        free(rest);
        close(fd);
        return 1;
    }
    free(rest);

    if (ftruncate(fd, start + new_len + got) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("Line 4 starts at byte %ld. Final content:\n", (long)start);
    fflush(stdout);
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

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}