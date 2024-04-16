#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h> 
#include <sys/stat.h>

#define SIZE 1024


typedef struct ParseInstruction{
    char CommandType[SIZE] ;
    int Time;
    char IsPipeline[SIZE]; //-u-> execução um programa, -p ->execução encadeada
    char args[SIZE];
    int pid;
    long timestamp;
}ParseInstruction;

typedef struct TaskNode {
    ParseInstruction instruction;
    struct TaskNode *next;
} TaskNode;

TaskNode *queueHead = NULL;
TaskNode *queueTail = NULL;





int parse_instruction(int argc, char *argv[], ParseInstruction *instruction) {
    if (argc < 5) {
        // Handle an error: not enough arguments provided
        return -1;
    }

    strcpy(instruction->CommandType, argv[1]);
    instruction->Time = atoi(argv[2]);
    strcpy(instruction->IsPipeline, argv[3]);
    strcpy(instruction->args, argv[4]);
    instruction->pid = getpid();

    struct timeval tv;
    gettimeofday(&tv, NULL);
    instruction->timestamp = tv.tv_sec * 1000000 + tv.tv_usec;

    return 0; // Success
}


/*
//METER NOUTRO FICHEIRO RUNRPOGRAM
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
*/


//TODO: FIFO PARA ESCREVER PARA O SERVER 

int main(int argc, char* argv[]){
    /* PARSE FUNCTION
    
    strcpy(instruction.CommandType,argv[1]);
    instruction.Time = atoi(argv[2]);
    strcpy(instruction.IsPipeline, argv[3]);
    strcpy(instruction.args, argv[4]);
    instruction.pid = getpid();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    instruction.timestamp = tv.tv_sec * 1000000 + tv.tv_usec; // Convertendo para microssegundos
*/
    ParseInstruction instruction;
    if (parse_instruction(argc, argv, &instruction) == -1) {
        fprintf(stderr, "Error: Invalid command line arguments\n");
        return 1;
   }
   printf("Command Type: %s\n", instruction.CommandType);



    char fifoc_name[30];
    sprintf(fifoc_name, "sv_cl_%d", instruction.pid);
    if(mkfifo(fifoc_name, 0666) == -1) {
        perror("mkfifo client");
        return -1;
    }

    int fd = open("fifo_cl_sv", O_WRONLY);

    if(write(fd, &instruction, sizeof(instruction)) ==-1) {
        perror("erro client write 2");
    }

    close(fd);
    //It works
    //Now I just need to send the struct via FIFO
    //printf("CommandType: %s \n Time ms: %d\n IsPipeline: %s\n Argumentos: %s\n",instruction.CommandType,instruction.Time,instruction.IsPipeline,instruction.args);

    /*if(strcmp(instruction.CommandType, "exec") == 0){
        printf("instruction.args: %s\n", instruction.args); // Before runProgram
        runProgram(instruction); // Check command type
         // Pass the whole instruction struct
    }*/

    int fd_sv = open(fifoc_name, O_RDONLY);
    int buf;
    read(fd_sv, &buf,sizeof(buf));

        close(fd_sv);
        printf("Recebi confirmção do pedido com identificador: %d", buf);

        unlink(fifoc_name);
        return 0;    
}



