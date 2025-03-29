#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARG_SIZE 100
#define red "\033[31m"
#define green "\033[32m"
#define yellow "\033[33m"
#define blue "\033[34m"
#define magenta "\033[35m"
#define cyan "\033[36m"
#define reset "\033[0m"
#define PROMPT cyan"minishell> "reset

void execute_command(char **args)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("minishell: fork");
		return;
	}
	if (pid == 0)
	{
		// Child process
		if (execve(args[0], args, NULL) == -1)
		{
			perror(red"minishell: execve"reset);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		// Parent process
		int status;
		waitpid(pid, &status, 0);
	}
}

void change_directory(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "minishell: cd: missing argument\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("minishell: cd");
		}
	}
}

void parse_input(char *input, char **args)//This must be reworked to use ft_split
{
	int index = 0;
	char *token = strtok(input, " \n");
	while (token != NULL && index < MAX_ARG_SIZE - 1) {
		args[index++] = token;
		token = strtok(NULL, " \n");
	}
	args[index] = NULL;
}

int main()//this is a proof of concept written by ai.
{//compile with cc main.c -lreadline
	char *input;
	char *args[MAX_ARG_SIZE];

	while (1) {
		input = readline(PROMPT);
		if (input == NULL) {
			perror("minishell: readline");
			continue;
		}

		// Exit command
		if (strcmp(input, "exit") == 0) {
			free(input);
			break;
		}

		// Add input to history
		add_history(input);

		// Parse input
		parse_input(input, args);

		// Handle built-in command 'cd'
		if (args[0] && strcmp(args[0], "cd") == 0) {
			change_directory(args);
		} else if (args[0]) {
			// Execute external command
			execute_command(args);
		}

		free(input);
	}

	return 0;
}
