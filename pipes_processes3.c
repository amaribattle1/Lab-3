#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_string>\n", argv[0]);
        exit(1);
    }

    int pipefd1[2]; // Pipe from P1 to P2
    int pipefd2[2]; // Pipe from P2 to P3

    pid_t p1, p2, p3;

    // Create the first pipe
    if (pipe(pipefd1) == -1) {
        perror("Pipe 1 Failed");
        exit(1);
    }

    // Create the second pipe
    if (pipe(pipefd2) == -1) {
        perror("Pipe 2 Failed");
        exit(1);
    }

    // Fork the first child process P1 (Parent - executes cat scores)
    p1 = fork();
    if (p1 < 0) {
        perror("Fork 1 Failed");
        exit(1);
    }

    if (p1 == 0) {
        // Child Process P1 (cat scores)

        // Redirect stdout to the write end of pipe 1
        close(pipefd1[0]); // Close the read end of pipe 1
        dup2(pipefd1[1], STDOUT_FILENO); // Redirect stdout to pipe 1's write end
        close(pipefd1[1]); // Close the original write end of pipe 1

        // Execute the cat command
        execlp("cat", "cat", "scores", (char *)NULL);
        perror("execlp cat Failed");
        exit(1);
    }

    // Fork the second child process P2 (Child - executes grep <argument>)
    p2 = fork();
    if (p2 < 0) {
        perror("Fork 2 Failed");
        exit(1);
    }

    if (p2 == 0) {
        // Child Process P2 (grep <argument>)

        // Redirect stdin to the read end of pipe 1
        close(pipefd1[1]); // Close the write end of pipe 1
        dup2(pipefd1[0], STDIN_FILENO); // Redirect stdin to pipe 1's read end
        close(pipefd1[0]); // Close the original read end of pipe 1

        // Redirect stdout to the write end of pipe 2
        close(pipefd2[0]); // Close the read end of pipe 2
        dup2(pipefd2[1], STDOUT_FILENO); // Redirect stdout to pipe 2's write end
        close(pipefd2[1]); // Close the original write end of pipe 2

        // Execute the grep command
        execlp("grep", "grep", argv[1], (char *)NULL);
        perror("execlp grep Failed");
        exit(1);
    }

    // Fork the third child process P3 (Child's Child - executes sort)
    p3 = fork();
    if (p3 < 0) {
        perror("Fork 3 Failed");
        exit(1);
    }

    if (p3 == 0) {
        // Child Process P3 (sort)

        // Redirect stdin to the read end of pipe 2
        close(pipefd2[1]); // Close the write end of pipe 2
        dup2(pipefd2[0], STDIN_FILENO); // Redirect stdin to pipe 2's read end
        close(pipefd2[0]); // Close the original read end of pipe 2

        // Execute the sort command
        execlp("sort", "sort", (char *)NULL);
        perror("execlp sort Failed");
        exit(1);
    }

    // Parent process: Close all pipe ends in parent since they're not needed
    close(pipefd1[0]);
    close(pipefd1[1]);
    close(pipefd2[0]);
    close(pipefd2[1]);

    // Wait for all child processes to finish
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}