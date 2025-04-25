#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include "libft/libft.h"

extern volatile sig_atomic_t    g_exit_status;

/* token / AST structure definitions */
typedef enum e_tokentype {
	TOK_WORD,
	TOK_PIPE,
	TOK_AND,
	TOK_OR,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_APPEND,
	TOK_HEREDOC,
	TOK_LPAREN,
	TOK_RPAREN
}   t_tokentype;

typedef struct s_token {
	t_tokentype type;
	char        *value;
	int         quoted;
}   t_token;

typedef struct s_command {
	char                **argv;
	char                *infile;
	char                *outfile;
	int                 append;
	int                 heredoc;
	int                 subshell;
	struct s_segment    *subshell_segments;
	struct s_command    *next;
}   t_command;

typedef struct s_segment {
	t_command           *pipeline;
	t_tokentype         op;
	struct s_segment    *next;
}   t_segment;

/* Parsing and execution function prototypes */
t_segment   *parse_input(const char *input, char ***envp);
void        free_segments(t_segment *seg);
void        free_commands(t_command *cmd);
int         execute_segments(t_segment *seg_list, char ***envp);

/* Built-in commands */
int         is_builtin(const char *cmd);
char      **execute_builtin(t_command *cmd, char **envp);
int         builtin_exit(char **argv);

/* Utility functions */
char       *find_executable(const char *cmd, char **envp);
char      **dup_envp(char **envp);

/* Signal handling */
void        setup_signals(void);
void        setup_heredoc_signals(void);
void        restore_default_signals(void);
void        signals_print_handler(int print);

#endif /* MINISHELL_H */
