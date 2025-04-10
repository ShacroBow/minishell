#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

#define MAX_ARGS 64

/*
* Parses the command string into an argv array.
* This simple parser splits on whitespace and does not handle quotes.
*/
void parse_command(const char *cmd, char **argv) {
	char *cmd_copy = strdup(cmd);
	if (!cmd_copy) {
		perror("strdup");
		exit(EXIT_FAILURE);
	}

	int idx = 0;
	char *token = strtok(cmd_copy, " ");
	while (token && idx < MAX_ARGS - 1) {
		argv[idx++] = strdup(token);
		token = strtok(NULL, " ");
	}
	argv[idx] = NULL;
	free(cmd_copy);
}

/*
* Frees up the allocated tokens in the argv array.
*/
void free_argv(char **argv) {
	for (int i = 0; argv[i]; i++) {
		free(argv[i]);
	}
}

/*
* Searches the directories in the PATH environment variable (obtained from envp)
* to find the first occurrence of an executable file matching 'cmd'.
*
* If found, returns a newly allocated string containing the full path.
* Otherwise, returns NULL.
*/
char *find_binary(const char *cmd, char *envp[]) {
	// If cmd already contains a '/', assume it is a path.
	if (strchr(cmd, '/')) {
		if (access(cmd, X_OK) == 0) {
			return strdup(cmd);
		} else {
			return NULL;
		}
	}

	// Look for the "PATH" entry in envp.
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

	// Duplicate path_env to safely tokenize it.
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

/*
* Executes the given command string using execve.
* First, parse the command string, then look up the full path of argv[0]
* using the provided environment pointer.
*
* A pipe is used to capture the command's output.
*
* Returns the exit status of the child process.
*/
int execute_command(const char *cmd, char *envp[]) {
	int fd[2];
	if (pipe(fd) == -1) {
		perror("pipe");
		return -1;
	}

	// Parse the command into an argv array.
	char *argv[MAX_ARGS];
	parse_command(cmd, argv);
	if (!argv[0]) {
		fprintf(stderr, "Command parse failed.\n");
		return -1;
	}

	// Use find_binary() to obtain the full path of the command.
	char *binary_path = find_binary(argv[0], envp);
	if (!binary_path) {
		fprintf(stderr, "Binary for command '%s' not found in PATH.\n", argv[0]);
		free_argv(argv);
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		free(binary_path);
		free_argv(argv);
		return -1;
	}

	if (pid == 0) {
		// Child Process:
		close(fd[0]); // Close the read end.

		// Redirect stdout and stderr to the pipe.
		if (dup2(fd[1], STDOUT_FILENO) == -1) {
			perror("dup2 stdout");
			exit(EXIT_FAILURE);
		}
		if (dup2(fd[1], STDERR_FILENO) == -1) {
			perror("dup2 stderr");
			exit(EXIT_FAILURE);
		}
		close(fd[1]); // Close the original write end.

		// Execute the command with the fully resolved binary path.
		if (execve(binary_path, argv, envp) == -1) {
			perror("execve");
			exit(EXIT_FAILURE);
		}
		// Never reached.
	} else {
		// Parent Process:
		close(fd[1]); // Close the write end.

		// Read and print the output from the child process.
		char buffer[1024];
		ssize_t count;
		while ((count = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
			buffer[count] = '\0';
			printf("%s", buffer);
		}
		if (count == -1) {
			perror("read");
		}
		close(fd[0]);

		// Wait for the child process.
		int status;
		if (waitpid(pid, &status, 0) == -1) {
			perror("waitpid");
			free(binary_path);
			free_argv(argv);
			return -1;
		}

		free(binary_path);
		free_argv(argv);
		return status;
	}
	return 0;
}

int main(int argc, char *argv[], char *envp[]) {
	char *command = NULL;
	if (argc != 2)
	{
		command = "echo Hello, World!";
	}
	else
		command = argv[1];
	int status = execute_command(command, envp);
	
	if (WIFEXITED(status)) {
		printf("\nCommand exited with status %d\n", WEXITSTATUS(status));
	} else {
		printf("\nCommand terminated abnormally %d\n", WEXITSTATUS(status));
	}
	return 0;
}
