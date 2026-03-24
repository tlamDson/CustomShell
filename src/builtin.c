#include "shell.h"

void print_help() {
    printf("shell208 Do you need help:\n");
    printf("Type the commands and hit enter.\n");
    printf("Here are some commands commands:\n");
    printf("help\tShow this help message\n");
    printf("exit\tExit the shell\n");
    printf("Ctrl+D\tExit the shell (EOF)\n");
    printf("jobs\tList background jobs\n");
    printf("ls\tList directory contents\n");
    printf("pwd\tPrint the working directory\n");
    printf("echo\tDisplay a line of text\n");
    printf("ls -l\tList directory contents in long format\n");
    printf("Any command followed by > and a filename redirects the output to the file.\n");
    printf("Append & to run a command in the background.\n");
}
