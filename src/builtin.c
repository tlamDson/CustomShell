#include "shell.h"

typedef struct Alias {
    char *name;
    char *value;
    struct Alias *next;
} Alias;

static Alias *alias_list = NULL;

static char *trim_inplace(char *s)
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

static Alias *find_alias(const char *name)
{
    Alias *current = alias_list;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void set_alias(const char *name, const char *value)
{
    Alias *existing = find_alias(name);
    if (existing != NULL)
    {
        char *new_value = strdup(value);
        if (new_value == NULL)
        {
            perror("alias: strdup");
            return;
        }
        free(existing->value);
        existing->value = new_value;
        return;
    }

    Alias *entry = (Alias *)malloc(sizeof(Alias));
    if (entry == NULL)
    {
        perror("alias: malloc");
        return;
    }

    entry->name = strdup(name);
    entry->value = strdup(value);
    if (entry->name == NULL || entry->value == NULL)
    {
        perror("alias: strdup");
        free(entry->name);
        free(entry->value);
        free(entry);
        return;
    }

    entry->next = alias_list;
    alias_list = entry;
}

static void print_aliases(void)
{
    Alias *current = alias_list;
    while (current != NULL)
    {
        printf("alias %s='%s'\n", current->name, current->value);
        current = current->next;
    }
}

void cleanup_aliases()
{
    Alias *current = alias_list;
    while (current != NULL)
    {
        Alias *next = current->next;
        free(current->name);
        free(current->value);
        free(current);
        current = next;
    }
    alias_list = NULL;
}

int handle_alias_command(const char *line)
{
    if (line == NULL)
    {
        return -1;
    }

    if (strcmp(line, "alias") == 0)
    {
        print_aliases();
        return 0;
    }

    if (strncmp(line, "alias ", 6) != 0)
    {
        return -1;
    }

    char buffer[COMMAND_BUFFER_SIZE * 2];
    strncpy(buffer, line + 6, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *assignment = trim_inplace(buffer);
    char *equal_sign = strchr(assignment, '=');
    if (equal_sign == NULL)
    {
        fprintf(stderr, "alias: usage alias name='value'\n");
        return 1;
    }

    *equal_sign = '\0';
    char *name = trim_inplace(assignment);
    char *value = trim_inplace(equal_sign + 1);

    if (strlen(name) == 0)
    {
        fprintf(stderr, "alias: invalid name\n");
        return 1;
    }

    size_t vlen = strlen(value);
    if (vlen >= 2 && ((value[0] == '\'' && value[vlen - 1] == '\'') || (value[0] == '"' && value[vlen - 1] == '"')))
    {
        value[vlen - 1] = '\0';
        value++;
    }

    set_alias(name, value);
    return 0;
}

int expand_alias_command(const char *input, char *output, size_t output_size)
{
    if (input == NULL || output == NULL || output_size == 0)
    {
        return -1;
    }

    char current[COMMAND_BUFFER_SIZE * 2];
    strncpy(current, input, sizeof(current) - 1);
    current[sizeof(current) - 1] = '\0';

    for (int depth = 0; depth < 16; depth++)
    {
        char working[COMMAND_BUFFER_SIZE * 2];
        strncpy(working, current, sizeof(working) - 1);
        working[sizeof(working) - 1] = '\0';

        char *trimmed = trim_inplace(working);
        if (strlen(trimmed) == 0)
        {
            break;
        }

        char *space = strpbrk(trimmed, " \t");
        char first_token[COMMAND_BUFFER_SIZE];
        const char *rest = "";

        if (space != NULL)
        {
            size_t token_len = (size_t)(space - trimmed);
            if (token_len >= sizeof(first_token))
            {
                token_len = sizeof(first_token) - 1;
            }
            memcpy(first_token, trimmed, token_len);
            first_token[token_len] = '\0';
            rest = space;
            while (*rest == ' ' || *rest == '\t')
            {
                rest++;
            }
        }
        else
        {
            strncpy(first_token, trimmed, sizeof(first_token) - 1);
            first_token[sizeof(first_token) - 1] = '\0';
        }

        Alias *alias = find_alias(first_token);
        if (alias == NULL)
        {
            break;
        }

        if (strlen(rest) > 0)
        {
            snprintf(current, sizeof(current), "%s %s", alias->value, rest);
        }
        else
        {
            snprintf(current, sizeof(current), "%s", alias->value);
        }
    }

    // Detect unresolved cycle after max expansion depth.
    {
        char working[COMMAND_BUFFER_SIZE * 2];
        strncpy(working, current, sizeof(working) - 1);
        working[sizeof(working) - 1] = '\0';
        char *trimmed = trim_inplace(working);
        char *space = strpbrk(trimmed, " \t");
        char first_token[COMMAND_BUFFER_SIZE];
        if (space != NULL)
        {
            size_t token_len = (size_t)(space - trimmed);
            if (token_len >= sizeof(first_token))
            {
                token_len = sizeof(first_token) - 1;
            }
            memcpy(first_token, trimmed, token_len);
            first_token[token_len] = '\0';
        }
        else
        {
            strncpy(first_token, trimmed, sizeof(first_token) - 1);
            first_token[sizeof(first_token) - 1] = '\0';
        }

        if (find_alias(first_token) != NULL)
        {
            fprintf(stderr, "alias: expansion loop detected\n");
            return 1;
        }
    }

    strncpy(output, current, output_size - 1);
    output[output_size - 1] = '\0';
    return 0;
}

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
    printf("alias\tCreate or list aliases\n");
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
