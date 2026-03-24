#include "shell.h"

Job *job_list = NULL;
int next_job_id = 1;

void init_shell() {
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);
}

void add_job(pid_t pid, char *cmd) {
    Job *new_job = (Job *)malloc(sizeof(Job));
    if (!new_job) {
        perror("malloc failed");
        return;
    }
    new_job->id = next_job_id++;
    new_job->pid = pid;
    new_job->cmd = strdup(cmd); // Malloc for string
    new_job->status = 0; // Running
    new_job->next = NULL;

    if (job_list == NULL) {
        job_list = new_job;
    } else {
        Job *current = job_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_job;
    }
    printf("[%d] %d\n", new_job->id, pid);
}

void delete_job(pid_t pid) {
    Job *current = job_list;
    Job *prev = NULL;

    while (current != NULL) {
        if (current->pid == pid) {
            if (prev == NULL) {
                job_list = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->cmd);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void print_jobs() {
    // First, cleanup done jobs
    Job *current = job_list;
    Job *prev = NULL;
    
    // First pass: Print all jobs, but separate logic for DONE vs RUNNING?
    // User wants "delete after run using free with while loops".
    // Standard shell: "jobs" prints list. When jobs finish, shell prints "Done" immediately (async) or on next prompt.
    // Let's implement cleanup here.
    
    current = job_list;
    while (current != NULL) {
        if (current->status == 1) { // Done
             printf("[%d] Done %s\n", current->id, current->cmd);
             // Remove from list
             Job *to_free = current;
             if (prev == NULL) {
                 job_list = current->next;
                 current = job_list;
             } else {
                 prev->next = current->next;
                 current = current->next;
             }
             free(to_free->cmd);
             free(to_free);
        } else {
             printf("[%d] Running %s\n", current->id, current->cmd);
             prev = current;
             current = current->next;
        }
    }
}

void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        Job *current = job_list;
        while (current != NULL) {
            if (current->pid == pid) {
                current->status = 1; // Mark as done
                break;
            }
            current = current->next;
        }
    }
}

void sigint_handler(int sig) {
    (void)sig;
    ssize_t written = write(STDOUT_FILENO, "\n", 1);
    (void)written;
}

