#include "parse_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//count number of tokens in the input
static int countToken(const char* input) {
    int count = 0; //define count of token
    char *copy = strdup(input); //define a copy of input that can be modify later
    char *token = strtok(copy, " \t\n"); //tokenize the input string
    while (token != NULL) { //in case token is not empty, create a loop through each token
        count++; //update token count
        token = strtok(NULL, " \t\n"); //keep tokenizing the string remained
    }
    free(copy); //free the string copy
    return count;
}

bool parseCmd(const char* input, ParsedCommand* command) { 
   // ParsedCommand *command = malloc(sizeof(ParsedCommand)); //allocate memory for ParsedCommand
    command->args = NULL; //locate args to NULL
    command->infile = NULL; //locate input file to NULL
    command->outfile = NULL; //locate output file to NULL
    command->append = false; //locate append flag to false
    command->background = false; //locate backgroung flag to false
    int cntToken = countToken(input);
    if(cntToken == 0) {
        //free(command); //in case there is not token, free the command structure
        return false; }
    command->args = malloc(sizeof(char*) * (cntToken + 1)); //allocate memory for args (+1 for NULL termination)
    char *copy = strdup(input); 
    char *token = strtok(copy, " \t\n");
    int argIndex = 0; //index of arguments in args

    while (token != NULL) {
        if (strcmp(token, "<") == 0) { //in case token is '<' (specifies an input file), the next token become the input file
            token = strtok(NULL, " \t\n");
            if (token == NULL) {
                fprintf(stderr, "invalid input file");
                free_parsedCmd(command);
                free(copy);
                return false;
            }
            command->infile = strdup(token); //store the input file in ParsedCommand
            if (command->infile == NULL) { //memory allocation failure handler
                    free_parsedCmd(command);
                    free(copy);
                    return false;
            }
        } else if (strcmp(token, ">") == 0) { //in case token is '>' (specifies an output file), the next token become the output file
            token = strtok(NULL, " \t\n");
            if (token != NULL) {
                command->outfile = strdup(token); //store the output file in ParsedCommand
                if (command->outfile == NULL) { //memory allocation failure handler
                    perror("strdup");
                    free_parsedCmd(command);
                    free(copy);
                    return false;
                }
                command->append = false; //set append mode to false
            }
        } else if (strcmp(token, ">>") == 0) { //in case the token is '>>' (append to the file), the next token become the output file
            token = strtok(NULL, " \t\n");
            if (token != NULL) {
                command->outfile = strdup(token); //store the output file
                if (command->outfile == NULL) { //memory allocation failure handler
                    free_parsedCmd(command);
                    free(copy);
                    return false;
                }
                command->append = true; //set append mode to true
            }
        } else if (strcmp(token, "&") == 0) { //in case the token is '&' (execute background), set background flag to true
            command->background = true; 
        } /*else if (token[0] == '$') {
            command->args[argIndex++] = strdup(token); */
        else { //otherwise, define as a regular argument
            command->args[argIndex++] = strdup(token); //store the argument in args
        }
        token = strtok(NULL, " \t\n"); //continue with the next token
    }
    command->args[argIndex] = NULL; 
    free(copy); //free input string copy
    return command; //return parsed command
}

//free the memory for ParsedCommand
void free_parsedCmd(ParsedCommand* command) {
    //in case the args is not empty, free each argument in the args array
    if (command->args != NULL) {
        for (int i = 0; command->args[i] != NULL; i++) {
            free(command->args[i]); //free each argument 
        }
        free(command->args); //free the args
    }
    if (command->infile != NULL) { //in case the input file is not empty, free the input redirection
        free(command->infile);
    }
    if (command->outfile != NULL) { //in case the output file is not empty, free the output redirection
        free(command->outfile);
    }
    free(command); //free the parsed command
}
