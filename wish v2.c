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
    // printf("executing the command '%s'..\n", command); //DEBUG

    // Tokenize the input command
    args[i] = strtok(command, " \t");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        // printf("args %d: '%s'\n",i,args[i]); //DEBUG
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

    printf("pid: %d\n", pid);
    if (pid == -1) {
        fprintf(stderr, "Failed forking child..\n");
    } else if (pid == 0) {
        printf("executing '%s'\n", args[0]);
        if (execvp(args[0], args) == -1) {
            fprintf(stdout, "could not execute the command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        printf("%s\n", "waiting for child to terminate..");
        waitpid(pid, NULL, 0);
        printf("%s","ded\n");
        return;
    }
}

void batchMode(char name[]) {
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
        executeCommand(line);
        free(line);
    }

    fclose(file);
    return;
}
int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    // too many arg error 
    if (argc > 2) {
        fprintf(stderr, "Too many arguments, exiting..\n");
        fprintf(stderr, "Usage: 'wish ' OR 'wish [input_file]'\n");
        exit(1);
    } else if (argc == 2) {
        batchMode(argv[1]);
        return 0;
    }

    // the interactive loop 
    while (1) {
        if (takeInput(line) == 1) {
            printf("%c",'\n');
            break; // EOF or read error
        }

        executeCommand(line);
    }

    return 0;
}