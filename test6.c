#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>

void execute_command(char *input) {
	char *command = strtok(input, " ");
	
	if (strcmp(command, "exit") == 0) {
		printf("bye\n");
		exit(0);
	} else if (strcmp(command, "pwd") == 0) {
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			printf("%s\n", cwd);
		} else {
			perror("getcwd() error");
		}
	} else if (strcmp(command, "ls") == 0) {
		DIR *dir;
		struct dirent *entry;

		dir = opendir(".");
		if (dir == NULL) {
			perror("opendir");
			return;
		}

		while ((entry = readdir(dir)) != NULL) {
			// Skip the current (.) and parent (..) directories
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
				printf("%s\n", entry->d_name);
			}
		}
		closedir(dir);
	} else if (strcmp(command, "cd") == 0) {
		char *path = strtok(NULL, " ");
		if (path == NULL) {
			fprintf(stderr, "cd: missing argument\n");
		} else if (chdir(path) != 0) {
			perror("cd error");
		}
	} else {
		printf("Unknown command: %s\n", command);
	}
}

int main()
{
	char *input;
	char prompt[100] = "directory> ";
	
	while (1) {
		input = readline(prompt);
		if (input == NULL) {
			printf("fart\n");
			break; // Handle Ctrl+D
		}
		add_history(input);
		
		execute_command(input);
		
		free(input);
	}
	
	return 0;
}
