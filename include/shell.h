#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#define COMMAND_BUFFER_SIZE 102
#define MAX_ARGS 128
#define MAX_PIPES 10
#define MAX_JOBS 20

// Return values for get_command
#define COMMAND_INPUT_SUCCEEDED 0
#define COMMAND_INPUT_FAILED 1
#define COMMAND_END_OF_FILE 2
#define COMMAND_TOO_LONG 3

// Job structure (Linked List)
typedef struct Job {
    int id;
    pid_t pid;
    char *cmd; // Dynamic string
    int status; // 0: Running, 1: Done
    struct Job *next;
} Job;

extern Job *job_list; // Head of linked list
extern int next_job_id;

// Function prototypes
int get_command(char *command_buffer, int buffer_size);
void print_help();
int builtin_cd(char *args[], int argc);
void init_shell();

// Parsing
int parse_command_with_input_output(char *command_line, char *args[], char **input_file, char **output_file, int *is_background);
int parse_commands_with_pipes(char *command_line, char *commands[MAX_PIPES][MAX_ARGS], int *is_background);

// Redirection
int setup_stdin_redirection(const char *input_file);
int setup_stdout_redirection(const char *output_file);

// Execution
char *get_full_path(char *command);
void run_external_command(char *args[]);
int execute_command_with_input_output(char *args[], char *input_file, char *output_file, int is_background, char *original_cmd);
void execute_piped_commands(char *commands[MAX_PIPES][MAX_ARGS], int num_commands, char *input_file, char *output_file, int is_background);

// Job Control
void add_job(pid_t pid, char *cmd);
void delete_job(pid_t pid); // New function to free memory
void cleanup_all_jobs();
void print_jobs();
void sigchld_handler(int sig);
void sigint_handler(int sig);

#endif
