#include "shell.h"

static int should_exit = 0;

static char *trim_whitespace(char *s)
{
    while (*s == ' ' || *s == '\t')
    {
        s++;
    }

    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t'))
    {
        s[len - 1] = '\0';
        len--;
    }

    return s;
}

static int execute_single_command_line(char *command_line)
{
    char *trimmed = trim_whitespace(command_line);
    char expanded[COMMAND_BUFFER_SIZE * 2];
    char *args[MAX_ARGS];
    char *input_file = NULL;
    char *output_file = NULL;
    char *commands[MAX_PIPES][MAX_ARGS];
    int is_background = 0;

    if (strlen(trimmed) == 0)
    {
        return 0;
    }

    if (strcmp(trimmed, "exit") == 0)
    {
        should_exit = 1;
        return 0;
    }

    if (strcmp(trimmed, "jobs") == 0)
    {
        print_jobs();
        return 0;
    }

    if (strcmp(trimmed, "alias") == 0 || strncmp(trimmed, "alias ", 6) == 0)
    {
        return handle_alias_command(trimmed);
    }

    if (expand_alias_command(trimmed, expanded, sizeof(expanded)) != 0)
    {
        return 1;
    }

    trimmed = trim_whitespace(expanded);

    if (strcmp(trimmed, "exit") == 0)
    {
        should_exit = 1;
        return 0;
    }

    if (strcmp(trimmed, "jobs") == 0)
    {
        print_jobs();
        return 0;
    }

    {
        char cd_parse_buffer[COMMAND_BUFFER_SIZE * 2];
        char *cd_args[MAX_ARGS];
        int cd_background = 0;
        char *cd_input_file = NULL;
        char *cd_output_file = NULL;
        int cd_argc;

        strncpy(cd_parse_buffer, trimmed, sizeof(cd_parse_buffer) - 1);
        cd_parse_buffer[sizeof(cd_parse_buffer) - 1] = '\0';
        cd_argc = parse_command_with_input_output(
            cd_parse_buffer,
            cd_args,
            &cd_input_file,
            &cd_output_file,
            &cd_background
        );

        if (cd_argc > 0 && strcmp(cd_args[0], "cd") == 0)
        {
            return builtin_cd(cd_args, cd_argc) == 0 ? 0 : 1;
        }
    }

    {
        char command_line_copy[COMMAND_BUFFER_SIZE * 2];
        char command_line_for_pipe[COMMAND_BUFFER_SIZE * 2];
        int num_commands;
        int argc;

        memset(args, 0, sizeof(args));
        memset(commands, 0, sizeof(commands));

        strncpy(command_line_copy, trimmed, sizeof(command_line_copy) - 1);
        command_line_copy[sizeof(command_line_copy) - 1] = '\0';
        strncpy(command_line_for_pipe, trimmed, sizeof(command_line_for_pipe) - 1);
        command_line_for_pipe[sizeof(command_line_for_pipe) - 1] = '\0';

        num_commands = parse_commands_with_pipes(command_line_for_pipe, commands, &is_background);

        if (num_commands == 1)
        {
            argc = parse_command_with_input_output(command_line_copy, args, &input_file, &output_file, &is_background);
            if (argc > 0)
            {
                return execute_command_with_input_output(args, input_file, output_file, is_background, args[0]);
            }
            return 0;
        }

        return execute_piped_commands(commands, num_commands, input_file, output_file, is_background);
    }
}

static int execute_with_logical_operators(char *command_line)
{
    char parse_buffer[COMMAND_BUFFER_SIZE * 2];
    char *segments[32];
    char ops[32];
    int segment_count = 0;
    int status = 0;
    char *p;

    strncpy(parse_buffer, command_line, sizeof(parse_buffer) - 1);
    parse_buffer[sizeof(parse_buffer) - 1] = '\0';

    p = parse_buffer;
    segments[segment_count++] = p;

    while (*p != '\0')
    {
        if (p[0] == '&' && p[1] == '&')
        {
            *p = '\0';
            ops[segment_count - 1] = '&';
            p += 2;
            while (*p == ' ' || *p == '\t')
            {
                p++;
            }
            if (segment_count < 32)
            {
                segments[segment_count++] = p;
            }
            continue;
        }

        if (p[0] == '|' && p[1] == '|')
        {
            *p = '\0';
            ops[segment_count - 1] = '|';
            p += 2;
            while (*p == ' ' || *p == '\t')
            {
                p++;
            }
            if (segment_count < 32)
            {
                segments[segment_count++] = p;
            }
            continue;
        }

        p++;
    }

    for (int i = 0; i < segment_count; i++)
    {
        char *segment = trim_whitespace(segments[i]);

        if (strlen(segment) == 0)
        {
            continue;
        }

        if (i == 0)
        {
            status = execute_single_command_line(segment);
            if (should_exit)
            {
                return status;
            }
            continue;
        }

        if (ops[i - 1] == '&')
        {
            if (status == 0)
            {
                status = execute_single_command_line(segment);
            }
        }
        else if (ops[i - 1] == '|')
        {
            if (status != 0)
            {
                status = execute_single_command_line(segment);
            }
        }

        if (should_exit)
        {
            return status;
        }
    }

    return status;
}

int main(int argc, char *argv[])
{
    const char *prompt = "shell208> ";
    char command_line[COMMAND_BUFFER_SIZE];
    int interactive_mode;

    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [script_file]\n", argv[0]);
        return 1;
    }

    if (argc == 2)
    {
        if (freopen(argv[1], "r", stdin) == NULL)
        {
            perror(argv[1]);
            return 1;
        }
    }

    interactive_mode = isatty(STDIN_FILENO);

    init_shell();

    while (1)
    {
        if (interactive_mode)
        {
            printf("%s", prompt);
            fflush(stdout);
        }

        memset(command_line, 0, sizeof(command_line));

        int result = get_command(command_line, COMMAND_BUFFER_SIZE);

        if (result == COMMAND_END_OF_FILE)
        {
            if (interactive_mode)
            {
                // Ctrl+D sends EOF on an empty prompt line: exit shell gracefully.
                putchar('\n');
            }
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

        execute_with_logical_operators(command_line);
        if (should_exit)
        {
            break;
        }
    }

    cleanup_all_jobs();
    cleanup_aliases();

    return 0;
}
