#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <limits.h>

#define MAX_ARG_SIZE 100
#define red "\033[31m"
#define green "\033[32m"
#define yellow "\033[33m"
#define blue "\033[34m"
#define magenta "\033[35m"
#define cyan "\033[36m"
#define reset "\033[0m"
#define PROMPT cyan"minishell> "reset

char *find_binary(const char *cmd, char *envp[]) {
	if (strchr(cmd, '/')) {
		if (access(cmd, X_OK) == 0) {
			return strdup(cmd);
		} else {
			return NULL;
		}
	}

	char *path_env = NULL;
	for (int i = 0; envp[i]; i++) {
		if (strncmp(envp[i], "PATH=", 5) == 0) {
			path_env = envp[i] + 5;
			break;
		}
	}

	if (!path_env) {
		fprintf(stderr, "PATH not found in the environment.\n");
		return NULL;
	}

	char *path_dup = strdup(path_env);
	if (!path_dup) {
		perror("strdup");
		return NULL;
	}

	char *saveptr = NULL;
	char *dir = strtok_r(path_dup, ":", &saveptr);
	while (dir != NULL) {
		char fullpath[PATH_MAX];
		snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, cmd);
		if (access(fullpath, X_OK) == 0) {
			free(path_dup);
			return strdup(fullpath);
		}
		dir = strtok_r(NULL, ":", &saveptr);
	}
	free(path_dup);
	return NULL;
}

int execute_command(char *args, char *envp[]) {
	char *binary_path = find_binary(args[0], envp);
	if (!binary_path) {
		fprintf(stderr, "Binary for command '%s' not found in PATH.\n", args[0]);
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		free(binary_path);
		return -1;
	}

	if (pid == 0) {
		// Child Process
		if (execve(binary_path, args, envp) == -1) {
			perror("execve");
			exit(EXIT_FAILURE);
		}
	} else {
		// Parent Process
		int status;
		if (waitpid(pid, &status, 0) == -1) {
			perror("waitpid");
			free(binary_path);
			return -1;
		}
		free(binary_path);
		return WEXITSTATUS(status);
	}
	return 0;
}

void change_directory(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "minishell: cd: missing argument\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("minishell: cd");
		}
	}
}


int main(int argc, char **argv, char **envp)
{//compile with cc main.c -lreadline
	char *input;
	char *args[MAX_ARG_SIZE];

	(void)argc;
	(void)argv;
	rl_clear_history();
	while (1)
	{
		input = readline(PROMPT);
		if (input == NULL) {
			perror("minishell: readline");
			continue;
	}
		if (strcmp(input, "exit") == 0) {
			printf("bye\n");
			free(input);
			return 0;
		}
		add_history(input);
		if (args[0] && strcmp(args[0], "cd") == 0) {
			change_directory(args);
		}
		else if (args[0]) {
			execute_command(input, envp);
		}
		free(input);
	}
	rl_clear_history();
	return 1;
}
