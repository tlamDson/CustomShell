#include "shell.h"

int main()
{
    const char *prompt = "shell208> ";
    char command_line[COMMAND_BUFFER_SIZE];
    char *args[MAX_ARGS];
    char *input_file = NULL;
    char *output_file = NULL;
    char *commands[MAX_PIPES][MAX_ARGS];
    int is_background = 0;

    init_shell();

    while (1)
    {
        printf("%s", prompt);
        fflush(stdout);

        // Reset variables
        input_file = NULL;
        output_file = NULL;
        is_background = 0;
        memset(args, 0, sizeof(args));
        memset(command_line, 0, sizeof(command_line));
        memset(commands, 0, sizeof(commands));

        int result = get_command(command_line, COMMAND_BUFFER_SIZE);

        if (result == COMMAND_END_OF_FILE)
        {
            // Ctrl+D sends EOF on an empty prompt line: exit shell gracefully.
            putchar('\n');
            cleanup_all_jobs();
            break;
        }
        else if (result == COMMAND_INPUT_FAILED)
        {
            fprintf(stderr, "There was a problem reading your command. Please try again.\n");
            break;
        }
        else if (result == COMMAND_TOO_LONG)
        {
            fprintf(stderr, "Commands are limited to length %d. Please try again.\n", COMMAND_BUFFER_SIZE - 2);
            continue;
        }

        if (strlen(command_line) == 0) continue;

        // Handle 'cd' in the parent shell process so cwd really changes.
        {
            char cd_parse_buffer[COMMAND_BUFFER_SIZE];
            char *cd_args[MAX_ARGS];
            int cd_background = 0;
            char *cd_input_file = NULL;
            char *cd_output_file = NULL;

            strncpy(cd_parse_buffer, command_line, COMMAND_BUFFER_SIZE - 1);
            cd_parse_buffer[COMMAND_BUFFER_SIZE - 1] = '\0';
            int cd_argc = parse_command_with_input_output(
                cd_parse_buffer,
                cd_args,
                &cd_input_file,
                &cd_output_file,
                &cd_background
            );

            if (cd_argc > 0 && strcmp(cd_args[0], "cd") == 0)
            {
                builtin_cd(cd_args, cd_argc);
                continue;
            }
        }

        // Handle 'exit' command directly
        if (strcmp(command_line, "exit") == 0)
        {
            cleanup_all_jobs();
            break;
        }
        
        // Handle 'jobs' command directly
        if (strcmp(command_line, "jobs") == 0)
        {
            print_jobs();
            continue;
        }

        // Make copies for parsing
        char command_line_copy[COMMAND_BUFFER_SIZE];
        strncpy(command_line_copy, command_line, COMMAND_BUFFER_SIZE);
        
        // Parse piped commands first
        int num_commands = parse_commands_with_pipes(command_line, commands, &is_background);

        if (num_commands == 1)
        {
            // Re-parse as single command to get args and redirection
            int argc = parse_command_with_input_output(command_line_copy, args, &input_file, &output_file, &is_background);
            
            if (argc > 0)
            {
                execute_command_with_input_output(args, input_file, output_file, is_background, args[0]);
            }
        }
        else
        {
             // Piped commands
             execute_piped_commands(commands, num_commands, input_file, output_file, is_background);
        }
    }

    cleanup_all_jobs();

    return 0;
}
