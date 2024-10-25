#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdbool.h>  // Include for boolean type
#include "parse_interface.h"  // Include for ParsedCommand structure

// Function prototypes for executing commands
void execCmd(const ParsedCommand* command);  // Executes a parsed command
//bool is_background_job(const ParsedCommand* command);  // Checks if the command is a background job

#endif // End of EXECUTE_H
