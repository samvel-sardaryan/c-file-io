#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 4096
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

int main(void) {
    char src_path[PATH_SIZE];
    char dst_path[PATH_SIZE];

    if (read_path("Enter the source file path: ", src_path) == -1 ||
        read_path("Enter the destination file path: ", dst_path) == -1) {
        fprintf(stderr, "Error: could not read the path\n");
        return 1;
    }

    int src = open(src_path, O_RDONLY);
    if (src == -1) {
        perror("open source");
        return 1;
    }

    int dst = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst == -1) {
        perror("open destination");
        close(src);
        return 1;
    }

    char buf[BUF_SIZE];
    long total = 0;
    ssize_t n;

    while ((n = read(src, buf, BUF_SIZE)) > 0) {
        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(dst, buf + written, n - written);
            if (w == -1) {
                perror("write");
                close(src);
                close(dst);
                return 1;
            }
            written += w;
        }
        total += n;
    }

    if (n == -1) {
        perror("read");
        close(src);
        close(dst);
        return 1;
    }

    if (close(src) == -1) {
        perror("close source");
        close(dst);
        return 1;
    }
    if (close(dst) == -1) {
        perror("close destination");
        return 1;
    }

    printf("Copied %ld bytes from %s to %s\n", total, src_path, dst_path);
    return 0;
}