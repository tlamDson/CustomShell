#include "shell.h"

int execute_piped_commands(char *commands[MAX_PIPES][MAX_ARGS], int num_commands, char *input_file, char *output_file, int is_background)
{
    int pipe_fds[2 * (num_commands - 1)];
    int i;
    pid_t pids[MAX_PIPES];

    for (i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipe_fds + i * 2) < 0)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < num_commands; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        { 
            // Child
            if (is_background)
            {
                setpgid(0, 0); // Detach background jobs from shell process group
            }

            // Child should react to Ctrl+C normally.
            signal(SIGINT, SIG_DFL);

            if (i == 0 && input_file)
            { 
                if (setup_stdin_redirection(input_file) == -1)
                    exit(EXIT_FAILURE);
            }
            if (i == num_commands - 1 && output_file)
            { 
                if (setup_stdout_redirection(output_file) == -1)
                    exit(EXIT_FAILURE);
            }
            if (i != 0)
            { 
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i != num_commands - 1)
            { 
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }
            
            for (int j = 0; j < 2 * (num_commands - 1); j++)
            {
                close(pipe_fds[j]);
            }
            run_external_command(commands[i]);
        }
        else if (pids[i] < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < 2 * (num_commands - 1); i++)
    {
        close(pipe_fds[i]);
    }

    if (is_background) {
        // Add the LAST command to jobs list? Or all of them?
        // Typically the shell tracks the process group or just the last PID.
        // For simplicity, let's track the last PID as the "job".
        // A better implementation would track the whole pipeline.
        char cmd_name[100];
        snprintf(cmd_name, sizeof(cmd_name), "%s | ...", commands[0][0]);
        add_job(pids[num_commands-1], cmd_name); 
        return 0;
    } else {
        // Wait for all children
        int status = 0;
        for (i = 0; i < num_commands; i++)
        {
            while (waitpid(pids[i], &status, 0) == -1 && errno == EINTR)
            {
            }
        }

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
