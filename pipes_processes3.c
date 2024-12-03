#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Executes the command "cat scores | grep <user_input> | sort".
 * The user provides an argument to be passed to the grep program.
 * This program demonstrates chaining multiple commands using pipes and forks.
 */

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_argument>\n", argv[0]);
        return 1;
    }

    int pipefd1[2], pipefd2[2];
    int pid1, pid2;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", "-n", NULL};  // Sort numerically

    // Create two pipes: pipefd1 for cat -> grep, and pipefd2 for grep -> sort
    if (pipe(pipefd1) == -1) {
        perror("pipe");
        return 1;
    }
    if (pipe(pipefd2) == -1) {
        perror("pipe");
        return 1;
    }

    pid1 = fork();
    if (pid1 == 0) {
        // First child process (grep)
        
        // Replace stdin with pipefd1[0] (input from cat)
        dup2(pipefd1[0], 0);
        // Replace stdout with pipefd2[1] (output to sort)
        dup2(pipefd2[1], 1);

        // Close unused pipes
        close(pipefd1[1]);
        close(pipefd2[0]);

        // Execute grep
        execvp("grep", grep_args);
        perror("execvp grep failed");
        return 1;
    } else {
        pid2 = fork();
        if (pid2 == 0) {
            // Second child process (sort)
            
            // Replace stdin with pipefd2[0] (input from grep)
            dup2(pipefd2[0], 0);

            // Close unused pipes
            close(pipefd1[0]);
            close(pipefd1[1]);
            close(pipefd2[1]);

            // Execute sort
            execvp("sort", sort_args);
            perror("execvp sort failed");
            return 1;
        } else {
            // Parent process (cat)
            
            // Replace stdout with pipefd1[1] (output to grep)
            dup2(pipefd1[1], 1);

            // Close unused pipes
            close(pipefd1[0]);
            close(pipefd2[0]);
            close(pipefd2[1]);

            // Execute cat
            execvp("cat", cat_args);
            perror("execvp cat failed");
            return 1;
        }
    }

    return 0;
}