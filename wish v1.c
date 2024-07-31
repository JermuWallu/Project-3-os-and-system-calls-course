#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

// Function to take input
// 
// It uses the getline()-method to get input, removes newline and copies the string into given param. 
// Finally it frees the input from memory.
int takeInput(char* line)
{
    char* string;
    size_t size = 0;
    ssize_t chars_read;

    printf("wish> ");

    chars_read = getline(&string, &size, stdin);
    if (chars_read < 0) {
        // getline() man:
        // If *lineptr is set to NULL before the call, then getline() will
        // allocate a buffer for storing the line.  This buffer should be
        // freed by the user program even if getline() failed.
        free(string); 
        return 1;
    } else {
        string[strlen(string)-1] = '\0';
        strcpy(line, string);
        free(string);
        return 0;
    }
}

void executeCommand(char *command) {
    char *args[MAX_ARGS];
    int i = 0;
    printf("executing the command '%s'..\n", command); //DEBUG

    // Tokenize the input command
    args[i] = strtok(command, " \t");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        printf("args %d: '%s'\n",i,args[i]); //DEBUG
        i++;
        args[i] = strtok(NULL, " \t");
    }
    args[i] = NULL;

    // Empty command (is also checked in takeInput()?)
    if (args[0] == NULL) {
        return; 
    };

    // exit check
    if (strcmp(args[0], "exit") == 0) {
        printf("exit command found, exiting wish..\n");
        exit(0);
    }

    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Failed forking child..\n");
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            fprintf(stdout, "could not execute the command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        waitpid(pid, NULL, 0);
        return;
    }
}

int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    // too many arg error 
    if (argc > 2) {
        fprintf(stderr, "Too many arguments, exiting..\n");
        exit(1);
    }
    // main loop
    while (1) {
        if (takeInput(line) == 1) {
            printf("%c",'\n');
            break; // EOF or read error
        }

        executeCommand(line);
    }

    return 0;
}
