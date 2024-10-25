#include "quash.h"
#include "builtin.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize Quash
void initQuash() { // Define Quash
    printf("Welcome to Quash!\n"); // Display welcome message
}

// Main function of Quash
void Quash() {
    char input[1024]; // Buffer for holding the input command

    while (true) { // Create a loop for the running shell
        printf("[QUASH]$ "); // Print the prompt
        fflush(stdout); // Ensure the prompt is printed immediately

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) { 
            perror("fgets"); // Print error message on read failure
            break; // Break the loop on error
        }
        
        // Remove trailing newline character
        input[strcspn(input, "\n")] = 0; 

        // Check if the input is empty
        if (strlen(input) == 0) {
            continue; // If empty, prompt again
        }

        ParsedCommand command; // Structure to hold parsed command

        // Parse the input command
        if (parseCmd(input, &command)) {
            execCmd(&command); // Execute the command if parsing was successful
            continue;
        }

        // Check if the command is a built-in command
        if (isBuiltin(&command)) {
            //if (execBuiltin(&command) == NULL) {
                //fprintf(stderr, "Failed to execute built-in command\n");
            execBuiltin(&command);
            
        } else {
            execCmd(&command); // Execute external command
        }

        free_parsedCmd(&command); // Free the command after executing
    }
}

// Cleanup resources before exiting
void quashCleanup() { 
    printf("Exit...\n"); // Display exit message
}

int main() {
    initQuash(); // Initialize the shell
    Quash(); // Start the shell loop
    quashCleanup(); // Cleanup before exiting
    return 0; // Exit the program
}
