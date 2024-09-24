#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_PIPES 4

// Structure to store command history
typedef struct
{
    char *command;      // The command executed
    char *full_command; // The full input including arguments
    int pid;            // Process ID of the command
    clock_t start_time, end_time; // Start and end times of the command execution
    double duration;    // Duration of the command execution
} history;

// Function to parse the user input into a command and its arguments
void parse_input(char *input, char *command, char **args) {
    char *token;
    int arg_count = 0;

    // Tokenize the input string based on whitespace
    token = strtok(input, " \t\n");

    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " \t\n");
    }

    // Null-terminate the argument list
    args[arg_count] = NULL;

    // Copy the first argument (command) to the command buffer
    strcpy(command, args[0]);
}

// Function to create a new process
int create_new_process(char *command)
{
    // Fork a child process
    pid_t pid = fork();
    return pid;
}

// Function to launch a new process
int launch_process(char *command)
{
    // Create a new process by calling create_new_process()
    int status = create_new_process(command);
    return status;
}

// Function to execute commands from a script file
int execute_script(const char *filename) {
    int status = 0;
    FILE *file = NULL;
    char line[MAX_INPUT_SIZE];

    file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open script file '%s'\n", filename);
        return 1;
    }

    while (fgets(line, sizeof(line), file)) {
        // Remove the trailing newline character
        line[strcspn(line, "\n")] = '\0';

        // Execute the line as a command
        status = launch_process(line);
        if (status != 0) {
            fprintf(stderr, "Error: Command '%s' failed with status %d\n", line, status);
            fclose(file);
            return status;
        }
    }

    fclose(file);
    return status;
}

int main()
{
    char input[MAX_INPUT_SIZE];        // To store user input
    char command[MAX_INPUT_SIZE];      // To store the parsed command
    char *args[MAX_ARGS];              // To store arguments for the command
    clock_t start_time, end_time;      // Timing variables for command execution
    double duration;                   // To store the duration of command execution
    history command_history[MAX_INPUT_SIZE]; // Array to store command history
    int history_count = 0;             // Number of commands in history
    char *temp;

    while(1)
    {
        // Print the shell prompt and get user input
        printf("group50@SimpleShell $ ");
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            // Handle EOF (Ctrl+D)
            break;
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // Check for pipe '|' in the input
        if (strchr(input, '|') != NULL)
        {
            char *commands[32]; // To store the commands split by pipes
            int cmd_count = 0;

            // Tokenize the input based on pipes '|'
            char *token = strtok(input, "|");
            while (token != NULL && cmd_count < 32)
            {
                commands[cmd_count] = token;
                cmd_count++;
                token = strtok(NULL, "|");
            }
            commands[cmd_count] = NULL;  // Null-terminate the array of commands

            int prev_pipe = STDIN_FILENO; // File descriptor for the previous pipe
            int pipefd[2]; // Pipe file descriptors

            // Loop through all commands in the pipeline
            for (int i = 0; i < cmd_count; i++)
            {
                char *raw_input = strdup(commands[i]); // Duplicate the command for history storage
                pipe(pipefd); // Create a pipe
                start_time = clock(); // Record start time of command
                int pid = fork(); // Fork a new process for each command in the pipeline
                end_time = clock();
                duration = (double)(end_time - start_time);

                if (pid == 0) // Child process
                {
                    close(pipefd[0]); // Close the read end of the pipe
                    dup2(prev_pipe, STDIN_FILENO); // Redirect input from the previous command
                    if (i < cmd_count - 1) // Redirect output to the pipe if not the last command
                    {
                        dup2(pipefd[1], STDOUT_FILENO);
                    }
                    close(prev_pipe);
                    close(pipefd[1]);

                    // Parse and execute the current command
                    parse_input(commands[i], command, args);
                    if (execvp(command, args) == -1) 
                    {
                        // Error in executing the command
                        fprintf(stderr, "Error: Failed to execute command '%s'\n", command);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }
                }
                else if (pid == -1) // Error in fork
                {
                    fprintf(stderr, "Error: Fork failed while trying to create a new process.\n");
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else // Parent process
                {
                    close(pipefd[1]); // Close the write end of the pipe
                    prev_pipe = pipefd[0]; // Save the read end for the next command
                }

                // Store the command and its details in history if fork succeeded
                if (pid != -1)
                {
                    history com;
                    char *new_raw_input = strdup(raw_input);
                    temp = strtok(raw_input, " ");
                    com.command = strdup(temp);
                    com.full_command = strdup(new_raw_input);
                    com.end_time = end_time;
                    com.pid = pid;
                    com.start_time = start_time;
                    com.duration = duration;
                    command_history[history_count] = com;
                    history_count++;
                }
            }

            // Wait for all processes in the pipeline to complete
            for (int i = 0; i < cmd_count; i++)
            {
                if (wait(NULL) == -1)
                {
                    perror("Error: Wait failed");
                    break;
                }
            }
        }

        // Handle non-pipe commands
        else
        {
            char *raw_input = strdup(input);
            parse_input(input, command, args);

            if (strcmp(command, "exit") == 0)
            {
                // Exit the shell if the user enters "exit"
                break;
            }

            if (strcmp(command, "history") == 0)
            {
                // Display the history of commands entered
                for (int i = 0; i < history_count; i++)
                {
                    printf("Command: %s\n", command_history[i].command);
                    printf("Arguments: %s\n", command_history[i].full_command);
                    printf("\n");
                }
            }
            else
            {
                start_time = clock(); // Record start time of the command
                int status = launch_process(command); // Launch the command
                end_time = clock();
                duration = (double)(end_time - start_time);

                // Handle child and parent processes
                if (status == 0) // Child process
                {
                    if (execvp(command, args) == -1)
                    {
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }
                }
                else if (status == -1) // Fork failed
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else // Parent process
                {
                    int parent_process_status;
                    waitpid(status, &parent_process_status, 0);

                    if (WIFEXITED(parent_process_status))
                    {
                        printf("Child process exited with status %d\n", WEXITSTATUS(parent_process_status));
                    }

                    // Store command in history
                    if (status != -1)
                    {
                        history com;
                        com.command = strdup(command);
                        com.full_command = strdup(raw_input);
                        com.end_time = end_time;
                        com.pid = status;
                        com.start_time = start_time;
                        com.duration = duration;
                        command_history[history_count] = com;
                        history_count++;
                    }
                }
            }
        }
        fflush(stdout); // Flush stdout buffer to ensure proper output
    } 

    // Print the final command history before exiting
    printf("Exiting group50@SimpleShell\n\n");
    for (int i = 0; i < history_count; i++)
    {
        printf("Command: %s\n", command_history[i].command);
        printf("Arguments: %s\n", command_history[i].full_command);
        printf("Pid: %i\n", command_history[i].pid);
        printf("Start_Time: %ld\n", command_history[i].start_time);
        printf("End_Time: %ld\n", command_history[i].end_time);
        printf("Duration: %f\n", command_history[i].duration);
        printf("\n");
    }
    return 0;
}
