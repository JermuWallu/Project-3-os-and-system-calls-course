#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

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


int createPaths(char *directories[], char *args[]) {
    int i = 0;
    while (args[i+1] != NULL) {
        directories[i] = args[i+1];
        // printf("dir: '%s'\n", directories[i]); 
        i++;
    }
    directories[i] = NULL;
    return 0;
}

/*
    Function that executes the given param as a command.

    TODO: explain more and define errors
*/
void executeCommand(char *command, char *directories[]) {
    char *args[MAX_ARGS];
    char path[MAX_LINE];
    int i = 0;

    int isRedirection = 0;
    char *redirect_file = NULL;
    // printf("executing the command '%s'..\n", command); //DEBUG

    // check of redirection 
    if (strtok(command, ">") != NULL) {
        
        redirect_file = strtok(NULL, ">");
        if (redirect_file != NULL) {
            isRedirection = 1;
            redirect_file = strtok(redirect_file, " \t\n");
            
            if (strtok(NULL, " \t\n") != NULL) {
                fprintf(stderr, "wish: syntax error: multiple redirection operators or files\n");
                return;
            }


        }
    }

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
        // argument error handling
        if (args[1] == NULL || args[2] != NULL) {
            fprintf(stderr, "invalid number of arguments.\n");
            return;
        }

        if (chdir(args[1]) != 0) {
            fprintf(stderr, "Error while changing directories.\n");
            return;
        }

        return;
    } else if (strcmp(args[0], "path") == 0) {
        
        if (createPaths(directories, args) != 0 ) {
            fprintf(stderr, "Error creating new directories.\n");
        }
        return;
    }
    
    // creating path
    // strcpy(path, "/bin/");
    // strcat(path, args[0]);

    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Failed forking child..\n");
    } else if (pid == 0) {
        // redirect
        if (isRedirection) {
            int feed = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            if (feed < 0) {
                fprintf(stderr, "Unable to open file.\n");
            }
            dup2(feed, STDOUT_FILENO);
            dup2(feed, STDERR_FILENO);
            close(feed);
        }


        // loop trough directories
        for (i = 0; directories[i] != NULL; i++) {
            snprintf(path, sizeof(path), "%s/%s", directories[i], args[0]);
            execv(path, args);
        }
        fprintf(stderr, "execv failed to run the program.\n");
        exit(1);
    } else {
        // waiting for child to terminate
        waitpid(pid, NULL, 0);

        // if (file) {
        //     fclose(file);
        // }
        return;
    }
}

void executeParallel(char *line, char *directories[]) {
    char *command = strtok(line, "&");
    pid_t pids[MAX_ARGS];
    int i = 0;

    while (command != NULL) {
        if ((pids[i] = fork()) == 0) {
            // exec child process
            executeCommand(command, directories);
            exit(0);
        } else if (pids[i] < 0) {
            // Fork failed
            perror("wish");
            exit(EXIT_FAILURE);
        }
        command = strtok(NULL, "&");
        i++;
    }

    for (int j = 0; j < i; j++) {
        waitpid(pids[j], NULL, 0);
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
    char *directories[999] = {"/bin", "/usr/bin", NULL};
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
        if (strchr(line, '&') != NULL) {
            executeParallel(line, directories);
        } else {
            executeCommand(line, directories);
        }
    }

    return 0;
}