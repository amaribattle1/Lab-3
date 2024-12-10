#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 

int main() 
{ 
    int fd1[2];  // Pipe 1 for sending string from P1 to P2 
    int fd2[2];  // Pipe 2 for sending string from P2 to P1 

    char fixed_str1[] = "howard.edu"; 
    char fixed_str2[] = "gobison.org"; 
    char input_str[100], second_input_str[100], final_str[200]; 
    pid_t p; 

    // Create both pipes 
    if (pipe(fd1) == -1) { 
        fprintf(stderr, "Pipe 1 Failed"); 
        return 1; 
    } 
    if (pipe(fd2) == -1) { 
        fprintf(stderr, "Pipe 2 Failed"); 
        return 1; 
    } 

    printf("Enter a string to concatenate: ");
    scanf("%s", input_str); 

    p = fork(); 

    if (p < 0) { 
        fprintf(stderr, "Fork Failed"); 
        return 1; 
    } 

    // Parent process (P1) 
    else if (p > 0) { 
        close(fd1[0]);  // Close reading end of pipe 1 
        close(fd2[1]);  // Close writing end of pipe 2 

        // Write input string to pipe 1 
        write(fd1[1], input_str, strlen(input_str) + 1); 
        close(fd1[1]);  // Close writing end of pipe 1 

        // Wait for the child to send a string back 
        wait(NULL); 

        // Read the string sent by P2 from pipe 2 
        read(fd2[0], second_input_str, 100); 

        // Concatenate the full string with "gobison.org"
        strcpy(final_str, second_input_str); // Copy second_input_str into final_str
        strcat(final_str, fixed_str2);       // Concatenate "gobison.org" to it
        
        printf("Final concatenated string: %s\n", final_str); 

        close(fd2[0]);  // Close reading end of pipe 2 
    } 

    // Child process (P2) 
    else { 
        close(fd1[1]);  // Close writing end of pipe 1 
        close(fd2[0]);  // Close reading end of pipe 2 

        // Read string from pipe 1 
        char concat_str[100]; 
        read(fd1[0], concat_str, 100); 

        // Concatenate "howard.edu" 
        strcat(concat_str, fixed_str1); 
        printf("Concatenated string: %s\n", concat_str); 

        // Prompt for the second input 
        printf("Enter another string: ");
        scanf("%s", second_input_str); 

        // Send the original concatenated string and second input back to P1 via pipe 2
        write(fd2[1], concat_str, strlen(concat_str) + 1); 
        close(fd2[1]);  // Close writing end of pipe 2 
        close(fd1[0]);  // Close reading end of pipe 1 

        exit(0); 
    } 

    return 0; 
}