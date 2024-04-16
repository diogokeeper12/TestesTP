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






//TODO: FIFO PARA LER DO CLIENTE





int main(int argc, char* argv[]){
    while(1) {
        int last_pid = 0;
        int fd;
        ParseInstruction instruction;
        mkfifo("fifo_cl_sv", 0666);
        fd = open("fifo_cl_sv", O_RDONLY);
        if (read(fd, &instruction, sizeof(instruction)) == -1) {
            perror("erro client write");
        }


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

        enqueueFCFS(instruction);

        // Debugging: Print queue contents
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

        
        close(fd_client);

        // Task Execution 
        ParseInstruction taskToRun = dequeueFCFS();  
        runProgram(taskToRun); 

       

        printf("READY FOR THE NEXT\n");
        unlink(fifoc_name);
        unlink("fifo_cl_sv");
    }
    return 0;
}