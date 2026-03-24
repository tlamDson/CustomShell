#include "shell.h"

int builtin_cd(char *args[], int argc)
{
    const char *target = NULL;
    const char *home = getenv("HOME");
    const char *oldpwd = getenv("OLDPWD");
    char old_cwd[PATH_MAX];
    char new_cwd[PATH_MAX];

    if (getcwd(old_cwd, sizeof(old_cwd)) == NULL)
    {
        perror("cd: getcwd");
        return -1;
    }

    if (argc > 2)
    {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }

    if (argc == 1 || strcmp(args[1], "~") == 0)
    {
        target = home;
        if (target == NULL)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return -1;
        }
    }
    else if (strcmp(args[1], "-") == 0)
    {
        target = oldpwd;
        if (target == NULL)
        {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return -1;
        }
    }
    else
    {
        target = args[1];
    }

    if (chdir(target) != 0)
    {
        fprintf(stderr, "cd: %s: %s\n", target, strerror(errno));
        return -1;
    }

    if (getcwd(new_cwd, sizeof(new_cwd)) == NULL)
    {
        perror("cd: getcwd");
        return -1;
    }

    if (setenv("OLDPWD", old_cwd, 1) != 0)
    {
        perror("cd: setenv OLDPWD");
        return -1;
    }

    if (setenv("PWD", new_cwd, 1) != 0)
    {
        perror("cd: setenv PWD");
        return -1;
    }

    if (argc == 2 && strcmp(args[1], "-") == 0)
    {
        printf("%s\n", new_cwd);
    }

    return 0;
}

void print_help() {
    printf("shell208 Do you need help:\n");
    printf("Type the commands and hit enter.\n");
    printf("Here are some commands commands:\n");
    printf("help\tShow this help message\n");
    printf("cd\tChange current directory\n");
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
