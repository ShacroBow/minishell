/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:03:17 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:03:18 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <signal.h>
# include <termios.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <dirent.h>
# include "libft/libft.h"
# include <limits.h>
# include <ctype.h>
# include <sys/ioctl.h>

extern volatile sig_atomic_t	g_exit_status;
typedef int						t_pipe_fd[2];
# define HD_PREFIX "/tmp/minishell_hd_"

/* token */
typedef enum e_tokentype
{
	TOK_WORD,
	TOK_PIPE,
	TOK_AND,
	TOK_OR,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_APPEND,
	TOK_HEREDOC,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_END
}	t_tokentype;

typedef struct s_token
{
	t_tokentype	type;
	char		*value;
	int			quoted;
}	t_token;

typedef struct s_tokenize
{
	char	*buf;
	t_token	*tok;
	int		cap;
	int		len;
	int		size_input;
	int		count;
	int		in_s;
	int		in_d;
	int		quoted;
	int		is_heredoc;
	char	**env_list;
}	t_tokenize;

typedef struct s_command
{
	char				**argv;
	char				*infile;
	char				*outfile;
	int					append;
	int					heredoc;
	int					subshell;
	t_pipe_fd			*pipe_tab;
	struct s_segment	*subshell_segments;
	struct s_command	*next;
	char				**envp;
}	t_command;

typedef struct s_segment
{
	t_command			*pipeline;
	t_tokentype			op;
	struct s_segment	*next;
	char				***envp;
}	t_segment;

//Parse

typedef struct s_parse_segments
{
	int	idx;
	int	n;
	int	in_sub;
}	t_parse_segments;

typedef struct s_ctx
{
	t_token				*tok;
	t_parse_segments	*ps;
	char				**env;
	t_segment			**seg_head;
	t_segment			**seg_tail;
	t_command			*cmd_head;
	t_command			*cmd_tail;
	int					need_cmd;
}	t_ctx;

//Parse

/* tokenizer */
void		ft_create_token(t_tokenize *t);
void		ft_tok_append(t_tokenize *t, char c);
int			ft_is_operator(char c);
void		ft_handle_expansion(t_tokenize *t, const char *input, int *i);
int			ft_handle_filler(t_tokenize *t, const char *input, \
							int *i);
void		ft_handle_operator(t_tokenize *t, const char *input, int *i);
void		ft_expand_exit_status(t_tokenize *t);
void		ft_tok_append_str(t_tokenize *t, char *val);
void		ft_free_env_list(char **env_list);
void		ft_exit_error(char *context);
void		ft_exit_tokenize(t_tokenize *t, char *msg);
char		*ft_get_env_value(t_tokenize *t, char *name);

t_token		*ft_tokenize(const char *input, int *out_count, char **env_list);

/* Parsing */
t_segment	*ft_parse_segments(t_token *tok, t_parse_segments *ps, char **env);
void		ft_new_command(t_command **chead, t_command **ctail, \
								int *needc);
int			ft_handle_word(t_token *tok, int *idx, t_command *cur);
int			ft_handle_parenthesis(t_token *tok, int *idx, int n, \
									t_command *cur);
int			ft_handle_redirection(t_token *tok, int *idx, int n, \
									t_command *cur);
int			ft_redir_error(void);
void		ft_push_pipe_to_seg(t_segment **h, t_segment **t, \
									t_command *pipe, t_tokentype op);
int			ft_write_value(int fd, const char *s);
int			ft_write_expanded_line(char *ln, char **env, int fd);
int			ft_handle_heredoc(t_token *tk, int *idx, t_command *c);
char		*ft_hd_tmp_name(void);
char		**ft_expand_wildcard(const char *pat);
void		ft_free_commands(t_command *cmd);
void		ft_free_segments(t_segment *seg);
void		ft_add_arg(t_command *cmd, const char *val);

t_segment	*ft_parse_input(const char *input, char ***envp);

/* execution */
char		*ft_find_binary(const char *cmd, char **envp);
void		ft_run_pipeline(t_command *cmds, t_segment *seg);
t_pipe_fd	*ft_make_pipes(int n);
void		ft_spawn_children(t_command *cmds, int n, pid_t *pid, \
							t_segment *seg);

int			ft_execute(t_segment *seg_list);

/* Built-in commands */
int			ft_isbuiltin(const char *cmd);
int			ft_exit(char **argv);
int			ft_unset(char **argv, char ***envp);
int			ft_export(char **argv, char ***envp);
void		ft_put_env(char ***envp, const char *kv);
int			ft_cd(char **argv, char ***envp);
int			ft_echo(char **argv);

int			ft_env(char **envp);
char		**ft_execute_builtin(t_command *cmd, char **envp);

/* Utility functions */
char		**ft_dupenvp(char **envp);
void		ft_envpfree(char **envp);
void		ft_free_tab(char **tab);

/* Signal handling */
void		ft_signals_heredoc_setup(void);
void		ft_default_signals(void);
void		ft_signals_print_handler(int print);

void		ft_signal_setup(void);

#endif
