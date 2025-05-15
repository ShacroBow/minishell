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

extern volatile sig_atomic_t	g_exit_status;
typedef int						t_pipe_fd[2];

/* token / AST structure definitions */
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

typedef struct	s_tokenize
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
}	t_command;

typedef struct s_segment
{
	t_command			*pipeline;
	t_tokentype			op;
	struct s_segment	*next;
	char				***envp;
}	t_segment;

/* lexer functions */
t_token		*tokenize(const char *input, int *out_count, char **env_list);
void		create_token(t_tokenize *t);
void		buf_append(t_tokenize *t, char c);
int			is_operator(char c);
void		handle_var_expansion(t_tokenize *t, const char *input, int *i);
int			handle_quotes_and_whitespace(t_tokenize *t, const char *input, int *i);
void		handle_operator(t_tokenize *t, const char *input, int *i);
void		expand_exit_status(t_tokenize *t);
void		append_value_to_buffer(t_tokenize *t, char *val);
void		ft_free_env_list(char **env_list);
void		free_token_array(t_token *tok, t_tokenize *t);
void		exit_error(char *context);
void		clean_exit_tokenize(t_tokenize *t, char *msg);

/* Parsing */
t_segment	*parse_input(const char *input, char ***envp);
void		free_commands(t_command *cmd);
void		free_segments(t_segment *seg);

/* execution */
char		*ft_find_binary(const char *cmd, char **envp);
void		ft_run_pipeline(t_command *cmds, char **envp);
t_pipe_fd	*ft_make_pipes(int n);
void		ft_spawn_children(t_command *cmds, int n, pid_t *pid, char **env);

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
char		*find_executable(const char *cmd, char **envp);
char		**ft_dupenvp(char **envp);
void		ft_envpfree(char **envp);

/* Signal handling */
void		setup_signals(void);
void		setup_heredoc_signals(void);
void		restore_default_signals(void);
void		signals_print_handler(int print);

#endif /* MINISHELL_H */
