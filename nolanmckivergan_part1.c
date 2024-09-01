//Nolan McKivergan
//12285017
//CS 300 Project 1
//A simple implementation of a Unix-like shell

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>


#define MAX_LINE 80 /* The maximum length command */

int main(void) {
    char *args[MAX_LINE / 2 + 1]; /* command line arguments */
    char line[MAX_LINE + 1]; /*Char array to store input string */
    int should_run = 1; /* flag to determine when to exit program */
    int ampersand; /*Flag to record whether an & is included in the commands*/

    while(should_run) {

        ampersand = 0;//Initialize ampersand flag to 0

        //Initialize command prompt
        printf("mckivergan_%d>", getpid());
        fflush(stdout);

        //Get line of input
        fgets(line, MAX_LINE + 1, stdin);

        //Replace newline at end with null terminator
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        //Tokenize input and push to args array
        char *token = strtok(line, " ");
        int argc = 0;
        while(token && argc < (MAX_LINE / 2 + 1)) {
            //If ampersand detected, mark flag and set argument to NULL            
            if (strchr(token, '&') != NULL) {
                args[argc] = NULL;
                ampersand = 1;
            }
            //If no ampersand detected, set argument to token
            else {
                args[argc] = token;
            }
            argc++;
            token = strtok(NULL, " ");
        }

        //Null terminate arguments list
        args[argc++] = NULL;

        //Check for no command
        if (argc == 1) {
            continue;
        }
        // Check for exit command
        else if (strcasecmp(args[0], "exit") == 0) {
            should_run = 0;
        }
        //Process commands
        else {
            pid_t pid = fork();

            //Handle error in fork
            if (pid < 0) {
                fprintf(stderr, "Fork Failed");
                exit(1);
            }

            //Child process
            else if (pid == 0) {
                //Execute commands
                execvp(args[0], args);
                //Terminate loop for child process after execution
                should_run = 0;
            }

            //Parent process
            else {
                //If no & was detected wait until child finishes to continue with parent
                if (ampersand == 0) {
                    wait(NULL);
                }
                //Otherwise if an & was detected parent execution will continue concurrent to child
            }
        }
    }
}