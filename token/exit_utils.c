/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:32 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:04:33 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

static void	ft_free_token_array(t_token *tok, t_tokenize *t)
{
	int	i;

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

void	ft_exit_error(char *context)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(context, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putstr_fd(strerror(errno), STDERR_FILENO);
	ft_putstr_fd("\n", STDERR_FILENO);
	exit(EXIT_FAILURE);
}

void	ft_exit_tokenize(t_tokenize *t, char *msg)
{
	if (t->buf)
		free(t->buf);
	ft_free_token_array(t->tok, t);
	ft_free_env_list(t->env_list);
	ft_exit_error(msg);
}
