#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
    int fd[2];
    pid_t pid;
    char line[64];
    
    if(pipe(fd) < 0)
    {
        perror("pipe error:");
        return -1;
    }

    pid = fork();
    if(pid > 0)
    {
        printf("father process: child process id is %d\n", pid);
        close(fd[0]);
        char *sendMsg = "hello my son!\n";
        write(fd[1], "hello my son!", strlen(sendMsg));
        close(fd[1]);
    }else if(pid == 0)
    {
        close(fd[1]);
        read(fd[0], line, 64);
        printf("read data: %s", line);
        close(fd[0]);
    }else{
        perror("fork error");
    }

    return 0;
}
