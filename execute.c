#include "execute.h"
#include "builtin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // POSIX operating system API
#include <sys/types.h> // data types
#include <sys/wait.h> // waitpid function
#include <fcntl.h> // file control options

#define MAX_JOBS 1024

BackgroundJob jobs[MAX_JOBS];

extern int jobCount;

// Execute parsed command
void execCmd(const ParsedCommand* command) {
    if (quitCmd(command)) {
        exit(0);
    }
    if (isBuiltin(command)) {
        execBuiltin(command);
        return;
    }

    pid_t pid = fork(); // Fork a new process

    // Child process
    if (pid == 0) {
        // Handle input redirection
        if (command->infile != NULL) {
            int fd_in = open(command->infile, O_RDONLY); // Open the input file for reading
            if (fd_in < 0) { // Error handling if the file can't be opened
                perror("Error opening input file");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO); // Redirect standard input to the input file
            close(fd_in); // Close the file descriptor after redirection
        }

        // Handle output redirection
        if (command->outfile != NULL) {
            FILE *fd_out; 
            if (command->append) {
                fd_out = fopen(command->outfile, "append"); // Open the file for appending
                //file = open(command->outfile, O_WRONLY | O_CREAT | O_APPEND, 0644); // Open for appending
            } else {
                fd_out = fopen(command->outfile, "write"); // Open the file for writing
                //file = open(command->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open for writing
            }
            if (fd_out < 0) { // Error handling for file opening failure
                perror("Error opening output file");
                exit(1); // Terminate the child process
            }
            dup2(fileno(fd_out), STDOUT_FILENO); // Redirect standard output to file descriptor
            fclose(fd_out); // Close file pointer after redirection
        }

        // Execute the user command
        execvp(command->args[0], command->args); // Replace current process with a new process
        perror("execvp failed"); // Print error message if execvp fails
        exit(1); // Exit child process
    } 
    // Fork error
    else if (pid < 0) {
        perror("Fork failed"); // Print fork error message
    } 
    else { //Sophia's code
        // Parent process
        if (command->background) {
            // Background execution: do not wait
            if (jobCount < MAX_JOBS) {
                jobs[jobCount].job_id = jobCount + 1;
                jobs[jobCount].pid = pid;
                strncpy(jobs[jobCount].command, command->args[0], sizeof(jobs[jobCount].command) - 1);
                jobs[jobCount].command[sizeof(jobs[jobCount].command) - 1] = '\0'; // ensure null-termination
                printf("Background job started: [%d] %d %s &\n", jobs[jobCount].job_id, jobs[jobCount].pid, jobs[jobCount].command);
                jobCount++;
            } else {
                fprintf(stderr, "Maximum number of background jobs reached.\n");
            }
        } else {
            waitpid(pid, NULL, 0); // wait until the child process is finished
        }
    }
}
//sophia's code
// Handle completed background jobs
void handleCompletedJobs() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Find the job in the jobs array
        for (int i = 0; i < jobCount; i++) {
            if (jobs[i].pid == pid) {
                printf("Completed: [%d] %d %s &\n", jobs[i].job_id, jobs[i].pid, jobs[i].command);
                // Remove the job from the array by shifting others
                for (int j = i; j < jobCount - 1; j++) {
                    jobs[j] = jobs[j + 1];
                }
                jobCount--;
                break;
            }
        }
    }
} 
