#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LEN 1000
//=========================== Functions ==========================
void newCommand();
void getCommand(char command[]);
int split(char command[], char* list[], int* background);
void normalExec(char *list[]);
void backExec(char* list[]);
void sigHandler(int signal);
void appendFile();
//======================= End Functions ==========================
char* dir;

int main() {

    char command[MAX_LEN];
    char* list[MAX_LEN / 2];
    int background = 0, listLen = 0;
    FILE *logFile = fopen("logFile", "w");
    dir = realpath("logFile", NULL);
    printf("This shell is created by Hossam Elkordi.\n\n");

    while (1){
        memset(command, NULL, MAX_LEN);
        memset(list, NULL, MAX_LEN / 2);
        getCommand(command);
        listLen = split(command, list, &background);
        if (listLen == 1 && strcmp(list[0], "exit") == 0){
            exit(0);
        } else if (listLen == 2 && strcmp(list[0], "cd") == 0){
            chdir(list[1]);
        }else {
            if (background){
                background = 0;
                backExec(list);
            }else{
                normalExec(list);
            }
        }
    }
}

void newCommand(){
    fflush(stdout);
    printf("\033[0;32m");
    printf(getenv("USER"));
    printf("\033[0m");
    printf(":");
    printf("\033[0;34m");
    printf("~");
    printf("\033[0m");
    printf("$ ");
}

void getCommand(char command[]) {
    newCommand();
    fgets(command, MAX_LEN, stdin);
    command[strlen(command) - 1] = '\0';
    if (strlen(command) == 0) getCommand(command);
}

int split(char command[], char *list[], int* background) {
    if (command[strlen(command) - 1] == '&'){
        command[strlen(command) - 1] = '\0';
        (*background) = 1;
        if (command[strlen(command) - 1] == ' '){
            command[strlen(command) - 1] = '\0';
        }
    }
    int listLen = 0;
    char* dilem = " ";
    char* token = strtok(command, dilem);
    while (token != NULL){
        list[listLen++] = token;
        token = strtok(NULL, dilem);
    }
    list[listLen] = NULL;
    return listLen;
}

void normalExec(char* list[]) {
    int id = fork();
    if (id == 0){
        if (execvp(list[0], list) == -1){
            perror("");
	    exit(1);
        }
    }else if (id == -1){
        printf("Error occurred while forking.\n");
    }else {
        int status;
        wait(&status);
        appendFile();
        if (WIFEXITED(status)){
            int statcode = WEXITSTATUS(status);
            if (statcode != 0){
                perror("");
            }
        }
    }
}

void backExec(char* list[]){
    struct sigaction saction;
    saction.sa_handler = ((void (*)(int))sigHandler);
    saction.sa_flags = SA_RESTART;

    int id = fork();
    if (id == 0){
        if(execvp(list[0], list) == -1){
            perror("");
        }
    } else if (id == -1){
        perror("Error occurred while forking.\n");
    }else{
        sigaction(SIGCHLD, &saction, NULL);
    }
}

void sigHandler(int signal){
    int status ,id = waitpid(-1, &status, WNOHANG);
    if (id > 0){
        appendFile(dir);
    }
}

void appendFile(){
    FILE *logFile = fopen(dir, "a");
    fprintf(logFile, "Child process was terminated.\n");
    fclose(logFile);
}
