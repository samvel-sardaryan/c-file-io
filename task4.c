#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define LINE_SIZE 256

int main(void) {
    int fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    printf("Enter a log line: ");
    fflush(stdout);

    /* read one byte at a time, so the read stops right after the newline */
    char line[LINE_SIZE];
    int len = 0;
    while (len < LINE_SIZE - 1) {
        ssize_t n = read(STDIN_FILENO, &line[len], 1);
        if (n == -1) {
            perror("read");
            close(fd);
            return 1;
        }
        if (n == 0) {
            break;
        }
        len++;
        if (line[len - 1] == '\n') {
            break;
        }
    }

    char entry[LINE_SIZE + 32];
    int total = snprintf(entry, sizeof(entry), "PID=%d: ", (int)getpid());
    memcpy(entry + total, line, len);
    total += len;
    if (len == 0 || line[len - 1] != '\n') {
        entry[total] = '\n';
        total++;
    }

    off_t before = lseek(fd, 0, SEEK_CUR);
    if (before == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    ssize_t w = write(fd, entry, total);
    if (w == -1) {
        perror("write");
        close(fd);
        return 1;
    }
    if (w != total) {
        fprintf(stderr, "Error: only %zd of %d bytes were written\n", w, total);
        close(fd);
        return 1;
    }

    /*
     * O_APPEND doesn't turn the file offset off. Right after open it's 0,
     * as "before" shows. O_APPEND only makes the kernel move it to the end
     * of the file right before every write. The write puts the data there
     * and leaves the offset right after it, at the new end of the file.
     * So SEEK_CUR returns the new file size, and since every run adds one
     * more entry, that number grows every time.
     */
    off_t after = lseek(fd, 0, SEEK_CUR);
    if (after == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("Offset before the write: %ld\n", (long)before);
    printf("Wrote %d bytes, offset after the write: %ld\n", total, (long)after);

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }
    return 0;
}