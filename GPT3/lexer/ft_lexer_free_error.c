#include "../minishell.h"

void	ft_free_env_list(char **env_list)
{
	int	i;

	i = 0;
	while (env_list[i] != NULL)
	{
		free(env_list[i]);
		i++;
	}
	free(env_list);
	env_list = NULL;
}

void	free_token_array(t_token *tok, t_tokenize *t)
{
	int i;

	if (!tok)
		return ;
	i = 0;
	while (tok[i].type != TOK_END || i < t->size_input)
	{
		free(tok[i].value);
		tok[i].value = NULL;
		i++;
	}
	free(tok);
	tok = NULL;
}

void	exit_error(char *context)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(context, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putstr_fd(strerror(errno), STDERR_FILENO);
	ft_putstr_fd("\n", STDERR_FILENO);
	exit(EXIT_FAILURE);
}

void	clean_exit_tokenize(t_tokenize *t, char *msg)
{
	if (t->buf)
		free(t->buf);
	free_token_array(t->tok, t);
	ft_free_env_list(t->env_list);
	exit_error(msg);
}
