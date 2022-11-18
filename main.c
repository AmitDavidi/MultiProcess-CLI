#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<sys/wait.h>
#include <sys/mman.h>

#define PRINT_PROMPT printf("hw1shell$ ")
#define GET_LINE_REGEX "%[^\n]%*c"
#define MAX_CMD_LEN 20
#define MAX_PROCESSES 4
#define MAX_ARGV 6
#define MAX_LEN_OF_USER_COMMAND 100
#define CHDIR_ERROR_CODE -1
#define CHILD_PROCESS_FORK_RETURN_VALUE 0


char* concat(const char *first, const char *second)
{
    char *result = malloc(strlen(first) + strlen(second) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, first);
    strcat(result, second);
    return result;
}

char to_lower(char character)
{
	if(character <= 'Z' && character >= 'A')
		character = character + ('a' - 'A');
	return character;
}

void convert_string_lower_capitals(char *string)
{
	int i;
	for(i = 0; string[i] != 0; i++) {
		string[i] = to_lower(string[i]);
	}
}

int compare_last_element_in_string(char *string_to_check, char character_to_compare) 
{	

	int string_length = strlen(string_to_check);
	char last_char = string_to_check[string_length - 1];

	return last_char == character_to_compare;
}


int main()
{

	int error_flag = 0, running = 1;
	int is_background_process = 0;
	const char delimiter[] = " ";

	char command[MAX_CMD_LEN] = { 0 }; // user input
	char *arguments[MAX_ARGV]; // parsed user input

	char child_process_command_history[MAX_PROCESSES][MAX_LEN_OF_USER_COMMAND] = { {0} }; // command history

	int *number_of_active_processes = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE , MAP_ANONYMOUS | MAP_SHARED, -1, 0); // num of processes
	*number_of_active_processes = 0;
	
	int *PID_external_command = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE , MAP_ANONYMOUS | MAP_SHARED, -1, 0); // num of processes
	*PID_external_command = 0;

	pid_t *pid_array_shared_memory;
	/* Get shared memory between child and father processes */
	pid_array_shared_memory = mmap(
								NULL, MAX_PROCESSES*sizeof(int), PROT_READ | PROT_WRITE,
								MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	int argc = 0;
	while(running) {
		PRINT_PROMPT;
		// scanf(GET_LINE_REGEX, command);
		fgets(command, sizeof(command), stdin);
		command[strcspn(command, "\n")] = 0; // remove new line character

		/* Background if  command ends with & */
		is_background_process = compare_last_element_in_string(command, '&');

		argc = 0;
		char *arguments[MAX_ARGV];

		/* Get the first token (cmd name) */
		arguments[argc] = strtok(command, " ");

		/* Walk through the other tokens (parameters) */
		while((arguments[argc] != NULL) && (argc < MAX_ARGV))
			arguments[++argc] = strtok(NULL, " ");

		if(is_background_process) {
			
			if(*number_of_active_processes == MAX_PROCESSES) {
				fprintf(stderr, "hw1shell: too many background commands running\n");
			}

			else {
				// create a background process
				pid_t fork_value = fork();
				strcpy(child_process_command_history[*number_of_active_processes], command); // save the command
			
				if (fork_value == CHILD_PROCESS_FORK_RETURN_VALUE) {
					pid_t current_pid = getpid(); 

					for(int i = 0; i < MAX_PROCESSES; i++) {
						if(pid_array_shared_memory[i] == 0) {
							pid_array_shared_memory[i] = current_pid;
							break;
						}
					}
					(*number_of_active_processes)++;
					
					


					for(int i = 0; i < MAX_PROCESSES; i++) {
						if(pid_array_shared_memory[i] == current_pid) {
							pid_array_shared_memory[i] = 0;
						}
					}

					(*number_of_active_processes)--;
					exit(1);
				}
			}
		}
	
		else if(strcmp(arguments[0], "cd") == 0) {
			int chdir_result = chdir(arguments[1]);
			if(chdir_result == CHDIR_ERROR_CODE)  {
				fprintf(stderr, "hw1shell: invalid command\n");
			}
		}


		else if(strcmp(arguments[0], "exit") == 0) {
			
			// kill procesess running in background..
			// free dynamically allocated memory
			running = 0;

			wait(NULL); // reap child processes
		}

		else if(strcmp(arguments[0], "jobs") == 0) {
			// print child processes - those that have pid != 0
			for(int i = 0; i < MAX_PROCESSES; i++) {
				if(pid_array_shared_memory[i] != 0)
					printf("%d	%s\n", pid_array_shared_memory[i], child_process_command_history[i]);
			}
		}

		else {
			pid_t fork_value = fork();

			if (fork_value == CHILD_PROCESS_FORK_RETURN_VALUE) {
				*PID_external_command = getpid();
				printf("\n");
				for(int i = 0 ; i < MAX_ARGV; i ++) {
					printf("argument %d = %s\n", i, arguments[i]);
		}
				execvp(arguments[0], arguments);
				exit(0);
			}
			else {
				// HOW TO USE WAITPID******? ask
				waitpid(*PID_external_command, (int*)NULL, 0);
			}
	}	

	
	}

	printf("Exiting...\n");
	return 0;

}
