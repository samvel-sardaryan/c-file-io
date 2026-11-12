#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define CHUNK_SIZE 4096
#define PATH_SIZE 256

int read_path(const char *prompt, char *path) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(path, PATH_SIZE, stdin) == NULL) {
        return -1;
    }
    path[strcspn(path, "\n")] = '\0';
    return 0;
}

ssize_t read_chunk(int fd, char *buf) {
    ssize_t total = 0;
    while (total < CHUNK_SIZE) {
        ssize_t n = read(fd, buf + total, CHUNK_SIZE - total);
        if (n == -1) {
            return -1;
        }
        if (n == 0) {
            break;
        }
        total += n;
    }
    return total;
}

int main(void) {
    char path1[PATH_SIZE];
    char path2[PATH_SIZE];

    if (read_path("Enter the first file path: ", path1) == -1 ||
        read_path("Enter the second file path: ", path2) == -1) {
        fprintf(stderr, "Error: could not read the path\n");
        return 2;
    }

    int fd1 = open(path1, O_RDONLY);
    if (fd1 == -1) {
        perror("open first file");
        return 2;
    }

    int fd2 = open(path2, O_RDONLY);
    if (fd2 == -1) {
        perror("open second file");
        close(fd1);
        return 2;
    }

    char buf1[CHUNK_SIZE];
    char buf2[CHUNK_SIZE];
    long offset = 0;

    while (1) {
        ssize_t n1 = read_chunk(fd1, buf1);
        ssize_t n2 = read_chunk(fd2, buf2);
        if (n1 == -1 || n2 == -1) {
            perror("read");
            close(fd1);
            close(fd2);
            return 2;
        }

        ssize_t common = (n1 < n2) ? n1 : n2;
        for (ssize_t i = 0; i < common; i++) {
            if (buf1[i] != buf2[i]) {
                printf("Files differ at byte %ld\n", offset + i);
                close(fd1);
                close(fd2);
                return 1;
            }
        }

        if (n1 != n2) {
            printf("Files differ at byte %ld\n", offset + common);
            close(fd1);
            close(fd2);
            return 1;
        }

        if (n1 == 0) {
            break;
        }
        offset += n1;
    }

    printf("Files are identical\n");

    if (close(fd1) == -1 || close(fd2) == -1) {
        perror("close");
        return 2;
    }
    return 0;
}