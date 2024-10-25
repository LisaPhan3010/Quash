#include "builtin.h"
#include "parse_interface.h"
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


BackgroundJob jobs[100];
bool cdCmd(const ParsedCommand* command) {
    if (command->args[1] == NULL) { //in case there is no target directory provided
        char* home = getenv("HOME");
        if (home != NULL) {
            if (chdir(home) != 0) {
            perror("cd"); //print error message
            }
        } else {
            fprintf(stderr, "cd:invalid environment variable");
        }
    } else if (chdir(command->args[1]) != 0) { //change to target directory
        perror("cd"); //if fails print error message
    }
    return true;
}

bool pwdCmd(const ParsedCommand* command) { 
    char cwd[1024]; //create a place to store the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) { //in case the cwd is not empty
        printf("%s\n", cwd); //print the cwd  
    } else { //otherwise, return fail and print error message
        perror("pwd");
    }
    return true;
}

bool echoCmd(const ParsedCommand* command) {
    for (int i = 1; command->args[i] != NULL; ++i) {  //loop for arguments starting from index 1, and it is not empty
         //printf("%s ", command->args[i]); //print each argument
         if (i > 1) {
            printf(" ");
         }
         if (command->args[i][0] == '$') {
            char* env_var = getenv(command->args[i] + 1);
            if (env_var != NULL) {
                printf("%s", env_var);
            } else {
                printf("%s", command->args[i]);
            }
         } else {
            printf("%s", command->args[i]);
         }
    }
    printf("\n"); //print a new line at the end
    return true;
}

bool exportCmd(const ParsedCommand* command) {
    if (command->args[1] == NULL) { //in case the argument is empty
        perror("export"); //print error message
        return false;
    }
    //split the argument in to key and value with the delimeter "="
    char* key = strtok(strdup(command->args[1]), "="); //set up key part
    char* value = strtok(NULL, "="); //set up value part
        if (key != NULL) { //in case the key is valid
            if (value != NULL) { //if the value is also valid
                if (value[0] == '$') { //then check if the value begins with $ denotes a variable reference
                    char* env_var = getenv(value + 1); //get the referenced environment variable 
                    if (env_var != NULL) { //in case the environemnt variable is valid
                        setenv(key, env_var, 1);
                    } else {
                        fprintf(stderr, "%s not found\n", value + 1);
                    }
                } else { //otherwise, set up the direct environment variable
                    if (setenv(key, value, 1) != 0) { 
                        perror("export");
                    }

                }
            } else { //otherwise, set the environment variable to empty
                setenv(key, "", 1);
            }
        } else {
            fprintf(stderr, "export: invalid argument\n");
        }
        free(strdup(command->args[1]));
        return true;
}



bool jobsCmd(const ParsedCommand* command) {
    int jobCount = 0;
    for (int i = 0; i < jobCount; i++) {
        printf("[%d] %s %s\n", i + 1,
              (jobs[i].status == 0) ? "Running" :
              (jobs[i].status == 1) ? "Stopped" : "Completed",
              jobs[i].command);
    }
    printf("Background jobs:\n"); //placeholder for jobs command
    return true;
}

bool quitCmd(const ParsedCommand* command) {
    //if the argument is "quit" or "exit", return true
    if (strcmp(command->args[0], "quit") == 0 || strcmp(command->args[0], "exit") == 0) {
        exit(0);
    }
}

bool isBuiltin(const ParsedCommand* command) {
    if (command->args == NULL || command->args[0] == NULL) { //in case the first argument is empty
        return false; //return false
    }
    //check if a command is a builtin command
    return strcmp(command->args[0], "cd") == 0 ||
           strcmp(command->args[0], "pwd") == 0 ||
           strcmp(command->args[0], "echo") == 0 ||
           strcmp(command->args[0], "export") == 0 ||
           strcmp(command->args[0], "jobs") == 0 ||
           quitCmd(command); 
}

bool execBuiltin(const ParsedCommand* command) {
    //execute the appropriate builtin command from the first argument
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
    } else if (quitCmd(command)) {
        exit(0); 
    }
    return false; //in case the command is not a builtin command
}
