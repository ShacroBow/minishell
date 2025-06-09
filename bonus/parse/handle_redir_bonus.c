/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redir_bonus.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:11 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/18 14:27:18 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell_bonus.h"

int	ft_write_value(int fd, const char *s)
{
	if (s)
		while (*s)
			if (write(fd, s++, 1) == -1)
				return (-1);
	return (0);
}

char	*ft_hd_tmp_name(void)
{
	static int	seq;
	char		*pid_str;
	char		*seq_str;
	char		*tmp;
	char		*res;

	pid_str = ft_itoa(0);
	seq_str = ft_itoa(seq++);
	if (!pid_str || !seq_str)
		return (free(pid_str), free(seq_str), NULL);
	tmp = ft_strjoin(HD_PREFIX, pid_str);
	res = ft_strjoin(tmp, "_");
	free(tmp);
	tmp = res;
	res = ft_strjoin(res, seq_str);
	free(pid_str);
	free(seq_str);
	free(tmp);
	return (res);
}

static int	ft_set_infile(char *name, t_command *c)
{
	int	ret;

	if (c->infile && c->heredoc)
		unlink(c->infile);
	free(c->infile);
	c->infile = ft_strdup(name);
	c->heredoc = 0;
	ret = 0;
	if (c->infile == NULL)
		ret = -1;
	return (ret);
}

static int	ft_set_outfile(char *name, t_command *c, int append)
{
	free(c->outfile);
	c->outfile = ft_strdup(name);
	if (!c->outfile)
		return (-1);
	c->append = append;
	return (0);
}

int	ft_handle_redirection(t_token *tk, int *i, int n, t_command *c)
{
	t_tokentype	tp;

	tp = tk[*i].type;
	(*i)++;
	if (*i >= n || tk[*i].type != TOK_WORD)
		return (ft_redir_error());
	if (tp == TOK_REDIR_IN && ft_set_infile(tk[*i].value, c) == -1)
		return (-1);
	if (tp == TOK_HEREDOC && ft_handle_heredoc(tk, i, c) == -1)
		return (-1);
	if ((tp == TOK_REDIR_OUT || tp == TOK_APPEND) && \
		ft_set_outfile(tk[*i].value, c, tp == TOK_APPEND) == -1)
		return (-1);
	(*i)++;
	return (0);
}
