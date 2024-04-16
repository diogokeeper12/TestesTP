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
    char CommandType[SIZE];
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


void enqueueFCFS(ParseInstruction instruction) {
    TaskNode *newTask = (TaskNode *)malloc(sizeof(TaskNode));
    newTask->instruction = instruction;
    newTask->next = NULL;

    if (queueTail == NULL) { // Queue is empty
        queueHead = newTask;
        queueTail = newTask;
    } else {
        queueTail->next = newTask;
        queueTail = newTask;
    }
}

ParseInstruction dequeueFCFS() {
    if (queueHead == NULL) {
        fprintf(stderr, "Queue underflow\n");
        // Return a dummy instruction
        return (ParseInstruction){ .CommandType = "" }; 
    }

    TaskNode *temp = queueHead;
    ParseInstruction task = temp->instruction;
    queueHead = queueHead->next;
    
    if (queueHead == NULL) { // Queue became empty
        queueTail = NULL;
    }

    free(temp);
    return task;
}



//TODO: FIFO PARA LER DO CLIENTE

int last_pid = 0;

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
        ParseInstruction instruction;
        mkfifo("fifo_cl_sv", 0666);
        fd = open("fifo_cl_sv", O_RDONLY);
        if (read(fd, &instruction, sizeof(instruction)) == -1) {
            perror("erro client write");
        }
        

        enqueueFCFS(instruction);


        printf("Queue Contents:\n");
        TaskNode *current = queueHead;
        int count = 1;
        while (current != NULL) {
            printf("  Task %d:\n", count);
            printf("    CommandType: %s\n", current->instruction.CommandType);
            printf("    Time: %d\n", current->instruction.Time);
            printf("    IsPipeline: %s\n", current->instruction.IsPipeline);
            printf("    Args: %s\n", current->instruction.args);
            printf("    pid: %d\n", current->instruction.pid);
            printf("    timestamp: %ld\n", current->instruction.timestamp);

            current = current->next;
            count++;
        }

        // Verifique se a leitura foi bem-sucedida
        printf("CommandType: %s\nTime: %d\nIsPipeline: %s\nArgs: %s\n",
               instruction.CommandType, instruction.Time,
               instruction.IsPipeline, instruction.args);
        
        char fifoc_name[30];
        sprintf(fifoc_name, "sv_cl_%d", instruction.pid, instruction.timestamp);
        last_pid++;
        instruction.pid = last_pid;
        int fd_client = open(fifoc_name, O_WRONLY);
        int buf = instruction.pid;
        if (write(fd_client, &buf, sizeof(buf)) == -1) {
            perror("erro server write");
            return -1;
        }
        close(fd_client);
        /**int pid = getpid();
        write(fd_client,&pid,sizeof(pid));
        close(fd_client);*/
/*
        // Task Execution Loop
        while (queueHead != NULL) {
            ParseInstruction taskToRun = dequeueFCFS();  
            if (strcmp(taskToRun.CommandType, "execute") == 0) {
                runProgram(taskToRun); 
            } 
        }
        */

        if (strcmp(instruction.CommandType, "execute") == 0) {
            printf("instruction.args: %s\n", instruction.args);// Before runProgram
            runProgram(instruction); // Check command type
            // Pass the whole instruction struct
            printf("READY FOR THE NEXT");
        }
        printf("READY FOR THE NEXT");


        unlink(fifoc_name);
        unlink("fifo_cl_sv");
    }
    return 0;
}


