#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#define SIZE 1024

typedef struct ParseInstruction{
    char CommandType[SIZE];
    int Time;
    char IsPipeline[SIZE]; //-u-> execução um programa, -p ->execução encadeada
    char args[SIZE];
    int pid;
}ParseInstruction;

//TODO: FIFO PARA LER DO CLIENTE



void runProgram(ParseInstruction instruction) {
    char *buffer = malloc(sizeof(char) * SIZE);
    char *commandArgs[SIZE];

    struct timeval start, end;
    gettimeofday(&start, NULL);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Não foi possível criar um processo filho!");
    } else if (pid == 0) {
        int i = 0;
        char *token = strtok(instruction.args, " ");
        while (token != NULL) {
            commandArgs[i++] = token;
            token = strtok(NULL, " ");
        }
        commandArgs[i] = NULL; // Terminate arguments array

        execvp(commandArgs[0], commandArgs);
        if(write(2, "Error: Exec failed\n", sizeof("Error: Exec failed\n"))==-1) {
            perror("erro client write 1");
        }

        _exit(1);
    } else {
        int status;
        wait(&status);
        if (WIFEXITED(status) == 1) {
            gettimeofday(&end, NULL);

            double elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0;
            elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0;
            int nbytes = sprintf(buffer, "Pid: %d\nProgram: %s\nTempo estimado: %d milissegundos\nTime Elapsed: %.2f milliseconds\n",
                                 getpid(), instruction.args, instruction.Time, elapsedTime);
            write(1, buffer, nbytes);
        }
    }
    free(buffer);
}




int main(int argc, char* argv[]){

    while(1) {
        int fd;
        ParseInstruction res;
        mkfifo("fifo", 0666);
        fd = open("fifo", O_RDONLY);
        if (read(fd, &res, sizeof(res)) == -1) {
            perror("erro client write");
        }

        // Verifique se a leitura foi bem-sucedida
        printf("CommandType: %s\nTime: %d\nIsPipeline: %s\nArgs: %s\n",
               res.CommandType, res.Time,
               res.IsPipeline, res.args);

        char fifoc_name[30];
        sprintf(fifoc_name, "sv_cl" "%d", res.pid);

        int fd_client = open(fifoc_name, O_WRONLY);
        int pid = getpid();
       write(fd_client,&pid,sizeof(pid));
       close(fd_client);

        if (strcmp(res.CommandType, "exec") == 0) {
            printf("instruction.args: %s\n", res.args);;// Before runProgram
            runProgram(res); // Check command type
            // Pass the whole instruction struct
        }


        close(fd);
        unlink(fifoc_name);
    }
    unlink("fifo");

    return 0;
}


