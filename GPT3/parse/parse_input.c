/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_input.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:14 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:04:15 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

extern volatile sig_atomic_t	g_exit_status;

static void	ft_free_tokens(t_token *tok, int count)
{
	int	i;

	i = 0;
	if (!tok)
		return ;
	while (i <= count)
	{
		free(tok[i].value);
		tok[i].value = NULL;
		i++;
	}
	free(tok);
	tok = NULL;
}

void	ft_add_arg(t_command *cmd, const char *val)
{
	int		argc;
	char	**newargv;
	int		i;

	argc = 0;
	i = -1;
	while (cmd->argv && cmd->argv[argc])
		argc++;
	newargv = malloc((argc + 2) * sizeof(char *));
	if (!newargv)
	{
		perror("minishell");
		exit(EXIT_FAILURE);
	}
	while (++i < argc)
		newargv[i] = cmd->argv[i];
	newargv[argc] = ft_strdup(val);
	if (!newargv[argc])
	{
		perror("minishell");
		exit(EXIT_FAILURE);
	}
	newargv[argc + 1] = NULL;
	free(cmd->argv);
	cmd->argv = newargv;
}

static t_segment	*ft_parse_seg(t_token *tok, int tcount, char **env)
{
	t_segment			*ast;
	t_parse_segments	ps;

	ps.idx = 0;
	ps.in_sub = 0;
	ps.n = tcount;
	ast = ft_parse_segments(tok, &ps, env);
	if (ps.idx < tcount && ast)
	{
		ft_putstr_fd("minishell: syntax error near unexpected token\n", \
						STDERR_FILENO);
		g_exit_status = 258;
		ft_free_segments(ast);
		ast = NULL;
	}
	return (ast);
}

static int	ft_check_quotes(const char *input)
{
	int	quotes_s;
	int	quotes_d;
	int	i;

	quotes_s = 0;
	quotes_d = 0;
	i = 0;
	while (input[i] != '\0')
	{
		if (input[i] == '\'' && !quotes_d)
			quotes_s ^= 1;
		else if (input[i] == '"' && !quotes_s)
			quotes_d ^= 1;
		i++;
	}
	return (quotes_s || quotes_d);
}

t_segment	*ft_parse_input(const char *input, char ***envp)
{
	int					tcount;
	t_token				*tok;
	t_segment			*ast;

	tcount = 0;
	if (ft_check_quotes(input))
	{
		ft_putstr_fd("minishell: syntax error unclosed quote\n", STDERR_FILENO);
		return (g_exit_status = 258, NULL);
	}
	tok = ft_tokenize(input, &tcount, *envp);
	if (!tok)
		return (NULL);
	ast = ft_parse_seg(tok, tcount, *envp);
	ft_free_tokens(tok, tcount);
	if (ast)
		ast->envp = envp;
	return (ast);
}
