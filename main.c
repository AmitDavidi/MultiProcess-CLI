#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define MAX_CMD_LEN 20
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

int main()
{
	int error_flag = 0, running = 1;
	char delimiter = ' ';
	char command[MAX_CMD_LEN] = { 0 };
	char arguments[MAX_ARGV][MAX_CMD_LEN];
	
	for(int i = 0; i < MAX_ARGV; i++) { 
		for(int j = 0; j < MAX_CMD_LEN; j++) {
			arguments[i][j] = 0;
		}
	}

	pid_t pidList[4] = {-1, -1, -1, -1};
	int pidListSize = 0;

	char childProccessUserCommand[MAX_CHILD_PROCESSES][MAX_LEN_OF_USER_COMMAND];
	
	for(int i = 0; i < MAX_CHILD_PROCESSES; i++) {
		for(int j = 0; j < MAX_LEN_OF_USER_COMMAND; j++) {
			arguments[i][j] = 0;
		}
	}

	while(running) {
		printf("hw1shell$ ");
		scanf("%[^\n]%*c", command);

		get_arguments_by_delimiter(arguments, command, &delimiter);

		if(strcmp(arguments[0], "cd") == 0) {
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
		}

		else if(strcmp(arguments[0], "jobs") == 0) {
			// print child processes - those that have pid != 0
			for(int i = 0; i < MAX_CHILD_PROCESSES && pidList[i] != -1; i++) {
				printf("%d	%s\n", pidList[i], childProccessUserCommand[i]);
			}
		}

		else if(strcmp(arguments[0], "fork") == 0) {
			pid_t fork_value = fork();
			pid_t current_pid = getpid(); // get pid of current proccess
			
			if (fork_value == CHILD_PROCESS_FORK_RETURN_VALUE) {
				// child process:
				printf("Child process started: \n");
				pidList[pidListSize++] = current_pid;  //save pid of child process.
				exit();
			}

			else {
				// parent process:
				printf("Father waiting for child process...\n");
				wait(NULL);
			}
			


		}
		
		
		
	}	

	printf("Exiting...\n");
	return 0;

}
