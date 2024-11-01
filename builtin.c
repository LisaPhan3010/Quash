#include "builtin.h"
#include "parse_interface.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> // Include signal.h for kill function
#include <stdbool.h> // Include stdbool.h for using bool type
#include <unistd.h> // POSIX operating system API
#include <sys/types.h> // data types
#include <sys/wait.h> // waitpid function
#include <fcntl.h> // file control options

#define MAX_JOBS 100
BackgroundJob jobs[MAX_JOBS];
int jobCount = 0;

bool cdCmd(const ParsedCommand* command) {
    const char *targetDir;
    
    // Determine the target directory: either the HOME variable or the provided argument
    if (command->args[1] == NULL) { 
        targetDir = getenv("HOME");
        if (targetDir == NULL) {
            fprintf(stderr, "cd: invalid environment variable\n");
            return true;
        }
    } else {
        targetDir = command->args[1];
    }

    // change the directory
    if (chdir(targetDir) != 0) {
        perror("cd");  
        return true;
    }

    // Update the PWD environment variable to reflect the new working directory
    char *newDir = getcwd(NULL, 0);  
    if (newDir != NULL) {
        setenv("PWD", newDir, 1);  
        free(newDir);  
    } else {
        perror("cd");  
    }

    return true;
}

// Print the current working directory
bool pwdCmd(const ParsedCommand* command) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd); // Print current working directory
    } else {
        perror("pwd"); // Print error message
    }
    return true;
}

// Print arguments or environment variable values
bool echoCmd(const ParsedCommand* command) {
    // Backup the original stdout file descriptor
    int original_stdout = dup(STDOUT_FILENO);

    // Handle output redirection if specified
    if (command->outfile != NULL) {
        int fd_out = open(command->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0) {
            perror("Error opening output file");
            return true;
        }
        dup2(fd_out, STDOUT_FILENO); // Redirect stdout to file
        close(fd_out);
    }

    // Print each argument of echo command
    for (int i = 1; command->args[i] != NULL; ++i) {
        if (command->args[i][0] == '$') {
            // Environment variable handling
            char* env_var = getenv(command->args[i] + 1);
            if (env_var != NULL) {
                printf("%s ", env_var);
            }
        } else {
            printf("%s ", command->args[i]);
        }
    }
    printf("\n");

    // Restore original stdout
    dup2(original_stdout, STDOUT_FILENO);
    close(original_stdout);

    return true;
}


// Set environment variable
bool exportCmd(const ParsedCommand* command) {
    if (command->args[1] == NULL) { // No argument provided
        fprintf(stderr, "export: missing argument\n"); // Print error message
        return false;
    }

    // Split the argument into key and value using "=" as a delimiter
    char* argCopy = strdup(command->args[1]); // Duplicate the argument for safe tokenization
    char* key = strtok(argCopy, "="); // Get key part
    char* value = strtok(NULL, "="); // Get value part

    if (key != NULL) { // If the key is valid
        if (value != NULL) { // If the value is valid
            if (value[0] == '$') { // Check if the value begins with $
                char* env_var = getenv(value + 1); // Get the referenced environment variable
                if (env_var != NULL) { // If the environment variable is valid
                    setenv(key, env_var, 1);
                } else {
                    fprintf(stderr, "%s not found\n", value + 1);
                }
            } else { // Otherwise, set the environment variable directly
                if (setenv(key, value, 1) != 0) {
                    perror("export");
                }
            }
        } else { // If no value is provided, set the environment variable to empty
            setenv(key, "", 1);
        }
    } else {
        fprintf(stderr, "export: invalid argument\n");
    }

    free(argCopy); // Free the duplicated argument
    return true;
}

// Display current background jobs
bool jobsCmd(const ParsedCommand* command) {
    //printf("Background jobs:\n");
    for (int i = 0; i < jobCount; i++) {
        printf("Background jobs:\n");
        printf("[%d] %d %s: %s\n", jobs[i].job_id, jobs[i].pid,
              (jobs[i].status == 0) ? "Running" :
              (jobs[i].status == 1) ? "Stopped" : "Completed",
              jobs[i].command);
    }
    if (jobCount == 0) {
        //printf("No background jobs\n");
        return false;
    }
    return true;
}
// Terminate a job by job ID or PID
bool killCmd(const ParsedCommand* command) {
    if (command->args[1] == NULL) {
        fprintf(stderr, "kill: missing argument\n");
        return false;
    }
    if (command->args[1][0] == '%') { // Job ID
        perror("I'm here");
        int jobId = atoi(command->args[1] + 1);
        for (int i = 0; i < jobCount; i++) {
            if (jobs[i].job_id == jobId) {
                if (kill(jobs[i].pid, SIGTERM) == 0) {
                    printf("Killed job [%d] with PID %d\n", jobId, jobs[i].pid);
                    for (int j = i; j < jobCount -1; j++) {
                        jobs[j] = jobs[j + 1];
                    }
                    jobCount--;
                    return true;
                } else {
                    perror("kill");
                }
                return true;
            }
        }
        fprintf(stderr, "No such job: %d\n", jobId);
    } else { // Process ID
        pid_t pid = atoi(command->args[1]);
        if (kill(pid, SIGTERM) == 0) {
            printf("Killed process with PID %d\n", pid);

            for (int i = 0; i < jobCount; i++) {
                if (jobs[i].pid == pid) {
                    for (int j = i; j < jobCount - 1; j++) {
                        jobs[j] = jobs[j + 1];
                    }
                    jobCount--;
                    return true;
                }
            }
        } else {
            perror("kill");
        }
        return true;
    }
    return false;
}

// Exit the shell when quit or exit command is invoked
bool quitCmd(const ParsedCommand* command) {
    if (strcmp(command->args[0], "quit") == 0 || strcmp(command->args[0], "exit") == 0) {
        exit(0);
    }
    return false;
}

// Check if a command is a builtin command
bool isBuiltin(const ParsedCommand* command) {
    if (command->args == NULL || command->args[0] == NULL) { // First argument is empty
        return false; // Return false
    }
    // Check if a command is a builtin command
    return strcmp(command->args[0], "cd") == 0 ||
           strcmp(command->args[0], "pwd") == 0 ||
           strcmp(command->args[0], "echo") == 0 ||
           strcmp(command->args[0], "export") == 0 ||
           strcmp(command->args[0], "jobs") == 0 ||
           strcmp(command->args[0], "kill") == 0 ||
           quitCmd(command);
}

// Execute the appropriate builtin command based on the first argument
bool execBuiltin(const ParsedCommand* command) {
    if (strcmp(command->args[0], "cd") == 0) {
        return cdCmd(command);
    } else if (strcmp(command->args[0], "pwd") == 0) {
        return pwdCmd(command);
    } else if (strcmp(command->args[0], "echo") == 0) {
        return echoCmd(command);
    } else if (strcmp(command->args[0], "export") == 0) {
        return exportCmd(command);
    } else if (strcmp(command->args[0], "jobs") == 0) {
        return jobsCmd(command);
    } else if (strcmp(command->args[0], "kill") == 0) {
        return killCmd(command);
    }
    return quitCmd(command); // Handle quit command
}

