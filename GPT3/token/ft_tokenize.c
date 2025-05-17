/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tokenize.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:46 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:04:47 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static void	ft_token_end(t_tokenize *t)
{
	t->tok[t->count].type = TOK_END;
	t->tok[t->count].value = NULL;
	t->tok[t->count].quoted = 0;
}

void	ft_create_token(t_tokenize *t)
{
	t->tok[t->count].type = TOK_WORD;
	t->tok[t->count].value = t->buf;
	t->tok[t->count].quoted = t->quoted;
	t->count++;
	if (t->is_heredoc == 1)
		t->is_heredoc = 0;
	t->cap = 128;
	t->len = 0;
	t->buf = ft_calloc(t->cap, sizeof(char));
	if (!t->buf)
		ft_exit_tokenize(t, "Create new token malloc failed");
	t->quoted = 0;
}

static void	ft_start_tokenize(t_tokenize *t, const char *input, int *i)
{
	while (input[*i + 1] != '\0' && (input[*i] == '(' || \
			input[*i] == ')') && !(t->in_s || t->in_d))
		(*i)++;
	if (ft_handle_filler(t, input, i))
		return ;
	if (!t->in_s && !t->in_d && ft_is_operator(input[*i]))
	{
		ft_handle_operator(t, input, i);
		return ;
	}
	if (((input[*i] == '\\' && input[*i + 1] == '$') || (input[*i] == '$' && \
		input[*i + 1] != '\0')) && !t->in_s && !t->is_heredoc)
	{
		ft_handle_expansion(t, input, i);
		return ;
	}
	if (input[*i] == '\\' && input[*i + 1] != '\0')
		(*i)++;
	ft_tok_append(t, input[*i]);
	if (t->in_s || t->in_d)
		t->quoted = 1;
	(*i)++;
}

static void	ft_init_tokenize(t_tokenize *t, const char *input, \
									char **env_list)
{
	t->cap = 128;
	t->len = 0;
	t->count = 0;
	t->buf = ft_calloc(t->cap, sizeof(char));
	if (!t->buf)
		ft_exit_error("Token buffer initialisation malloc failed");
	t->size_input = ft_strlen(input) + 1;
	t->tok = malloc(sizeof(t_token) * (t->size_input));
	if (!t->tok)
	{
		free(t->buf);
		ft_exit_error("Array of struct Token initialisation malloc failed");
	}
	t->in_s = 0;
	t->in_d = 0;
	t->quoted = 0;
	t->env_list = env_list;
	if (!t->env_list ||!(*(t->env_list)))
		ft_exit_tokenize(t, "Environnement variable empty");
	t->is_heredoc = 0;
}

t_token	*ft_tokenize(const char *input, int *out_count, char **env_list)
{
	t_tokenize	t;
	int			i;

	ft_init_tokenize(&t, input, env_list);
	i = 0;
	while (input[i])
		ft_start_tokenize(&t, input, &i);
	if (t.len)
		ft_create_token(&t);
	else
	{
		free(t.buf);
		t.buf = NULL;
	}
	*out_count = t.count;
	ft_token_end(&t);
	free(t.buf);
	return (t.tok);
}
