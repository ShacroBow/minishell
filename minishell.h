#ifndef H_MINISHELL
#define H_MINISHELL
//normals
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
//errors
# include <signal.h>
# include <errno.h>
//idk
# include <sys/types.h>
# include <sys/wait.h>
//readline
# include <readline/readline.h>
# include <readline/history.h>
//selfwritten
# include "libft/libft.h"

//	redirection max 7 values or 3 bits
//	001 = 1 = redirect command output to file (>)
//	010 = 2 = redirect file content to command (<)
//	100 = 4 = pipe it (|)
//	111 = 7 = do all of the above (cat < tmp.txt > tmp2.txt | cat -e)
typedef struct s_parse
{
	char	*command;
	int		redirection;
	char	*redir_file;
	char	*target_file;
	int		oldexitcode;
	//t_parse *pipetowards;
	
}	t_parse;

typedef struct s_data 
{
	char	**envp;
	int		exitcode;
	int		oldexitcode;
	char	*lastexec;
	t_parse	*stuffed;
	
}	t_data;


void	ft_signalhandle(void);

#endif