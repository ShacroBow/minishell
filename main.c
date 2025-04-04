
#include "minishell.h"

int main (int argc, char **argv, char **envp)
{
	char *prompt;
	char *input;

	if (argc > 1)
	{
		ft_putstr_fd("minishell does not accept arguments", 2);
		return (EXIT_FAILURE);
	}
	(void)envp;
	(void)argv;
	prompt = "hey";
	while (1)
	{
		input = readline(prompt);
		if (ft_strcmp(input, "exit") == 0)
		{
			printf("bye\n");
			break ;
		}
		printf("%s\n", input);
	}
	
	return (0);
}
