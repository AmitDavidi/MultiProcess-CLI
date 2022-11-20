#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
 #include <errno.h>

extern int errno; 


#define PRINT_PROMPT printf("hw1shell$ ")

#define MAX_CMD_LEN 100
#define MAX_PROCESSES 4
#define MAX_ARGV 50
#define MAX_LEN_OF_USER_COMMAND 100
#define CHDIR_ERROR_CODE -1
#define CHILD_PROCESS_FORK_RETURN_VALUE 0
#define CHILD_PROCESS_ERROR_CODE -1
#define SYSTEM_CALL_ERROR -1
#define NO_ACESS_CODE -1

int compare_last_element_in_string_and_remove(char *string_to_check, char character_to_compare) 
{	

	int string_length = strlen(string_to_check);
	char last_char = string_to_check[string_length - 1];
	
	if(last_char == character_to_compare) { // removes '&' from the command arguments
		string_to_check[string_length - 1] = '\0';
	}

	return last_char == character_to_compare; 
}


int main()
{


	int error_flag = 0, running = 1;
	int is_background_process = 0;
	int system_call_status;

	char command[MAX_CMD_LEN] = { 0 }; // user input
	char *arguments[MAX_ARGV]; // parsed user input

	char child_process_command_history[MAX_PROCESSES][MAX_LEN_OF_USER_COMMAND] = { {0} }; // command history

	int *number_of_active_processes = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE,
			 MAP_ANONYMOUS | MAP_SHARED, -1, 0); // num of processes
	*number_of_active_processes = 0;
	
	int *PID_external_command = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE ,
			 MAP_ANONYMOUS | MAP_SHARED, -1, 0); // num of processes
	*PID_external_command = 0;

	pid_t *pid_array_shared_memory;
	/* Get shared memory between child and father processes */
	pid_array_shared_memory = mmap(
								NULL, MAX_PROCESSES*sizeof(int), PROT_READ | PROT_WRITE,
								MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	int argc = 0;
	while(running) {
		PRINT_PROMPT;
		fgets(command, sizeof(command), stdin);

		/* user input parsing*/

		if(command[0] == '\n') {
			continue;
		}

		command[strcspn(command, "\n")] = 0; // remove new line character

		is_background_process = compare_last_element_in_string_and_remove(command, '&');

		/* Parse arguments */
		argc = 0;
		char *arguments[MAX_ARGV];
		arguments[argc] = strtok(command, " ");
		while((arguments[argc] != NULL) && (argc < MAX_ARGV)) {
			arguments[++argc] = strtok(NULL, " "); 
		}


		/* execute command & arguments */

		if(is_background_process) {
			
			/* max proccesses reached */
			if(*number_of_active_processes == MAX_PROCESSES) {
				fprintf(stderr, "hw1shell: too many background commands running\n");
			}

			/* new process */
			else {
				/*store command for future use - jobs*/
				strcpy(child_process_command_history[*number_of_active_processes], command); 

				pid_t fork_value = fork();

				/* fork failed - error 13.*/
				if(fork_value == -1) { 
					fprintf(stderr,"%s failed, errno is %d\n", arguments[0], errno);
				}


				/* fork successfull - do work */
				else { 

					// child process
					if (fork_value == CHILD_PROCESS_FORK_RETURN_VALUE) {
						pid_t current_pid = getpid();

						/* insert into shared memory array */ 
						for(int i = 0; i < MAX_PROCESSES; i++) { 
							if(pid_array_shared_memory[i] == 0) {
								pid_array_shared_memory[i] = current_pid;
								(*number_of_active_processes)++;
								break;
							}
						}

						printf("pid %d started\n", current_pid);
						system_call_status = execvp(arguments[0], arguments);
						
						/* system call fail + command fail*/
						if(system_call_status == SYSTEM_CALL_ERROR) {
							fprintf(stderr, "invalid command\n");
							fprintf(stderr,"%s failed, errno is %d\n", arguments[0], errno);
						}
						
						exit(0); // terminate process
					}

					// father process
					else {
						sleep(0.01);
					}
				}
			}
		}
	
		else if(strcmp(arguments[0], "cd") == 0) {
			
			/* can access file ? */
			if (access(arguments[1], F_OK) == NO_ACESS_CODE) {
				fprintf(stderr, "hw1shell: invalid command\n");
			}

			/* file does exist - use chdir */
			else {
				int chdir_result = chdir(arguments[1]);
				/* chdir failed - system call fail - error 13.*/
				if(chdir_result == CHDIR_ERROR_CODE)  {
					fprintf(stderr,"%s failed, errno is %d\n", arguments[0], errno);
				}
			}
		}
 
		else if(strcmp(arguments[0], "exit") == 0) {
			running = 0;
			wait(NULL); // reap all child processes
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

			/*fork failed - print error 13.*/
			if(fork_value == -1) {
					fprintf(stderr,"%s failed, errno is %d\n", arguments[0], errno);
				}

			/* fork successfull - do work */
			else {
				if (fork_value == CHILD_PROCESS_FORK_RETURN_VALUE) {
					*PID_external_command = getpid();

					system_call_status = execvp(arguments[0], arguments);
					
					/* system call failed + command fail, print error 13.11. */
					if(system_call_status == SYSTEM_CALL_ERROR) {
						fprintf(stderr, "invalid command\n");
						fprintf(stderr, "%s failed, errno is %d\n", arguments[0], errno);
					}
					exit(0);

				}

				else {
					sleep(0.1);
					system_call_status = waitpid(*PID_external_command, (int*)NULL, 0);
					
					/* system call fail - print error 13.*/
					if(system_call_status == -1) {
						fprintf(stderr,"%s failed, errno is %d\n", arguments[0], errno);
					}
				}
			}
		}


		// reap zombie processes
		for(int proccess_num = 0; proccess_num < MAX_PROCESSES; proccess_num++) {

			int pid_to_check = pid_array_shared_memory[proccess_num];
			if(pid_to_check == 0)
				continue; // skip empty pids
			
			system_call_status = waitpid(pid_to_check, (int*)NULL, WNOHANG);
			
			/* system call error print error 13.*/
			if(system_call_status == -1) {
				fprintf(stderr,"%s failed, errno is %d\n", arguments[0], errno);
			}
			/* system call success - do work*/
			else {
				printf("pid %d finished\n", pid_to_check );
				pid_array_shared_memory[proccess_num] = 0; // remove from shared memory
				(*number_of_active_processes)--; // reduce active processes counter by one
			}
		}

	}

	printf("Exiting...\n");
	return 0;

}
