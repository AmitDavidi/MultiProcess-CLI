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
#define MAX_ARGV 4
#define MAX_CHILD_PROCESSES 4
#define MAX_LEN_OF_USER_COMMAND 100
#define CHDIR_ERROR_CODE -1
#define CHILD_PROCESS_FORK_RETURN_VALUE 0

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

void get_arguments_by_delimiter(char arguments[MAX_ARGV][MAX_CMD_LEN], char *command, char *delimiter)
{
		char *token;
		int i = 0;

		token = strtok(command, delimiter);
		while(token != NULL) {
			strcpy(arguments[i], token);
			i++;

			token = strtok(NULL, delimiter);
		}
}

int compare_last_element_in_string(char *string_to_check, char character_to_compare) 
{	

	int string_length = strlen(string_to_check);
	char last_char = string_to_check[string_length - 1];

	return last_char == character_to_compare;
}

void init_matrix(char **arguments, char **child_proccess_user_cmd)
{
	memset(arguments, 0, sizeof(char) * MAX_ARGV * MAX_CMD_LEN);
	memset(child_proccess_user_cmd, 0, sizeof(char) * MAX_CHILD_PROCESSES * MAX_LEN_OF_USER_COMMAND);
}

int main()
{

	int pidListSize = 0;
	int error_flag = 0, running = 1;
	int is_background_process = 0;
	char delimiter = ' ';
	char command[MAX_CMD_LEN] = { 0 };
	char arguments[MAX_ARGV][MAX_CMD_LEN];
	char child_proccess_user_cmd[MAX_CHILD_PROCESSES][MAX_LEN_OF_USER_COMMAND];
	pid_t *pid_array_shared_memory;

	/* Init matrices */
	init_matrix(arguments, child_proccess_user_cmd);
	/* Get shared memory between child and father processes */
	pid_array_shared_memory = mmap(
								NULL, MAX_PROCESSES*sizeof(int), PROT_READ | PROT_WRITE,
								MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	while(running) {
		PRINT_PROMPT;
		scanf(GET_LINE_REGEX, command);
		/* Background if  command ends with & */
		is_background_process = compare_last_element_in_string(command, '&');

		get_arguments_by_delimiter(arguments, command, &delimiter);

		if(is_background_process) {
			pid_t fork_value = fork();
		
			if (fork_value == CHILD_PROCESS_FORK_RETURN_VALUE) {
				/* Get pid of child process */
				pid_t current_pid = getpid();
				/* Save pid of child process.*/
				pid_array_shared_memory[pidListSize++] = current_pid;
				sleep(5);


				printf("Child process terminated: \n");
				exit(1);
			}

			else {
				/* Parent process: */
				printf("well...\n");
				//printf("Father waiting for child process...\n");
				//wait(NULL); // reap child process
			}
		}


		else if(strcmp(arguments[0], "cd") == 0) {
			int chdir_result = chdir(arguments[1]);
			if(chdir_result == CHDIR_ERROR_CODE)
			{
				fprintf(stderr, "hw1shell: invalid command\n");
			}
		
			// remove this part later-- *******
			char cwd[100];
			printf("%s\n", getcwd(cwd, sizeof(cwd)));
			// 
		}


		else if(strcmp(arguments[0], "exit") == 0) {
			
			// kill procesess running in background..
			// free dynamically allocated memory
			running = 0;
			wait(NULL); // reap child processes
		}

		else if(strcmp(arguments[0], "jobs") == 0) {
			// print child processes - those that have pid != 0
			for(int i = 0; i < MAX_CHILD_PROCESSES; i++) {
				printf("%d	%s\n", pid_array_shared_memory[i], child_proccess_user_cmd[i]);
			}
		}


		
		
		
	}	

	printf("Exiting...\n");
	return 0;

}
