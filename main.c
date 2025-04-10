
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
	prompt = "Minishell>";
	int i = 0;
	char ptr[1000];
	while (1)
	{
		input = readline(prompt);
		add_history(input);
		if (ft_strcmp(input, "exit") == 0)
		{
			free(input);
			printf("bye\n");
			break ;
		}
		if (ft_strcmp(input, "pwd") == 0)
			printf("%s\n", getcwd(ptr, 600));
		else
		{
			printf("%2d[%s]\n", i, input);
		}
		rl_on_new_line();
		i++;
		free(input);
	}
	
	return (0);
}
