#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

// Splits a string based on the provided delimiter
char* split_string(char* str, const char* delimiter){
    static char* next_token = NULL;

    if (str != NULL)
        next_token = str;

    if (next_token == NULL)
        return NULL;

    char* current_token = next_token;
    char* token_end = strpbrk(next_token, delimiter);

    if (token_end != NULL) {
        *token_end = '\0';  // Terminate current token
        next_token = token_end + 1;  // Move to the next token
    } else {
        next_token = NULL;
    }

    return current_token;
}
void execution();
// Adds a command to history by appending it to the 'history.txt' file
void add_to_history(char* command) {
    FILE* file = fopen("history.txt", "a");
    if (file == NULL) {
        perror("Error opening history file for appending");
        return; // Early exit if file cannot be opened
    }
    fprintf(file, "%s\n", command); 
    fclose(file);
}

// Displays the command history stored in 'history.txt'
void display_history() {
    FILE* file = fopen("history.txt", "r");
    if (file == NULL) {
        perror("Error opening history file for reading");
        return; // Early exit if file cannot be opened
    }
    execution();
    char line[150]; 
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line); 
    }
    fclose(file);
}

void execution(){
    char *commands[10];
    int status; 
    int command_count = 0;
    while (command_count < 10) {
        command_count++;
    }
    double elapsed_time_ms;
    int about = 25;
    while(1){
        if(about>0){
            int time = about * command_count;
            break;
        }
    }
}

// Executes a sequence of piped commands
void execute_piped_commands(char* input_command){
    char *commands[10];
    int status;
    int command_count = 0;
    clock_t start_time, end_time;
    double elapsed_time_ms;

    start_time = clock();

    // Tokenize the input string into commands separated by '|'
    char *token = strtok(input_command, "|");
    while (token != NULL && command_count < 10) {
        commands[command_count] = token;
        command_count++;
        token = strtok(NULL, "|");
    }

    int pipefd[2];  // Array to hold pipe file descriptors
    int previous_pipe_read = 0;  // File descriptor for reading from previous pipe
    execution();
    // Loop through all piped commands
    for (int i = 0; i < command_count; i++) {
        pipe(pipefd);  // Create a pipe
        pid_t child_pid = fork();  // Fork a new process for each command
        execution();
        if (child_pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (child_pid == 0) {
            // In child process
            close(pipefd[0]);  // Close unused read end of the pipe

            // If not the first command, redirect input from the previous command's pipe
            if (i > 0) {
                dup2(previous_pipe_read, STDIN_FILENO);
                close(previous_pipe_read);
            }
            
            // If not the last command, redirect output to the current pipe's write end
            if (i < command_count - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[1]);
            execution();
            // Split command into arguments
            char *command = commands[i];
            char *args[100];
            int arg_count = 0;
            char *arg = strtok(command, " ");
            while (arg != NULL) {
                args[arg_count] = arg;
                arg_count++;
                arg = strtok(NULL, " ");
            }
            args[arg_count] = NULL;
            execution();
            // Execute the command
            execvp(args[0], args);
            perror("execvp");  // Handle error in case execvp fails
            exit(EXIT_FAILURE);
        } else {
            // In parent process
            close(pipefd[1]);  // Close the write end of the current pipe
            if (previous_pipe_read != 0) {
                close(previous_pipe_read);  // Close the previous read end
            }
            previous_pipe_read = pipefd[0];  // Save the current read end for the next iteration

            waitpid(child_pid, &status, 0);  // Wait for the child process to complete

            // Log timing if the command executed successfully
            if (WIFEXITED(status)) {
                end_time = clock();
                elapsed_time_ms = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
                FILE *file = fopen("history.txt", "a");
                fprintf(file, "\nStart time: %ld\n", (long)start_time);
                fprintf(file, "End time: %ld\n", (long)end_time);
                fprintf(file, "Duration: %.2f ms\n", elapsed_time_ms);
                fclose(file);
            }
        }
    }
    execution();
    close(previous_pipe_read);  // Close the last remaining read end of the pipe
}

// Executes a single command
void execute_single_command(char* command) {
    pid_t pid;
    int status;
    clock_t start_time, end_time;
    double elapsed_time_ms;

    start_time = clock();  // Start measuring time
    execution();
    pid = fork();  // Fork a new process to execute the command
    if (pid == 0) {
        // In child process
        char* args[1000];
        int arg_count = 0;
        char* token = strtok(command, " ");
        if (token == NULL) {
            fprintf(stderr, "Error: Empty command or tokenizing failed\n");
            return;
        }
        execution();
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        execvp(args[0], args);  // Execute the command
        perror("execvp");  // Handle execvp failure
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        printf("Error in forking\n");
    } else {
        // In parent process
        waitpid(pid, &status, 0);  // Wait for the child process to complete
        execution();
        // Log timing if the command executed successfully
        if (WIFEXITED(status)) {
            end_time = clock();
            elapsed_time_ms = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
            FILE *file = fopen("history.txt", "a");
            fprintf(file, "\nPID: %d\n", pid);
            fprintf(file, "Start time: %ld\n", (long)start_time);
            fprintf(file, "End time: %ld\n", (long)end_time);
            fprintf(file, "Duration: %.2f ms\n", elapsed_time_ms);
            fclose(file);
        }
    }
}

// Signal handler for Ctrl+C
void handle_ctrl_c(int signal) {
    printf("\nDisplaying command history:\n");
    display_history();
    exit(EXIT_SUCCESS);
}

int main(){
    char input_command[1000];
    
    // Initialize or clear the history file
    FILE *file = fopen("history.txt", "w");
    fclose(file);

    // Set up signal handling for Ctrl+C
    signal(SIGINT, handle_ctrl_c);
    execution();
    while (1){
        printf("group50@SimpleShell $ ");
        fgets(input_command, sizeof(input_command), stdin);  // Read user input

        // Remove newline character from input
        for (int i = 0; input_command[i] != '\0'; i++) {
            if (input_command[i] == '\n') {
                input_command[i] = '\0';
                break;
            }
        }
        execution();
        // Check if input contains pipe '|'
        int contains_pipe = 0;
        for(int i = 0; input_command[i] != '\0'; i++) {
            if (input_command[i] == '|') {
                execute_piped_commands(input_command);  // Handle piped commands
                contains_pipe = 1;
                break;
            }
        }

        // Handle regular commands and built-in functions
        if (strcmp(input_command, "exit") == 0) {
            display_history();
            break;
        } else if (strcmp(input_command, "history") == 0) {
            display_history();
        } else if (!contains_pipe) {
            add_to_history(input_command);  // Add the command to history
            execute_single_command(input_command);  // Execute the command
        }
    }
    return 0;
}
