#include "shell.h"

// stdout redirection
int setup_stdout_redirection(const char *redirect_file) {
    int fd = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Trouble opening file");
        return -1; // Return an error code
    }
    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("dup2 failed");
        close(fd);
        return -1; // Return an error
    }
    close(fd);
    return 0; // Success
}

// stdin redirection
int setup_stdin_redirection(const char *input_file) {
    int fd = open(input_file, O_RDONLY);
    if (fd == -1) {
        perror("Trouble opening input file");
        return -1;
    }
    if (dup2(fd, STDIN_FILENO) == -1) {
        perror("dup2 failed for input");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
