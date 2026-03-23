#include "shell.h"

int get_command(char *command_buffer, int buffer_size)
{
    assert(buffer_size > 0);
    assert(command_buffer != NULL);

    if (fgets(command_buffer, buffer_size, stdin) == NULL)
    {
        if (feof(stdin))
        {
            return COMMAND_END_OF_FILE;
        }
        else
        {
            return COMMAND_INPUT_FAILED;
        }
    }

    int command_length = strlen(command_buffer);
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
        setpgid(0, 0); // Create new process group

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
             // jobs command logic handled in parent mostly, but here it's executed in child?
             // Actually, jobs should be a built-in executed by parent!
             // But for now let's leave it. If executed in child, it accesses parent memory? NO.
             // Fork copies memory. So child has a COPY of jobs array.
             // This works for printing, but not for modifying.
             print_jobs();
             exit(EXIT_SUCCESS);
        }
        else
        {
            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        }
    }
    else if (pid > 0)
    {
        /* Parent */
        if (is_background) {
            add_job(pid, original_cmd ? original_cmd : args[0]);
            // Don't wait!
        } else {
            // Foreground job
            int status;
            // Wait for this specific child
            waitpid(pid, &status, 0); 
            fflush(stdout);
        }
    }
    else
    {
        perror("fork failed");
        exit(1);
    }

    return 0;
}
