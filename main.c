#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define MAX_CMD_LEN 20
#define MAX_ARGV 4
#define CHDIR_ERROR_CODE -1

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

void init_matrix(char arguments[MAX_ARGV][MAX_CMD_LEN])
{
	int i, j;

	for(i = 0; i < MAX_ARGV; i++)
		for(j = 0; j< MAX_CMD_LEN; j++)
			arguments[i][j] = 0;
}

int main()
{
	int error_flag = 0, running = 1;
	char delimiter = ' ';
	char command[MAX_CMD_LEN] = { 0 };
	char arguments[MAX_ARGV][MAX_CMD_LEN];

	init_matrix(arguments); // zero the buffer - arguments

	while(running) {
		printf("hw1shell$ ");
		scanf("%[^\n]%*c", command);
		// convert_string_lower_capitals(command);

		get_arguments_by_delimiter(arguments, command, &delimiter);
		printf("Command: %s\n", arguments[0]);
		
		
		if(strcmp(arguments[0], "cd") == 0) {
			int chdir_result = chdir(arguments[1]);
			if(chdir_result == CHDIR_ERROR_CODE)
			{
				fprintf(stderr, "hw1shell: invalid command");
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

		else if(strcmp(arguments[0], "jobs")) {

			
		}
		
		
		
	}	

	printf("Exiting...\n");
	return 0;

}
