#include "shell.h"

#include <termios.h>

#define HISTORY_SIZE 128

static char command_history[HISTORY_SIZE][COMMAND_BUFFER_SIZE];
static int history_count = 0;

static void add_history_entry(const char *line)
{
    if (line == NULL || line[0] == '\0')
    {
        return;
    }

    if (history_count > 0 && strcmp(command_history[history_count - 1], line) == 0)
    {
        return;
    }

    if (history_count < HISTORY_SIZE)
    {
        strncpy(command_history[history_count], line, COMMAND_BUFFER_SIZE - 1);
        command_history[history_count][COMMAND_BUFFER_SIZE - 1] = '\0';
        history_count++;
        return;
    }

    for (int i = 1; i < HISTORY_SIZE; i++)
    {
        strncpy(command_history[i - 1], command_history[i], COMMAND_BUFFER_SIZE);
    }
    strncpy(command_history[HISTORY_SIZE - 1], line, COMMAND_BUFFER_SIZE - 1);
    command_history[HISTORY_SIZE - 1][COMMAND_BUFFER_SIZE - 1] = '\0';
}

static void redraw_input_line(const char *line)
{
    printf("\r\033[2K%s%s", SHELL_PROMPT, line);
    fflush(stdout);
}

char *get_full_path(char *command)
{
    if (command == NULL || command[0] == '\0')
    {
        return NULL;
    }

    // Absolute/relative path provided directly by user.
    if (strchr(command, '/') != NULL)
    {
        return strdup(command);
    }

    char *path_env = getenv("PATH");
    if (path_env == NULL)
    {
        return NULL;
    }

    char *path_copy = strdup(path_env);
    if (path_copy == NULL)
    {
        return NULL;
    }

    char *saveptr = NULL;
    char *dir = strtok_r(path_copy, ":", &saveptr);
    while (dir != NULL)
    {
        size_t needed = strlen(dir) + 1 + strlen(command) + 1;
        char *candidate = (char *)malloc(needed);
        if (candidate == NULL)
        {
            free(path_copy);
            return NULL;
        }

        snprintf(candidate, needed, "%s/%s", dir, command);
        if (access(candidate, X_OK) == 0)
        {
            free(path_copy);
            return candidate;
        }

        free(candidate);
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_copy);
    return NULL;
}

void run_external_command(char *args[])
{
    if (args == NULL || args[0] == NULL)
    {
        exit(EXIT_FAILURE);
    }

    char *full_path = get_full_path(args[0]);
    if (full_path != NULL)
    {
        execv(full_path, args);
        perror("execv failed");
        free(full_path);
    }
    else
    {
        fprintf(stderr, "Command not found: %s\n", args[0]);
    }

    exit(EXIT_FAILURE);
}

int get_command(char *command_buffer, int buffer_size)
{
    assert(buffer_size > 0);
    assert(command_buffer != NULL);

    command_buffer[0] = '\0';

    if (!isatty(STDIN_FILENO))
    {
        if (fgets(command_buffer, buffer_size, stdin) == NULL)
        {
            if (feof(stdin))
            {
                return COMMAND_END_OF_FILE;
            }
            else if (errno == EINTR)
            {
                clearerr(stdin);
                return COMMAND_INPUT_SUCCEEDED;
            }
            else
            {
                return COMMAND_INPUT_FAILED;
            }
        }

        int command_length = strlen(command_buffer);
        if (command_length == buffer_size - 1 && command_buffer[command_length - 1] != '\n')
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }
            command_buffer[0] = '\0';
            return COMMAND_TOO_LONG;
        }

        if (command_length > 0 && command_buffer[command_length - 1] == '\n') {
            command_buffer[command_length - 1] = '\0';
            command_length--;
        }
        if (command_length > 0 && command_buffer[command_length - 1] == '\r') {
            command_buffer[command_length - 1] = '\0';
            command_length--;
        }

        return COMMAND_INPUT_SUCCEEDED;
    }

    struct termios oldt;
    struct termios newt;
    if (tcgetattr(STDIN_FILENO, &oldt) == -1)
    {
        return COMMAND_INPUT_FAILED;
    }

    newt = oldt;
    newt.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1)
    {
        return COMMAND_INPUT_FAILED;
    }

    int length = 0;
    int history_index = history_count;
    command_buffer[0] = '\0';

    while (1)
    {
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);

        if (n == -1)
        {
            if (errno == EINTR)
            {
                command_buffer[0] = '\0';
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                return COMMAND_INPUT_SUCCEEDED;
            }

            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return COMMAND_INPUT_FAILED;
        }

        if (n == 0)
        {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return COMMAND_END_OF_FILE;
        }

        if (c == '\r' || c == '\n')
        {
            command_buffer[length] = '\0';
            putchar('\n');
            fflush(stdout);
            add_history_entry(command_buffer);
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return COMMAND_INPUT_SUCCEEDED;
        }

        if (c == 4)
        {
            if (length == 0)
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                return COMMAND_END_OF_FILE;
            }
            continue;
        }

        if ((c == 127 || c == 8) && length > 0)
        {
            length--;
            command_buffer[length] = '\0';
            redraw_input_line(command_buffer);
            continue;
        }

        if (c == 27)
        {
            unsigned char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1 || read(STDIN_FILENO, &seq[1], 1) != 1)
            {
                continue;
            }

            if (seq[0] == '[' && seq[1] == 'A')
            {
                if (history_count > 0 && history_index > 0)
                {
                    history_index--;
                    strncpy(command_buffer, command_history[history_index], buffer_size - 1);
                    command_buffer[buffer_size - 1] = '\0';
                    length = strlen(command_buffer);
                    redraw_input_line(command_buffer);
                }
                continue;
            }

            if (seq[0] == '[' && seq[1] == 'B')
            {
                if (history_count > 0 && history_index < history_count)
                {
                    history_index++;
                    if (history_index == history_count)
                    {
                        command_buffer[0] = '\0';
                    }
                    else
                    {
                        strncpy(command_buffer, command_history[history_index], buffer_size - 1);
                        command_buffer[buffer_size - 1] = '\0';
                    }
                    length = strlen(command_buffer);
                    redraw_input_line(command_buffer);
                }
                continue;
            }

            continue;
        }

        if (c >= 32 && c < 127)
        {
            if (length < buffer_size - 1)
            {
                command_buffer[length++] = (char)c;
                command_buffer[length] = '\0';
                putchar((int)c);
                fflush(stdout);
            }
            continue;
        }
    }

    return COMMAND_INPUT_FAILED;
}

int parse_command_with_input_output(char *command_line, char *args[], char **input_file, char **output_file, int *is_background)
{
    int argc = 0;
    char *token = strtok(command_line, " ");
    *is_background = 0;

    while (token != NULL)
    {
        if (strcmp(token, ">") == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Syntax error: Expected output file name after '>'\n");
                return -1;
            }
            *output_file = token;
        }
        else if (strcmp(token, "<") == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Syntax error: Expected input file name after '<'\n");
                return -1;
            }
            *input_file = token;
        }
        else if (strcmp(token, "&") == 0)
        {
            *is_background = 1;
        }
        else
        {
            args[argc++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[argc] = NULL;
    return argc;
}

int parse_commands_with_pipes(char *command_line, char *commands[MAX_PIPES][MAX_ARGS], int *is_background)
{
    int command_count = 0;
    int arg_count = 0;
    *is_background = 0;

    char *token = strtok(command_line, " ");
    while (token != NULL)
    {
        if (strcmp(token, "|") == 0)
        {
            commands[command_count][arg_count] = NULL;
            command_count++;
            arg_count = 0;
        }
        else if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0)
        {
            strtok(NULL, " "); // Skip file name
        }
        else if (strcmp(token, "&") == 0)
        {
            *is_background = 1;
        }
        else
        {
            commands[command_count][arg_count++] = token;
        }
        token = strtok(NULL, " ");
    }
    commands[command_count][arg_count] = NULL;
    return command_count + 1;
}

int execute_command_with_input_output(char *args[], char *input_file, char *output_file, int is_background, char *original_cmd)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        // Child Process
        if (is_background)
        {
            setpgid(0, 0); // Detach background jobs from shell process group
        }

        // Child should react to Ctrl+C normally.
        signal(SIGINT, SIG_DFL);

        if (input_file != NULL && setup_stdin_redirection(input_file) == -1)
        {
            exit(1);
        }

        if (output_file != NULL && setup_stdout_redirection(output_file) == -1)
        {
            exit(1);
        }

        if (strcmp(args[0], "help") == 0)
        {
            print_help();
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(args[0], "jobs") == 0)
        {
             print_jobs();
             exit(EXIT_SUCCESS);
        }
        else
        {
            run_external_command(args);
        }
    }
    else if (pid > 0)
    {
        /* Parent */
        if (is_background) {
            add_job(pid, original_cmd ? original_cmd : args[0]);
            // Don't wait!
            return 0;
        } else {
            // Foreground job
            int status;
            // Retry wait if interrupted by a signal like Ctrl+C.
            while (waitpid(pid, &status, 0) == -1 && errno == EINTR)
            {
            }
            fflush(stdout);

            if (WIFEXITED(status))
            {
                return WEXITSTATUS(status);
            }
            if (WIFSIGNALED(status))
            {
                return 128 + WTERMSIG(status);
            }
            return 1;
        }
    }
    else
    {
        perror("fork failed");
        exit(1);
    }

    return 1;
}
