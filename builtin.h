#ifndef BUILTIN_H
#define BUILTIN_H

#include "parse_interface.h"
#include <stdbool.h>
#include <unistd.h> //POSIX operating system API

typedef struct {
    pid_t pid;
    char command[1024];
    int status; 
} BackgroundJob;

bool cdCmd(const ParsedCommand* command); //change directory command
bool pwdCmd(const ParsedCommand* command); //check current working directory
bool echoCmd(const ParsedCommand* command); //print stdout
bool exportCmd(const ParsedCommand* command); //set up enviroment variables
bool jobsCmd(const ParsedCommand* command); //list of background jobs
bool quitCmd(const ParsedCommand* command); //quit or exit quash

//check if a command is a builtin command
bool isBuiltin(const ParsedCommand* command);

//execute the builtin command
bool execBuiltin(const ParsedCommand* command);

#endif
