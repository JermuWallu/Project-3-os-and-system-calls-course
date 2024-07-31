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

/*
    Function that executes the given param as a command.

    TODO: explain more and define errors
*/
void executeCommand(char *command, char *directories[]) {
    char *args[MAX_ARGS];
    char path[MAX_LINE];
    int i = 0;
    // printf("executing the command '%s'..\n", command); //DEBUG

    // Tokenize the input command
    // didn't know how to use strtok_r() and this was easer to understa for me.
    args[i] = strtok(command, " \t");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        // printf("args %d: '%s'\n",i,args[i]); //DEBUG
        i++;
        args[i] = strtok(NULL, " \t");
    }
    args[i] = NULL;

    // Empty command (need to be checked for batchmode)
    if (args[0] == NULL) {
        return; 
    };

    // built-in command check
    if (strcmp(args[0], "exit") == 0) {
        printf("exit command found, exiting wish..\n");
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        printf("%s\n","'cd' functionality here.");
        // check arg amount
        // chdir error
        exit(0);
    } else if (strcmp(args[0], "path") == 0) {
        printf("%s\n","'path' functionality here.");
        // create search path
        // make the args change overwrite the old path
        exit(0);
    }
    
    // creating path
    strcpy(path, "/bin/");
    strcat(path, args[0]);

    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Failed forking child..\n");
    } else if (pid == 0) {
        if (execv(path, args) == -1) {
            // change path
            strcpy(path, "/usr/bin/");
            strcat(path, args[0]);

            if (execv(path, args) == -1) {
                fprintf(stderr, "could not execute the command..\n");
                exit(1);
            }
        }

        exit(0);
    } else {
        // waiting for child to terminate
        waitpid(pid, NULL, 0);
        return;
    }
}

/*
    Function made to go trough a file and excecute every row as a command

    TODO: explain more and define errors
*/
void batchMode(char name[], char *directories[]) {
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    // File opening and its error handling
    if ((file = fopen(name, "r")) == NULL) {
        fprintf(stderr, "Unable to open the file, exiting..\n");
        exit(1);
    }
    
    // printf("Reading the file '%s'...\n", name); //DEBUG
    
    // reading the file
    while ((nread = getline(&line, &len, file)) != -1) {
        if (line[strlen(line)-1] == '\n') {
            line[strlen(line)-1] = '\0';
        }
        executeCommand(line, directories);
        free(line);
    }

    fclose(file);
    return;
}

int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    char *directories[] = {"/bin", "/usr/bin", NULL};
    // too many arg error 
    if (argc > 2) {
        fprintf(stderr, "Too many arguments, exiting..\n");
        fprintf(stderr, "Usage: 'wish ' OR 'wish [input_file]'\n");
        exit(1);
    } else if (argc == 2) {
        batchMode(argv[1], directories);
        return 0;
    }

    // the interactive loop 
    while (1) {
        if (takeInput(line) == 1) {
            printf("%c",'\n');
            break; // EOF or read error
        }

        executeCommand(line, directories);
    }

    return 0;
}