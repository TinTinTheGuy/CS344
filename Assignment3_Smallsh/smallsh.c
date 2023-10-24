#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>


// Constants Declaractions
#define ARGS 	512		
#define LENGTH 	2048	
#define processesMax	1000	

// declaration
int   argrument = 0;				// Number of arguments
char* argList[ARGS];	
int   allowBackground = 1;		
int   isBackground = 0;	
char  CD[100];			        // String of the current directory
int   processes[processesMax];	// Array of all process's pids
int   forkPr = 0;			    // Total number of forked processes
int   PStatus;			        // status of process

struct sigaction SIGINTACTION;	// SIGINT handler
struct sigaction SIGTSTPACTION; // SIGTSTP handler
// declare
int  getCMD(char* args);
void handle_SIGTSTP();
void makeCommands();
void exit_action();
void cd_action();
void status_action(int* errorSignal);
void otherCommands(int* errorSignal);
void childFork();
void parentFork(pid_t childPid);


int main() {
	char argsString[2048];						
	
	//CTRL-Z
	SIGTSTPACTION.sa_handler = handle_SIGTSTP; 	
    SIGTSTPACTION.sa_flags = SA_RESTART; 		
    sigfillset(&SIGTSTPACTION.sa_mask);			
    sigaction(SIGTSTP, &SIGTSTPACTION, NULL);	

    //CTRL-C
    SIGINTACTION.sa_handler=SIG_IGN;			
    sigfillset(&SIGINTACTION.sa_mask); 			
    sigaction(SIGINT, &SIGINTACTION, NULL);		
	
	// Loop to continuously read the lines of shell
	while(1) {
		argrument = getCMD(argsString);		
		argList[argrument] = NULL; 				
		makeCommands();							// Use the params to make commands
	}
	return 0;
}

void handle_SIGTSTP() {
    const char* statusMessages[] = {
        "\nExiting foreground-only mode\n",
        "\nEntering foreground-only mode (& is now ignored)\n",
        "\nError: allowBackground is not 0 or 1\n"
    };
    const int statusMessagesSize[] = {30, 50, 38};
    char* promptMessage = ": ";

    int newIndex = (allowBackground + 1) % 2;

    if (allowBackground != 0 && allowBackground != 1) {
        newIndex = 2;  // Error message index
    } else {
        allowBackground = newIndex;  // Toggling between 0 and 1
    }

    write(STDOUT_FILENO, statusMessages[newIndex], statusMessagesSize[newIndex]);
    write(STDOUT_FILENO, promptMessage, 2);
}

int getCMD(char* args) {
    char* token;
    int num_args = 0;
    char tempString [LENGTH];

    printf(": ");
    fflush(stdout);
    fgets(args, LENGTH, stdin);
    args[strcspn(args, "\n")] = '\0';  

    // Tokenize arguments and perform substitution
    for (token = strtok(args, " "); token != NULL; token = strtok(NULL, " ")) {
        char* dollar_loc = strstr(token, "$$");
        
        // Handle the expansion of variable $$
        if (dollar_loc != NULL) {
            *dollar_loc = '\0';  // Null-terminate at first $
            snprintf(tempString, LENGTH, "%s%d", token, getpid());
            token = tempString;
        }

        argList[num_args++] = token;
    }

    return num_args;
}

typedef void (*CommandFunction)(int*);

void exit_action_wrapper(int* errorSignal) {
    exit_action();
}

void cd_action_wrapper(int* errorSignal) {
    cd_action();
}

void status_action_wrapper(int* errorSignal) {
    status_action(errorSignal);
}

void otherCommands_wrapper(int* errorSignal) {
    otherCommands(errorSignal);
    if (WIFSIGNALED(PStatus) && *errorSignal == 0) {
        status_action(errorSignal);
    }
}

struct Command {
    const char* name;
    CommandFunction func;
};

const struct Command commands[] = {
    {"exit", exit_action_wrapper},
    {"cd", cd_action_wrapper},
    {"status", status_action_wrapper},
    {NULL, otherCommands_wrapper} 
};

void makeCommands() {
    int errorSignal = 0;

    if (argList[0][0] == '#' || argList[0][0] == '\n') {
        return;  // Ignore comments and empty lines
    }

    for (const struct Command* cmd = commands; ; ++cmd) {
        if (cmd->name == NULL || strcmp(argList[0], cmd->name) == 0) {
            cmd->func(&errorSignal);
            break;
        }
    }
}


void exit_action() {
    for (int i = 0; i < forkPr; i++) { // Kill child process if there is one
        kill(processes[i], SIGTERM);
    }
    exit(forkPr == 0 ? 0 : 1);  // exit 0 if no process, 1 if there is
}

void cd_action() {
    const char* dir = argrument > 1 ? argList[1] : getenv("HOME");
    if (chdir(dir) == 0) {
        printf("%s\n", getcwd(CD, 100));
    } else {
        perror("chdir");
    }
    fflush(stdout);
}

void status_action(int* errorSignal) {
	int errHold = 0;
    int sigHold = 0;
    int exitValue;
    waitpid(getpid(), &PStatus, 0);  //status

    if (WIFEXITED(PStatus)) {
        printf("exit value %d\n", WEXITSTATUS(PStatus));
    } else if (WIFSIGNALED(PStatus)) {
        *errorSignal = 1;
        printf("terminated by signal %d\n", WTERMSIG(PStatus));
    }
    fflush(stdout);

}

void otherCommands(int* errorSignal) {
	pid_t pid; // child pid when fork
	isBackground = 0;
    if(strcmp(argList[argrument-1], "&") == 0) {
    	if(allowBackground == 1) 
    		isBackground = 1;
    	// Ignore arg
    	argList[argrument - 1] = NULL;
    }

	pid = fork();					
	processes[forkPr] = pid;	//save pid process
	forkPr++;

	switch(pid) {
		case -1: 
			perror("fork() failed\n");
			exit(1);
			break;

		case 0:  
			childFork();
			break;

		default: 
			parentFork(pid);
	}

	while ((pid = waitpid(-1, &PStatus, WNOHANG)) > 0) {  //	// Wait for the child to finish
        printf("background pid %d is done: ", pid);
        fflush(stdout);
        status_action(errorSignal); 
	}
}
void childFork() {
    char *inputFile = NULL, *outputFile = NULL;
    for (int i = 0; argList[i] != NULL; i++) {
        if (strcmp(argList[i], "<") == 0) {
            inputFile = strdup(argList[i+1]);
            argList[i] = NULL;
            i++;
        } else if (strcmp(argList[i], ">") == 0) {
            outputFile = strdup(argList[i+1]);
            argList[i] = NULL;
            i++;
        }
    }

    if (inputFile) {
        int inputFileDes = open(inputFile, O_RDONLY);       // Set up input redirection
        if (inputFileDes < 0) {
            perror("cannot open input file");
            exit(1);
        }
        dup2(inputFileDes, 0);
        close(inputFileDes);
        free(inputFile);
    }

    if (outputFile) {               // Set up output redirection
        int outputFileDes = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outputFileDes < 0) {
            perror("cannot open output file");
            exit(1);
        }
        dup2(outputFileDes, 1);
        close(outputFileDes);
        free(outputFile);
    }

    if (!isBackground)
        SIGINTACTION.sa_handler = SIG_DFL;      // Set signal handler for SIGINT
    sigaction(SIGINT, &SIGINTACTION, NULL);

    // Execute command
    if (execvp(argList[0], argList) < 0) {
        perror("failed to execute command");
        exit(1);
    }
}

void parentFork(pid_t childPid) {
    if (isBackground) {
        printf("background pid is %d\n", childPid);
        fflush(stdout);
    }
    waitpid(childPid, &PStatus, isBackground ? WNOHANG : 0);
}

