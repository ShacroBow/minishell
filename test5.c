#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

void ft_error(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

char **ft_split(const char *s, char c)
{
	char **result;
	int i, j, k, count = 0;

	for (i = 0; s[i]; i++)
		if (s[i] == c)
			count++;
	result = malloc((count + 2) * sizeof(char *));
	if (!result)
		return NULL;
	i = 0;
	count = 0;
	while (s[i]) {
		while (s[i] == c)
			i++;
		if (s[i] != '\0') {
			j = i;
			while (s[i] && s[i] != c)
				i++;
			k = i - j;
			result[count] = malloc(k + 1);
			if (!result[count])
				return NULL;
			strncpy(result[count], &s[j], k);
			result[count][k] = '\0';
			count++;
		}
	}
	result[count] = NULL;
	return result;
}

void execute_command(char *cmd, char **envp)
{
	char **args = ft_split(cmd, ' ');
	if (!args) {
		ft_error("Memory allocation failed");
	}
	execve(args[0], args, envp);
	perror("execve failed");
	// Clean up allocated memory
	for (int i = 0; args[i] != NULL; i++) {
		free(args[i]);
	}
	free(args);
	exit(EXIT_FAILURE);
}

void pipex(int argc, char **argv, char **envp)
{
	int fd1 = open(argv[1], O_RDONLY);
	int fd2 = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	int pipefd[2];

	if (fd1 < 0 || fd2 < 0 || pipe(pipefd) == -1) {
		ft_error("Error with file or pipe");
	}

	pid_t pid1 = fork();
	if (pid1 < 0) {
		ft_error("Fork failed");
	}

	if (pid1 == 0) 
	{
		dup2(fd1, STDIN_FILENO);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);
		close(fd1);
		close(fd2);
		execute_command(argv[2], envp);
	}

	pid_t pid2 = fork();
	if (pid2 < 0)
	{
		ft_error("Fork failed");
	}

	if (pid2 == 0)
	{
		dup2(pipefd[0], STDIN_FILENO);
		dup2(fd2, STDOUT_FILENO);
		close(pipefd[1]);
		close(fd1);
		close(fd2);
		execute_command(argv[3], envp);
	}

	close(pipefd[0]);
	close(pipefd[1]);
	close(fd1);
	close(fd2);

	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
}
