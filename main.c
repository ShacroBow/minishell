
#include "minishell.h"


extern int g_signal;

// char *find_binary(const char *cmd, char **envp)
// {
	
// }
char **ft_getenv(char **envp)
{
	char	**data;
	int		i;
	int		j;

	i = 0;
	while (envp[i])
		i++;
	data = malloc((i + 1) * sizeof(char *));
	data[i] = NULL;
	while (i)
	{
		i--;
		j = 0;
		data[i] = malloc((ft_strlen(envp[i]) + 2) * sizeof(char));
		while (envp[i][++j] != '\0')
			data[i][j] = envp[i][j];
		data[i][0] = envp[i][0];
		data[i][j] = '\0';
	}
	return (data);
}


void	ft_init(t_data *data, char **envp)
{
	int i;

	i = 0;
	data->envp = ft_getenv(envp);
	while (envp[i])
	{
		printf("envp[%d]:%s\n", i, envp[i]);
		printf("data[%d]:%s\n", i, data->envp[i]);
		i++;
	}
}


int main (int argc, char **argv, char **envp)
{
	t_data	data;
	char	*prompt;
	char	*input;

	if (argc > 1 && argv)
	{
		// ft_putstr_fd("minishell does not accept arguments", 2);
		return (EXIT_FAILURE);
	}
	// ft_signalhandle();
	ft_init(&data, envp);
	prompt = "Minishell>";
	int i = 0;
	char ptr[1000];
	while (1)
	{
		input = readline(prompt);
		add_history(input);
		//add ft_parse here.
		
		//replace with ft_execute
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
		//replace with ft_execute
		rl_on_new_line();
		i++;
		free(input);
	}
	
	return (0);
}
